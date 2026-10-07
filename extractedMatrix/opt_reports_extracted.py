#!/usr/bin/env python3
"""GCC optimisation reports + kernel disassembly for the extracted-kernel matrix (Part A), exactly as
llr40Matrix/v2/opt_reports.py builds them: the harness's own compile argv for the cell
(``languages.build_kernel_lib_commands``, same framework compiler and framework extra flags) plus

    -fopt-info-vec-optimized -fopt-info-vec-missed -fopt-info-loop-optimized

then ``objdump -d`` of ``<k>_fp64*``. numba: ``inspect_asm()`` for the signature the harness calls.
The functions below are v2's, with the output directory as a parameter. A flag variant comes from
LLR40_BENCH pointing at the patched worktree, so its argv carries the flag exactly as the timed
build did.

usage: opt_reports_extracted.py --out-dir DIR --index CSV  KERNEL:REPR:LANG:SOURCE [...]
Run on a compute node under the sweep's numactl binding (-march=native, parloops n).

A ``native`` cell's SOURCE is its adapter. The adapter #includes the upstream extraction by a path
relative to the kernel's cpp_backend/, so it is compiled IN PLACE: copied over cpp_backend/<k>_fp64.cpp,
compiled with the harness argv, and the pristine file is restored. Every other source is compiled from
a copy beside its report, as in v2. numba / numba_hand: inspect_asm(), as v2.
"""
import argparse, csv, importlib.util, os, pathlib, re, shutil, subprocess, sys

HERE = pathlib.Path(__file__).resolve().parent
BENCH = pathlib.Path(os.environ["LLR40_BENCH"]).resolve()
sys.path[:0] = [str(BENCH), str(BENCH / "hpcagent_bench/numpy_translators/src")]
OPTINFO = ["-fopt-info-vec-optimized", "-fopt-info-vec-missed", "-fopt-info-loop-optimized"]
FW = {"c": "cc", "c_reference": "cc", "cpp": "cpp", "fortran": "fortran"}
EXT = {"c": ".c", "cpp": ".cpp", "fortran": ".f90"}
FIELDS = ["kernel", "representation", "variant", "lang", "compiler", "source", "source_sha256", "optreport", "asm",
          "status", "n_vec_optimized", "n_vec_missed", "n_loop_optimized", "kernel_functions", "error",
          "compile_argv"]


def count(report, kind):
    pats = {"vec_optimized": r"optimized: loop vectorized|optimized: basic block part vectorized|"
                             r"optimized: .*vectoriz",
            "vec_missed": r"missed: ",
            "loop_optimized": r"optimized: (loop (unrolled|interchanged|distributed|split|peeled|"
                              r"with \d+ iterations completely unrolled|versioned)|"
                              r"parallelizing|completely unrolled|Loop \d+ distributed|loops interchanged)"}
    return sum(1 for l in report.splitlines() if re.search(pats[kind], l))


def compile_argv(lang, fw, src, obj):
    from hpcagent_bench import languages
    from hpcagent_bench.benchmarks import cpp_runtime as R
    so = obj.with_suffix(".so")
    cmds = languages.build_kernel_lib_commands(
        [(lang, src)], so, build_dir=obj.parent, compiler=R.FRAMEWORK_COMPILER.get(fw),
        extra_flags=R._framework_extra_flags(fw))
    comp = [c for c in cmds if "-c" in c]
    assert len(comp) == 1, cmds
    return list(comp[0])


def kernel_asm(obj, k, all_functions=False):
    dis = subprocess.run(["objdump", "-d", str(obj)], capture_output=True, text=True).stdout
    blocks, cur, keep, names = [], [], False, []
    for line in dis.splitlines():
        m = re.match(r"^[0-9a-f]+ <(.+)>:$", line)
        if m:
            if keep and cur:
                blocks.append("\n".join(cur))
            name = m.group(1)
            keep = all_functions or name.startswith(f"{k}_fp64")
            cur = [line] if keep else []
            if keep:
                names.append(name)
        elif keep:
            cur.append(line)
    if keep and cur:
        blocks.append("\n".join(cur))
    head = f"# objdump -d {obj.name}  ({'all functions' if all_functions else f'functions matching {k}_fp64*'})\n"
    return head + "\n\n".join(blocks) + "\n", names


def sha(p):
    import hashlib
    return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()


def kernel_dir(k):
    sc = BENCH / "hpcagent_bench/benchmarks/scientific_computing"
    hits = sorted(p.parent for p in sc.glob(f"**/{k}.yaml") if ".cache" not in p.parts)
    assert len(hits) == 1, (k, hits)
    return hits[0]


def do_compiled(out, k, rep, lang, src_in, rows, variant):
    d = out / k / rep
    d.mkdir(parents=True, exist_ok=True)
    if rep == "native":   # compile the adapter where its relative #include resolves
        src = kernel_dir(k) / "cpp_backend" / f"{k}_fp64{EXT[lang]}"
        bak = d / f"pristine.{src.name}"
        shutil.copy2(src, bak)
        shutil.copy2(src_in, d / pathlib.Path(src_in).name)   # the adapter, beside its report
        shutil.copy2(src_in, src)
        try:
            _do_compiled(out, k, rep, lang, src, rows, variant, d, report_src=d / pathlib.Path(src_in).name)
        finally:
            shutil.copy2(bak, src); os.utime(src); bak.unlink()
        return
    # the harness compiles <k>_fp64.<ext>; keep that name so report lines read like v2's
    src = d / f"{k}_fp64{EXT[lang]}"
    shutil.copy2(src_in, src)
    _do_compiled(out, k, rep, lang, src, rows, variant, d, report_src=src)


