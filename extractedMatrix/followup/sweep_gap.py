#!/usr/bin/env python3
"""Follow-up to the extracted-kernel matrix: the open gaps (triangle counting, the WarpX Fortran losses,
the hand-written Numba Esirkepov deposition, and four small Fortran losses on LLR-40).

Same measurement path as ``../sweep_extracted.py`` (and LLR-40 v2's sweep.py): every cell runs

    numactl --cpunodebind=0 --membind=0 python -m hpcagent_bench.cli run --benchmark K --framework F
        --precision fp64 --preset M --mode single_core --repeat 30 --validate

with 5 warm-up reps, validated against the NumPy oracle before it is timed, on one core of one
exclusive node. What this script adds:

* named CELLS, ``LABEL[=REPR][:src=PATH][:bin=DIR]``. REPR is c / cpp / fortran / numba (default:
  the label). ``src=PATH`` times PATH in place of the emitted source (or of the numba file); the
  pristine file is restored before every cell and after the kernel, and the bytes in place are
  re-checked after the run. ``bin=DIR`` puts DIR first on the harness's PATH, which is how the
  harness resolves ``gfortran`` (languages.resolve_compiler: first ``gfortran`` on PATH);
  ``bin14/gfortran -> /usr/bin/gfortran-14`` replaces the default ``~/bin/gfortran`` (13.3.1).
  ``env=NAME=VALUE`` (repeatable) sets an environment variable for the cell's harness run.
  ``src32=PATH`` places PATH over ``<k>_fp32.<ext>`` as well: the harness's wrapper binds the symbol by the
  ARGUMENT dtypes (cpp_runtime.wrap_kernel: fp64 if any argument is a float64/complex128 array, else
  fp32), so an integer-only kernel runs ``<k>_fp32`` whatever ``--precision`` says.
  Every source is touched before its cell, so the harness rebuilds lib<k>_<fw>.so each time;
* ROUNDS: all cells of a kernel run ``--rounds`` times in alternating order (as given, reversed,
  as given, ...) on the same node, so a difference between two cells cannot come from their
  position in the sequence or from a different node;
* ``--perf``: ``perf stat -e cycles,instructions,branch-misses`` over the TIMED reps only (the
  LLR-40 follow-up's perf gate, ``perfgate/sitecustomize.py``, byte-identical);
* ``--record-reps N``: after the rounds, one extra series per cell under ``perf record -e cycles:u
  -c 100000``, gated the same way (timed reps only). These rows are profiles, not timings: they
  carry ``round=record``. ``perf report`` (by DSO and symbol) and ``perf annotate`` of the hottest
  kernel symbols are written to ``profiles/<k>/<label>.*.txt`` right after the run, while the
  library that was profiled is still on disk;
* the disassembly of the BUILT library's kernel functions, with their load addresses
  (``asm_so/<k>/<label>.s.txt``), for libraries up to 400 kB of text.

Rows: one per (kernel, label, round). Resumable: a (kernel, label, round) already in --out is skipped.
"""
import argparse, csv, hashlib, json, os, pathlib, re, shutil, statistics, subprocess, sys, time

HERE = pathlib.Path(__file__).resolve().parent                       # extractedMatrix/followup/
BENCH = pathlib.Path(os.environ.get("LLR40_BENCH", HERE / "bench")).resolve()
BENCHMARKS = BENCH / "hpcagent_bench/benchmarks"
PY = os.environ.get("LLR40_PYTHON", sys.executable)
MARK = "hpcagent_bench-autogen"
FW = {"c": ("cc", ".c"), "cpp": ("cpp", ".cpp"), "fortran": ("fortran", ".f90"), "numba": ("numba", None)}
COMPILER = {"cc": "gcc", "cpp": "g++", "fortran": "gfortran", "numba": "numba"}
FIELDS = ["kernel", "label", "representation", "round", "position", "preset", "status", "time_ns_median",
          "time_ns_min", "time_ns_all", "rsd_pct", "compiler_resolved", "compiler_version", "flags", "override",
          "bin_prefix", "cell_env", "source_sha256", "provenance", "perf_cycles", "perf_instructions", "perf_branch_misses", "perf_page_faults",
          "perf_gated_reps", "perf_raw", "notes", "slurm_job", "node", "timestamp", "commit_hash"]
LAST = {"log": "", "extra": {}}


def sha(p):
    return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()


def place(src, dst):
    """copy2 + touch: the harness reuses lib<k>_<fw>.so while it is newer than every source."""
    shutil.copy2(src, dst)
    os.utime(dst)


