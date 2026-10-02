#!/usr/bin/env python3
"""Task 2: GCC optimisation reports + kernel disassembly for every compiled cell of the v2 run.

Compile-only. Run on a COMPUTE node (``-march=native``), under the same ``numactl`` binding as the
sweep's build child, so ``-ftree-parallelize-loops={n}`` resolves to the same n.

For each (kernel, representation) in results.csv (preset M) that is compiled -- c, cpp, fortran,
c_reference, and agent in its delivered language -- the fp64 translation unit that was timed is
recompiled with the EXACT compile argv the harness used (``languages.build_kernel_lib_commands``,
same framework compiler and framework extra flags), plus

    -fopt-info-vec-optimized -fopt-info-vec-missed -fopt-info-loop-optimized

The report goes beside the source as ``<source>.optreport.txt`` (first line: the argv), and the
disassembly of the kernel function(s) -- ``<k>_fp64`` and any outlined pieces such as parloops'
``<k>_fp64._loopfn.0`` -- as ``<source>.s.txt``. numba: ``inspect_asm()`` of the kernel for the
signature the harness calls it with. Failures are rows, never skips -> opt_reports_index.csv.
"""
import csv, importlib.util, json, os, pathlib, re, shutil, subprocess, sys

REPO = pathlib.Path(__file__).resolve().parent
BENCH = pathlib.Path(os.environ.get("LLR40_BENCH", "/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2")).resolve()
sys.path[:0] = [str(BENCH), str(BENCH / "hpcagent_bench/numpy_translators/src")]
LLR = BENCH / "hpcagent_bench/benchmarks/loop_level_reasoning"
OUT = REPO / "opt_reports"
OPTINFO = ["-fopt-info-vec-optimized", "-fopt-info-vec-missed", "-fopt-info-loop-optimized"]
FW = {"c": "cc", "c_reference": "cc", "cpp": "cpp", "fortran": "fortran"}
EXT = {"c": ".c", "cpp": ".cpp", "fortran": ".f90"}
FIELDS = ["kernel", "representation", "lang", "compiler", "source", "optreport", "asm", "status",
          "n_vec_optimized", "n_vec_missed", "n_loop_optimized", "kernel_functions", "error", "compile_argv"]


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
    argv = list(comp[0])
    # the harness names the object <src.name>.o in build_dir; keep that
    return argv


def kernel_asm(obj, k):
    dis = subprocess.run(["objdump", "-d", str(obj)], capture_output=True, text=True).stdout
    blocks, cur, keep, names = [], [], False, []
    for line in dis.splitlines():
        m = re.match(r"^[0-9a-f]+ <(.+)>:$", line)
        if m:
            if keep and cur:
                blocks.append("\n".join(cur))
            name = m.group(1)
            keep = name.startswith(f"{k}_fp64")
            cur = [line] if keep else []
            if keep:
                names.append(name)
        elif keep:
            cur.append(line)
    if keep and cur:
        blocks.append("\n".join(cur))
    head = f"# objdump -d {obj.name}  (functions matching {k}_fp64*)\n"
    return head + "\n\n".join(blocks) + "\n", names


def do_compiled(k, rep, lang, src_in, rows):
    d = OUT / k / rep
    d.mkdir(parents=True, exist_ok=True)
    src = d / src_in.name if rep in ("c", "cpp", "fortran") else d / f"{k}_fp64{EXT[lang]}"
    shutil.copy2(src_in, src)
    fw = FW.get(rep) or {"c": "cc", "cpp": "cpp", "fortran": "fortran"}[lang]
    obj = d / "build" / f"{src.name}.o"
    obj.parent.mkdir(exist_ok=True)
    row = {"kernel": k, "representation": rep, "lang": lang, "source": str(src.relative_to(REPO)),
           "optreport": "", "asm": "", "status": "", "n_vec_optimized": "", "n_vec_missed": "",
           "n_loop_optimized": "", "kernel_functions": "", "error": "", "compile_argv": ""}
    try:
        argv = compile_argv(lang, fw, src, obj)
    except Exception as e:  # noqa: BLE001 -- a failure is a row
        row.update(status="argv_error", error=repr(e)[:400]); rows.append(row); return
    argv = argv + OPTINFO
    row["compile_argv"] = " ".join(argv)
    row["compiler"] = argv[0]
    p = subprocess.run(argv, cwd=str(obj.parent), capture_output=True, text=True)
    rep_path = pathlib.Path(str(src) + ".optreport.txt")
    rep_text = p.stderr.replace(str(d) + "/", "")
    rep_path.write_text(f"$ {' '.join(argv)}\n[exit {p.returncode}]\n{rep_text}{p.stdout}")
    row["optreport"] = str(rep_path.relative_to(REPO))
    if p.returncode != 0 or not obj.is_file():
        err = next((l for l in p.stderr.splitlines() if re.search(r"error", l, re.I)), p.stderr[-300:])
        row.update(status="build_error", error=err.strip()[:400]); rows.append(row); return
    asm, names = kernel_asm(obj, k)
    asm_path = pathlib.Path(str(src) + ".s.txt")
    asm_path.write_text(asm)
    row.update(status="ok" if names else "no_kernel_symbol", asm=str(asm_path.relative_to(REPO)),
               kernel_functions=" ".join(names),
               n_vec_optimized=count(p.stderr, "vec_optimized"), n_vec_missed=count(p.stderr, "vec_missed"),
               n_loop_optimized=count(p.stderr, "loop_optimized"))
    rows.append(row)