def _do_compiled(out, k, rep, lang, src, rows, variant, d, report_src):
    fw = FW.get(rep) or {"c": "cc", "cpp": "cpp", "fortran": "fortran"}[lang]
    obj = d / "build" / f"{src.name}.o"
    obj.parent.mkdir(exist_ok=True)
    row = {"kernel": k, "representation": rep, "variant": variant, "lang": lang,
           "source": str(report_src.relative_to(HERE)), "source_sha256": sha(src)}
    try:
        argv = compile_argv(lang, fw, src, obj)
    except Exception as e:  # noqa: BLE001 -- a failure is a row
        row.update(status="argv_error", error=repr(e)[:400]); rows.append(row); return
    argv = argv + OPTINFO
    row["compile_argv"] = " ".join(argv)
    row["compiler"] = argv[0]
    p = subprocess.run(argv, cwd=str(obj.parent), capture_output=True, text=True)
    rep_path = pathlib.Path(str(report_src) + ".optreport.txt")
    rep_text = p.stderr.replace(str(d) + "/", "").replace(str(src.parent) + "/", "")
    rep_path.write_text(f"$ {' '.join(argv)}\n[exit {p.returncode}]\n{rep_text}{p.stdout}")
    row["optreport"] = str(rep_path.relative_to(HERE))
    if p.returncode != 0 or not obj.is_file():
        err = next((l for l in p.stderr.splitlines() if re.search(r"error", l, re.I)), p.stderr[-300:])
        row.update(status="build_error", error=err.strip()[:400]); rows.append(row); return
    # native: the object holds only the adapter and the #included upstream code, and the loop may sit
    # in the original's own (non-inlined) function, so every function is kept
    asm, names = kernel_asm(obj, k, all_functions=(rep == "native"))
    asm_path = pathlib.Path(str(report_src) + ".s.txt")
    asm_path.write_text(asm)
    row.update(status="ok" if names else "no_kernel_symbol", asm=str(asm_path.relative_to(HERE)),
               kernel_functions=" ".join(names),
               n_vec_optimized=count(p.stderr, "vec_optimized"), n_vec_missed=count(p.stderr, "vec_missed"),
               n_loop_optimized=count(p.stderr, "loop_optimized"))
    shutil.rmtree(obj.parent, ignore_errors=True)   # objects are not artifacts
    rows.append(row)


def do_numba(out, k, src_in, rows, variant):
    d = out / k / variant   # numba or numba_hand
    d.mkdir(parents=True, exist_ok=True)
    src = d / f"{k}_numba_np.py"
    shutil.copy2(src_in, src)
    row = {"kernel": k, "representation": variant, "variant": "", "lang": "python", "compiler": "numba",
           "source": str(src.relative_to(HERE)), "source_sha256": sha(src),
           "compile_argv": "@nb.njit(parallel=True, cache=True); inspect_asm()"}
    try:
        from hpcagent_bench.frameworks.benchmark import Benchmark
        from hpcagent_bench.initialize import auto_initialize
        from hpcagent_bench.precision import precision_from_datatype
        import inspect
        b = Benchmark(k)
        spec = b.spec
        # types only: the signature does not depend on the size, so preset S inputs give the
        # same specialisation the M run compiled. get_data is the harness's own input path (it
        # dispatches to a kernel's custom init.func_name where auto_initialize cannot).
        data = dict(b.get_data(preset="S", datatype="float64"))
        for n, v in b.info["parameters"]["S"].items():
            data.setdefault(n, v)
        sp = importlib.util.spec_from_file_location(f"nbk_{k}_{variant or 'base'}", src)
        mod = importlib.util.module_from_spec(sp); sp.loader.exec_module(mod)
        fn = vars(mod)[b.info["func_name"]]
        py = getattr(fn, "py_func", fn)   # a hand-written file may wrap its njit helpers in plain Python
        args = [data[p] for p in inspect.signature(py).parameters]
        fn(*args)
        disp = [(n, f) for n, f in vars(mod).items() if hasattr(f, "inspect_asm") and getattr(f, "signatures", None)]
        asm = "\n\n".join(f"# function {n}  signature {s}\n{f.inspect_asm(s)}" for n, f in disp for s in f.signatures)
        asm_path = pathlib.Path(str(src) + ".s.txt"); asm_path.write_text(asm)
        row.update(status="ok", asm=str(asm_path.relative_to(HERE)), kernel_functions=" ".join(n for n, _ in disp))
    except Exception as e:  # noqa: BLE001
        row.update(status="numba_error", error=repr(e)[:400])
    rows.append(row)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out-dir", required=True)
    ap.add_argument("--index", required=True)
    ap.add_argument("--variant", default="")
    ap.add_argument("cells", nargs="+", help="KERNEL:REPR:LANG:SOURCE")
    a = ap.parse_args()
    out = pathlib.Path(a.out_dir).resolve()
    rows = []
    for cell in a.cells:
        k, rep, lang, src = cell.split(":", 3)
        if rep in ("numba", "numba_hand"):
            do_numba(out, k, pathlib.Path(src), rows, rep)
        else:
            do_compiled(out, k, rep, lang, pathlib.Path(src), rows, a.variant)
        print(f"{k:28s} {rep:12s} {rows[-1]['status']}", flush=True)
    idx = pathlib.Path(a.index)
    idx.parent.mkdir(parents=True, exist_ok=True)
    old = list(csv.DictReader(open(idx))) if idx.is_file() else []
    with open(idx, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=FIELDS); w.writeheader()
        for row in old + rows:
            w.writerow({f: row.get(f, "") for f in FIELDS})


if __name__ == "__main__":
    # a fresh numba cache per run (v2 DEVIATIONS item 19); cache=True does not change codegen
    import tempfile
    os.environ["NUMBA_CACHE_DIR"] = tempfile.mkdtemp(prefix="optrep_numba_cache_")
    main()