def kernel_dir(k):
    hits = sorted(p.parent for p in BENCHMARKS.glob(f"**/{k}.yaml") if ".cache" not in p.parts)
    if len(hits) != 1:
        raise SystemExit(f"{k}: expected one manifest under {BENCHMARKS}, found {hits}")
    return hits[0]


def parse_cell(spec):
    label, _, rest = spec.partition("=")
    parts = rest.split(":") if rest else []
    repr_ = parts[0] if parts and "=" not in parts[0] else label
    opts = dict(p.split("=", 1) for p in parts if "=" in p and not p.startswith("env="))
    env = dict(p[4:].split("=", 1) for p in parts if p.startswith("env="))
    if repr_ not in FW:
        raise SystemExit(f"cell {spec}: unknown representation {repr_}")
    return {"label": label, "repr": repr_, "src": opts.get("src", ""), "src32": opts.get("src32", ""),
            "bin": opts.get("bin", ""), "env": env}


def env_for(cell, perf):
    e = dict(os.environ)
    e.update({
        "PYTHONPATH": f"{BENCH}:{BENCH}/hpcagent_bench/numpy_translators/src",
        "OMP_NUM_THREADS": "1", "OMP_PLACES": "cores", "OMP_PROC_BIND": "close",
        "NUMBA_NUM_THREADS": "1",
        "HPCAGENT_BENCH_MEASUREMENT_WARMUP": str(ARGS.warmup),
        "HPCAGENT_BENCH_DB_SHARD": str(ARGS.db_shard),
    })
    if cell["bin"]:
        e["PATH"] = f"{(HERE / cell['bin']).resolve()}:{e['PATH']}"
    e.update(cell.get("env") or {})
    if perf:
        e["PYTHONPATH"] = f"{HERE / 'perfgate'}:{e['PYTHONPATH']}"
    return e


def resolved_compiler(cell):
    fw = FW[cell["repr"]][0]
    if fw == "numba":
        import numba
        return "numba", f"numba {numba.__version__}"
    name = COMPILER[fw]
    code = ("import sys;sys.path[:0]=sys.argv[2:];from hpcagent_bench import languages as L;"
            "print(L.resolve_compiler(sys.argv[1]) or sys.argv[1])")
    p = subprocess.run([PY, "-c", code, name, str(BENCH), str(BENCH / "hpcagent_bench/numpy_translators/src")],
                       env=env_for(cell, False), capture_output=True, text=True)
    exe = p.stdout.strip() or name
    v = subprocess.run([exe, "--version"], capture_output=True, text=True).stdout.splitlines()
    return exe, (v[0] if v else "")


def parse_perf_stat(path):
    got, raw = {}, []
    for line in pathlib.Path(path).read_text().splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        raw.append(line.strip())
        f = line.split(",")
        if len(f) >= 3:
            try:
                got[f[2].split(":")[0]] = int(float(f[0]))
            except ValueError:
                got[f[2].split(":")[0]] = f[0]
    return got, " | ".join(raw)