def do_numba(k, src_in, rows):
    d = OUT / k / "numba"
    d.mkdir(parents=True, exist_ok=True)
    src = d / src_in.name
    shutil.copy2(src_in, src)
    row = {"kernel": k, "representation": "numba", "lang": "python", "compiler": "numba",
           "source": str(src.relative_to(REPO)), "optreport": "", "asm": "", "status": "",
           "n_vec_optimized": "", "n_vec_missed": "", "n_loop_optimized": "", "kernel_functions": "",
           "error": "", "compile_argv": "@nb.njit(parallel=True, cache=True); inspect_asm()"}
    try:
        from hpcagent_bench.frameworks.benchmark import Benchmark
        from hpcagent_bench.initialize import auto_initialize
        from hpcagent_bench.precision import precision_from_datatype
        import inspect
        b = Benchmark(k)
        spec = b.spec
        # types only: the signature does not depend on the size, so preset S inputs give the
        # same specialisation the M run compiled (float64 C-contiguous arrays, int64 sizes)
        vals = auto_initialize(spec, "S", precision_from_datatype("fp64"), distribution="uniform", seed=0)
        data = dict(zip(spec.init.output_args, vals))
        data.update({n: v for n, v in b.info["parameters"]["S"].items()})
        sp = importlib.util.spec_from_file_location(f"nbk_{k}", src)
        mod = importlib.util.module_from_spec(sp); sp.loader.exec_module(mod)
        fn = vars(mod)[b.info["func_name"]]
        args = [data[p] for p in inspect.signature(fn.py_func).parameters]
        fn(*args)
        sigs = list(fn.signatures)
        asm = "\n\n".join(f"# signature {s}\n{fn.inspect_asm(s)}" for s in sigs)
        asm_path = pathlib.Path(str(src) + ".s.txt"); asm_path.write_text(asm)
        row.update(status="ok", asm=str(asm_path.relative_to(REPO)), kernel_functions=b.info["func_name"])
    except Exception as e:  # noqa: BLE001
        row.update(status="numba_error", error=repr(e)[:400])
    rows.append(row)


def main():
    picks = json.load(open(REPO / "agent_picks.json"))
    res = [r for r in csv.DictReader(open(REPO / "results.csv")) if r["preset"] == "M" and not r.get("flags_variant")]
    rows = []
    for r in res:
        k, rep = r["kernel"], r["representation"]
        es = REPO / "emitted_sources" / k
        if rep in ("c", "cpp", "fortran"):
            lang = rep
            do_compiled(k, rep, lang, es / f"{k}_fp64{EXT[lang]}", rows)
        elif rep == "c_reference":
            hand = LLR / k / f"{k}_reference.c"
            if not hand.is_file():
                rows.append({"kernel": k, "representation": rep, "lang": "c", "status": "unsupported",
                             "error": "no hand-written _reference.c in the corpus"}); continue
            do_compiled(k, rep, "c", hand, rows)
        elif rep == "agent":
            m = re.search(r"sha256=([0-9a-f]+)", r["notes"]); lm = re.search(r"lang=(\w+)", r["notes"])
            if not m:
                rows.append({"kernel": k, "representation": rep, "status": "unsupported",
                             "error": r["notes"][:300]}); continue
            c = next(c for c in picks[k] if c["sha256"].startswith(m.group(1)))
            do_compiled(k, rep, lm.group(1), pathlib.Path(c["path"]), rows)
        elif rep == "numba":
            do_numba(k, es / f"{k}_numba_np.py", rows)
        print(f"{k:32s} {rep:12s} {rows[-1]['status']}", flush=True)
    with open(REPO / "opt_reports_index.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=FIELDS); w.writeheader()
        for row in rows:
            for f in ("source", "optreport", "asm"):  # paths are relative to v2/
                row.setdefault(f, "")
            w.writerow({f: row.get(f, "") for f in FIELDS})
    print(f"wrote {len(rows)} rows -> opt_reports_index.csv")


if __name__ == "__main__":
    os.environ.setdefault("NUMBA_CACHE_DIR", str(REPO / "scratch" / "numba_cache"))
    main()