def run_framework(k, cell, reps, mode):
    """mode: 'time' (optionally under perf stat) or 'record' (perf record). Returns (status, samples_ms, note)."""
    fw = FW[cell["repr"]][0]
    tag = f"{k}.{cell['label']}.{os.getpid()}"
    out = pathlib.Path(ARGS.scratch) / f"{tag}.jsonl"
    out.unlink(missing_ok=True)
    cmd = ["numactl", "--cpunodebind=0", "--membind=0",
           PY, "-m", "hpcagent_bench.cli", "run", "--benchmark", k,
           "--framework", fw, "--precision", "fp64", "--preset", ARGS.preset,
           "--mode", "single_core", "--repeat", str(reps), "--validate", "--output", str(out)]
    gated = ARGS.perf or mode == "record"
    env = env_for(cell, gated)
    LAST["extra"] = {}
    if gated:
        ctl, ack = (pathlib.Path(ARGS.scratch) / f"perf.{tag}.{x}.fifo" for x in ("ctl", "ack"))
        plog = pathlib.Path(ARGS.scratch) / f"perf.{tag}.gate.log"
        for f in (ctl, ack, plog):
            f.unlink(missing_ok=True)
        os.mkfifo(ctl); os.mkfifo(ack)
        env.update({"FOLLOWUP_PERF_CTL": str(ctl), "FOLLOWUP_PERF_ACK": str(ack), "FOLLOWUP_PERF_LOG": str(plog)})
        if mode == "record":
            pdata = pathlib.Path(ARGS.scratch) / f"perf.{tag}.data"
            pdata.unlink(missing_ok=True)
            pre = ["perf", "record", "-e", "cycles:u", "-c", "100000", "--delay=-1",
                   f"--control=fifo:{ctl},{ack}", "-o", str(pdata)]
        else:
            pout = pathlib.Path(ARGS.scratch) / f"perf.{tag}.csv"
            pout.unlink(missing_ok=True)
            pre = ["perf", "stat", "-x,", "-e", "cycles,instructions,branch-misses,page-faults", "--delay=-1",
                   f"--control=fifo:{ctl},{ack}", "-o", str(pout)]
        # perf prepends /usr/lib/perf-core:/usr/bin to its child's PATH; `env PATH=` restores the
        # PATH the cell resolves its compiler from (LLR-40 follow-up, sweep_followup.py)
        cmd = pre + ["--", "env", f"PATH={env['PATH']}"] + cmd
    try:
        p = subprocess.run(cmd, cwd=BENCH, env=env, capture_output=True, text=True, timeout=ARGS.timeout)
    except subprocess.TimeoutExpired as e:
        LAST["log"] = f"$ {' '.join(cmd)}\n[timeout]\n{e.stdout or ''}\n{e.stderr or ''}"
        return "timeout", [], f"exceeded {ARGS.timeout}s"
    LAST["log"] = f"$ {' '.join(cmd)}\n[exit {p.returncode}]\n--- stdout\n{p.stdout}\n--- stderr\n{p.stderr}"
    if gated:
        gl = plog.read_text().strip().splitlines() if plog.is_file() else []
        LAST["extra"]["perf_gated_reps"] = ";".join(re.search(r"gated=(\d+)", l).group(1) for l in gl) or "0"
        for f in (ctl, ack):
            f.unlink(missing_ok=True)
        if mode == "record":
            LAST["extra"]["perf_data"] = str(pdata)
        elif pout.is_file():
            got, raw = parse_perf_stat(pout)
            LAST["extra"].update(perf_cycles=got.get("cycles", ""), perf_instructions=got.get("instructions", ""),
                                 perf_branch_misses=got.get("branch-misses", ""),
                                 perf_page_faults=got.get("page-faults", ""), perf_raw=raw)
    if not out.is_file():
        tail = (p.stderr or p.stdout or "").strip().splitlines()
        return "build_error", [], " | ".join(tail[-4:])[:900]
    rows = [json.loads(l) for l in out.read_text().splitlines() if l.strip()]
    rows = [r for r in rows if r.get("framework") == fw]
    if not rows or not (rows[-1].get("impls") or {}):
        tail = (p.stderr or "").strip().splitlines()
        return "build_error", [], " | ".join(tail[-4:])[:900]
    impl = list(rows[-1]["impls"].values())[0]
    samples = impl.get("time_python") or []
    if impl.get("validated") is False:
        return "incorrect", samples, "output did not match the NumPy reference"
    if not samples:
        return "build_error", [], "no timings"
    return "ok", samples, ""


def so_path(k, cell):
    fw = FW[cell["repr"]][0]
    return kernel_dir(k) / "cpp_backend" / "build" / f"lib{k}_{fw}.so"


def save_so_asm(k, cell):
    """objdump of the built library's kernel functions, at their load addresses."""
    so = so_path(k, cell)
    if cell["repr"] == "numba" or not so.is_file():
        return
    text = subprocess.run(["size", "-A", str(so)], capture_output=True, text=True).stdout
    m = re.search(r"^\.text\s+(\d+)", text, re.M)
    if not m or int(m.group(1)) > 400_000:
        return
    dis = subprocess.run(["objdump", "-d", str(so)], capture_output=True, text=True).stdout
    keep, out = False, [f"# objdump -d lib{k}_{FW[cell['repr']][0]}.so, functions matching {k}_fp64* "
                        f"(sha256 {sha(so)[:16]})"]
    for line in dis.splitlines():
        m = re.match(r"^([0-9a-f]+) <(.+)>:$", line)
        if m:
            keep = m.group(2).startswith(f"{k}_fp64")
            if keep:
                out.append(f"\n{line}    # address mod 64 = {int(m.group(1), 16) % 64}")
            continue
        if keep and line.strip():
            out.append(line)
    d = HERE / "asm_so" / k
    d.mkdir(parents=True, exist_ok=True)
    (d / f"{cell['label']}.s.txt").write_text("\n".join(out) + "\n")


def save_profile(k, cell, pdata):
    """perf report by DSO+symbol, and perf annotate of the hottest symbols in the kernel's own code."""
    d = HERE / "profiles" / k
    d.mkdir(parents=True, exist_ok=True)
    base = d / cell["label"]
    rep = subprocess.run(["perf", "report", "-i", pdata, "--stdio", "--sort", "dso,sym", "--percent-limit", "0.1"],
                         capture_output=True, text=True)
    pathlib.Path(f"{base}.report.txt").write_text(rep.stdout + ("\n# stderr\n" + rep.stderr if rep.stderr.strip() else ""))
    syms = []
    for line in rep.stdout.splitlines():
        m = re.match(r"^\s+([\d.]+)%\s+(\S+)\s+\[\.\]\s+(\S+)", line)
        if m and float(m.group(1)) >= 2.0 and m.group(2).startswith(f"lib{k}_"):
            syms.append(m.group(3))
    ann = []
    for s in syms[:4]:
        a = subprocess.run(["perf", "annotate", "-i", pdata, "--stdio", "-s", s], capture_output=True, text=True)
        ann.append(f"########## {s}\n{a.stdout}")
    if ann:
        pathlib.Path(f"{base}.annotate.txt").write_text("\n".join(ann))
    pathlib.Path(pdata).unlink(missing_ok=True)


def flush(rows):
    tmp = pathlib.Path(str(ARGS.out) + ".tmp")
    with open(tmp, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=FIELDS, extrasaction="ignore"); w.writeheader(); w.writerows(rows)
    tmp.replace(ARGS.out)


def main():
    k = ARGS.kernel
    cells = [parse_cell(c) for c in ARGS.cells]
    rows = list(csv.DictReader(open(ARGS.out))) if pathlib.Path(ARGS.out).is_file() else []
    kdir = kernel_dir(k)
    backend = kdir / "cpp_backend"
    # remove an unmarked fp64 source a killed run left behind (never the numba file: it may be hand-written)
    for ext in (".c", ".cpp", ".f90"):
        f = backend / f"{k}_fp64{ext}"
        if f.is_file() and MARK not in f.read_text().splitlines()[0]:
            print(f"removing leftover non-autogen {f.name}", flush=True); f.unlink()
    gen = subprocess.run(
        [PY, "-c", "import sys,hpcagent_bench.autogen as A;A.ensure_native(sys.argv[1]);A.ensure(sys.argv[1],['numba_np'])", k],
        cwd=BENCH, env=env_for({"bin": ""}, False), capture_output=True, text=True)
    if gen.returncode != 0:
        raise SystemExit(f"autogen failed: {gen.stderr[-800:]}")
    pristine = {}
    for f in ([backend / f"{k}_fp{b}{e}" for b in (64, 32) for e in (".c", ".cpp", ".f90")]
              + [kdir / f"{k}_numba_np.py"]):
        if f.is_file():
            bak = pathlib.Path(ARGS.scratch) / f"pristine.{f.name}"
            shutil.copy2(f, bak); pristine[f] = (bak, sha(f))
            keep = HERE / "emitted_sources" / k; keep.mkdir(parents=True, exist_ok=True)
            shutil.copy2(f, keep / f.name)
            print(f"pristine {f.name} {sha(f)[:16]}", flush=True)
    vers = {c["label"]: resolved_compiler(c) for c in cells}
    for c in cells:
        print(f"cell {c['label']}: {c['repr']} src={c['src'] or '-'} src32={c['src32'] or '-'} bin={c['bin'] or '-'} "
              f"env={c['env'] or '-'} -> {vers[c['label']]}", flush=True)
    schedule = []
    for r in range(1, ARGS.rounds + 1):
        order = cells if r % 2 == 1 else cells[::-1]
        schedule += [(str(r), pos, c) for pos, c in enumerate(order, 1)]
    if ARGS.record_reps:
        schedule += [("record", pos, c) for pos, c in enumerate(cells, 1)]

    def target(c):
        fw, ext = FW[c["repr"]]
        return kdir / f"{k}_numba_np.py" if fw == "numba" else backend / f"{k}_fp64{ext}"

    try:
        for rnd, pos, c in schedule:
            if any(r["kernel"] == k and r["label"] == c["label"] and r["round"] == rnd for r in rows):
                print(f"  [{rnd}] {c['label']:<16} already recorded", flush=True); continue
            for f, (bak, want) in pristine.items():
                place(bak, f); assert sha(f) == want, f"pristine restore mismatch {f}"
            timed = target(c)
            if c["src"]:
                place(HERE / c["src"], timed)
            else:
                os.utime(timed)
            t32 = timed.with_name(timed.name.replace("_fp64", "_fp32"))
            if c["src32"]:
                place(HERE / c["src32"], t32)
            want = sha(timed)
            want32 = sha(t32) if c["src32"] else ""
            reps = ARGS.record_reps if rnd == "record" else ARGS.reps
            st, samples, note = run_framework(k, c, reps, "record" if rnd == "record" else "time")
            if sha(timed) != want or (c["src32"] and sha(t32) != want32):
                st, samples, note = "build_error", [], f"source in place changed during the run ({timed.name})"
            if rnd != "record" and rnd == "1" and st == "ok":
                save_so_asm(k, c)
            if rnd == "record" and LAST["extra"].get("perf_data") and st == "ok":
                save_profile(k, c, LAST["extra"].pop("perf_data"))
            ns = [int(round(s * 1e6)) for s in samples]
            prov = "autogen" if MARK in timed.read_text().splitlines()[0] else "hand"
            rows.append({
                "kernel": k, "label": c["label"], "representation": c["repr"], "round": rnd, "position": pos,
                "preset": ARGS.preset, "status": st,
                "time_ns_median": int(statistics.median(ns)) if ns else "", "time_ns_min": min(ns) if ns else "",
                "time_ns_all": " ".join(map(str, ns)),
                "rsd_pct": round(100 * statistics.stdev(ns) / statistics.mean(ns), 2) if len(ns) > 1 else "",
                "compiler_resolved": vers[c["label"]][0], "compiler_version": vers[c["label"]][1],
                "flags": FLAGS.get({"cc": "c", "cpp": "cpp", "fortran": "fortran"}.get(FW[c["repr"]][0], ""),
                                   "@nb.njit as written in the numba file"),
                "override": " ".join(x for x in (c["src"], c["src32"]) if x), "bin_prefix": c["bin"], "cell_env": " ".join(f"{k}={v}" for k, v in c["env"].items()), "source_sha256": want, "provenance": prov,
                "notes": note, "slurm_job": f"{os.environ.get('SLURM_JOB_PARTITION', '')}/{os.environ.get('SLURM_JOB_ID', '')}",
                "node": os.uname().nodename, "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S%z"), "commit_hash": COMMIT,
                **LAST["extra"]})
            flush(rows)
            print(f"  [{rnd}] {c['label']:<16} {st:<12} "
                  f"{'min %.3f ms' % (min(ns) / 1e6) if ns else ''} {note[:120]}", flush=True)
            if st != "ok":
                d = HERE / "logs" / "cells"; d.mkdir(parents=True, exist_ok=True)
                (d / f"{k}.{c['label']}.{rnd}.log").write_text(LAST["log"])
    finally:
        for f, (bak, want) in pristine.items():
            place(bak, f)
    print(f"wrote {len(rows)} rows -> {ARGS.out}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--kernel", required=True)
    ap.add_argument("--cells", nargs="+", required=True, help="LABEL[=REPR][:src=PATH][:bin=DIR]")
    ap.add_argument("--rounds", type=int, default=3)
    ap.add_argument("--preset", default="M")
    ap.add_argument("--reps", type=int, default=30)
    ap.add_argument("--warmup", type=int, default=5)
    ap.add_argument("--record-reps", type=int, default=0)
    ap.add_argument("--perf", action="store_true")
    ap.add_argument("--timeout", type=int, default=1500)
    ap.add_argument("--out", required=True)
    ap.add_argument("--scratch", required=True)
    ap.add_argument("--db-shard", type=int, required=True)
    ARGS = ap.parse_args()
    for _a in ("out", "scratch"):
        setattr(ARGS, _a, str(pathlib.Path(getattr(ARGS, _a)).resolve()))
    pathlib.Path(ARGS.scratch).mkdir(parents=True, exist_ok=True)
    pathlib.Path(ARGS.out).parent.mkdir(parents=True, exist_ok=True)
    COMMIT = subprocess.run(["git", "-C", str(BENCH), "rev-parse", "HEAD"], capture_output=True, text=True).stdout.strip()
    _probe = subprocess.run(["numactl", "--cpunodebind=0", "--membind=0", PY, "-c",
                             "import json;from hpcagent_bench import languages as L;"
                             "print(json.dumps({l: L.baseline_flags(l) for l in ('c','cpp','fortran')}))"],
                            cwd=BENCH, env=env_for({"bin": ""}, False), capture_output=True, text=True)
    FLAGS = json.loads(_probe.stdout.strip().splitlines()[-1])
    main()
