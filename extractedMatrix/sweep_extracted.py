#!/usr/bin/env python3
"""Extracted-kernel matrix (Part A): one row per (kernel, column) cell, the LLR-40 v2 protocol.

Derived from llr40Matrix/v2/sweep.py. The measurement path is unchanged: every cell runs
``hpcagent_bench.cli run --benchmark K --framework F --precision fp64 --preset M --mode single_core
--repeat 30 --validate`` under ``numactl --cpunodebind=0 --membind=0`` with 5 warm-up reps
(``HPCAGENT_BENCH_MEASUREMENT_WARMUP``), and is validated against the NumPy oracle before it is
timed. The changes from v2's sweep.py:

* kernels live under ``benchmarks/scientific_computing/<area>/<folder>/`` (looked up by manifest);
* columns are c / cpp / fortran / numba (labelled ``numba_hand`` when the numba file is a hand-written
  override) / native. There is no c_reference or agent column;
* ``native`` substitutes a THIN ADAPTER (extractedMatrix/adapters/) as the ``cpp`` framework's
  source. The adapter includes the hand-written upstream extraction unchanged and only maps
  arguments. It is re-checked by sha256 after the run. Kernels without a timeable extraction get an
  ``unsupported`` row with the reason;
* added columns: provenance (autogen / hand, from the marker on the timed source's first line),
  adapter, source_sha256, declared dtypes, graded config, node;
* the harness results DB is sharded per unit (``HPCAGENT_BENCH_DB_SHARD``), the harness's own knob.
  Every srun step has SLURM_PROCID 0, so concurrent nodes would otherwise share one SQLite file
  and hit "database is locked" (LLR-40 follow-up 2b, job 4990019);
* v2's step 0 deletes unmarked cpp_backend sources left by a killed run. It never touches the numba
  file, because two of these kernels have a hand-written, git-tracked numba override.
"""
import argparse, csv, hashlib, json, os, pathlib, re, shutil, statistics, subprocess, sys, time

REPO = pathlib.Path(__file__).resolve().parent                      # extractedMatrix/
BENCH = pathlib.Path(os.environ.get("LLR40_BENCH", REPO / "bench")).resolve()
SC = BENCH / "hpcagent_bench/benchmarks/scientific_computing"
PY = os.environ.get("LLR40_PYTHON", sys.executable)
MARK = "hpcagent_bench-autogen"

COLUMNS = ["c", "cpp", "fortran", "numba", "native"]
FW = {"c": ("cc", ".c"), "cpp": ("cpp", ".cpp"), "fortran": ("fortran", ".f90"), "numba": ("numba", None),
      "native": ("cpp", ".cpp")}
COMPILER = {"cc": "gcc", "cpp": "g++", "fortran": "gfortran", "numba": "numba"}
# native: kernel -> adapter (relative to extractedMatrix/), or the reason it is unsupported
NATIVE = {
    "comet_int4_gemm": ("adapters/comet_int4_gemm_native.cpp",
                        "tests/ports/comet_int4_gemm/comet_int4_gemm_ref.cpp"),
    "warpx_boris_push": ("adapters/warpx_boris_push_native.cpp", "<kernel>/warpx_boris_push_reference.cpp"),
}
NATIVE_UNSUPPORTED = {
    "quatrex_rgf": "upstream extraction is Python (quatrex_rgf_reference.py), not a C-ABI function",
    "spgemm_hash": "upstream extraction is CUDA (spgemm_hash_reference.cu)",
    "triangle_count": "upstream extraction is CUDA (triangle_count_reference.cu)",
    "nfa_frontier": "nfa_frontier_reference.cpp is an excerpt of VASim's Automata class methods, not a "
                    "function with the harness ABI",
    "warpx_esirkepov_deposition": "warpx_esirkepov_deposition_original needs the grid extents n1, n2, ncomp, "
                                  "m1, m2, which the harness ABI does not pass; an adapter would have to "
                                  "compute them from ncells/depos_order/n_rz_azimuthal_modes (arithmetic)",
    "warpx_field_gather": "warpx_field_gather_original needs the grid extents n0, n1, n2, ncomp (arithmetic "
                          "from ncells/depos_order) and geom/galerkin_interpolation/n_rz_azimuthal_modes, "
                          "which the harness ABI does not pass",
}

FIELDS = ["kernel", "representation", "preset", "status", "time_ns_median", "time_ns_min", "time_ns_all", "compiler",
          "compiler_version", "flags", "threads", "n_warmup", "n_reps", "commit_hash", "timestamp", "notes",
          "slurm_job",
          # extractedMatrix columns
          "provenance", "adapter", "source", "source_sha256", "declared_dtypes", "graded_config", "node"]


def place(src, dst):
    """copy2 + touch. The harness reuses lib<k>_<fw>.so while it is NEWER than every source
    (cpp_runtime._ensure_built); copy2 keeps the source's old mtime, so without the touch a
    substituted or restored source could be shadowed by the previous cell's library."""
    shutil.copy2(src, dst)
    os.utime(dst)


def sha(p):
    return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()


def kernel_dir(k):
    hits = sorted(p.parent for p in SC.glob(f"**/{k}.yaml") if ".cache" not in p.parts)
    if len(hits) != 1:
        raise SystemExit(f"{k}: expected one manifest under {SC}, found {hits}")
    return hits[0]


def manifest_facts(k, kdir):
    import yaml
    m = yaml.safe_load(open(kdir / f"{k}.yaml"))
    arrays = (m.get("init") or {}).get("arrays") or {}
    dt = {}
    for name, v in arrays.items():
        dt[name] = v.get("dtype", "float64") if isinstance(v, dict) else "float64"
    cfg = {}
    for name, v in (m.get("config") or {}).items():
        cfg[name] = v["domain"][0] if "domain" in v else v.get("value")
    return " ".join(f"{a}:{d}" for a, d in dt.items()), " ".join(f"{a}={c}" for a, c in cfg.items())


def provenance(path):
    first = pathlib.Path(path).read_text().splitlines()[0] if pathlib.Path(path).is_file() else ""
    return "autogen" if MARK in first else "hand"


def env_for_run():
    e = dict(os.environ)
    e.update({
        "PYTHONPATH": f"{BENCH}:{BENCH}/hpcagent_bench/numpy_translators/src",
        "OMP_NUM_THREADS": "1", "OMP_PLACES": "cores", "OMP_PROC_BIND": "close",
        "NUMBA_NUM_THREADS": "1",
        "HPCAGENT_BENCH_MEASUREMENT_WARMUP": str(ARGS.warmup),
        "HPCAGENT_BENCH_DB_SHARD": str(ARGS.db_shard),
    })
    return e


def run_framework(kernel, framework, preset, reps, timeout_s):
    """Invoke the harness driver for one cell. Returns (status, samples_ms, note)."""
    out = pathlib.Path(ARGS.scratch) / f"{kernel}.{framework}.{os.getpid()}.jsonl"
    out.unlink(missing_ok=True)
    # Pin to ONE Grace socket (NUMA node 0), as in v2: a single timing process otherwise migrates
    # across sockets mid-series and its working set becomes remote.
    cmd = ["numactl", "--cpunodebind=0", "--membind=0",
           PY, "-m", "hpcagent_bench.cli", "run", "--benchmark", kernel,
           "--framework", framework, "--precision", "fp64", "--preset", preset,
           "--mode", "single_core", "--repeat", str(reps), "--validate",
           "--output", str(out)]
    try:
        p = subprocess.run(cmd, cwd=BENCH, env=env_for_run(), capture_output=True, text=True, timeout=timeout_s)
    except subprocess.TimeoutExpired as e:
        LAST_LOG[0] = f"$ {' '.join(cmd)}\n[timeout after {timeout_s}s]\n{e.stdout or ''}\n{e.stderr or ''}"
        return "timeout", [], f"exceeded {timeout_s}s wall budget"
    LAST_LOG[0] = f"$ {' '.join(cmd)}\n[exit {p.returncode}]\n--- stdout\n{p.stdout}\n--- stderr\n{p.stderr}"
    if not out.is_file():
        return "build_error", [], first_error_line(p.stderr or p.stdout or f"exit {p.returncode}, no output")
    rows = [json.loads(l) for l in out.read_text().splitlines() if l.strip()]
    rows = [r for r in rows if r.get("framework") == framework]
    if not rows:
        return "build_error", [], "driver wrote no row for this framework"
    r = rows[-1]
    impls = r.get("impls") or {}
    if not impls:
        reason = r.get("reason") or ""
        return "build_error", [], (first_error_line(p.stderr or "") or f"status={r.get('status')} {reason}")[:900]
    impl = list(impls.values())[0]
    samples = impl.get("time_python") or []
    if impl.get("validated") is False:
        cerr = COMPILE_ERR.search(p.stderr or "")
        if cerr and "_ensure_built" in (p.stderr or ""):
            return "build_error", [], cerr.group(0).strip()[:600]
        # numba compiles at the first call: a typing/lowering failure is a build failure, not a
        # wrong answer (no output was ever produced)
        nerr = re.search(r"numba\.core\.errors\.(TypingError|LoweringError|UnsupportedError)[^\n]*(\n[^\n]*){0,3}", p.stderr or "")
        if nerr and not samples:
            return "build_error", [], " ".join(nerr.group(0).split())[:600]
        return "incorrect", samples, ("output did not match the NumPy reference | "
                                      + mismatch_line(p.stdout + "\n" + p.stderr))[:900]
    if not samples:
        return "build_error", [], f"no timings; driver status={r.get('status')}"
    return "ok", samples, ""


COMPILE_ERR = re.compile(r"^.*:\d+(:\d+)?: (fatal )?[Ee]rror:.*$|^\s*Error: .*$|^.*undefined reference.*$", re.M)
LAST_LOG = [""]


def first_error_line(text):
    m = COMPILE_ERR.search(text)
    if m:
        return m.group(0).strip()[:600]
    for pat in (r"Error\b", r"\berror\b", r"Traceback", r"Exception"):
        for l in text.splitlines():
            if re.search(pat, l) and not re.search(r"Events (en|dis)abled", l):
                return l.strip()[:600]
    return " | ".join(text.strip().splitlines()[-3:])[:600]


def mismatch_line(text):
    for l in text.splitlines():
        if re.search(r"mismatch|did not validate|max (abs|rel)|tolerance", l, re.I):
            return l.strip()[:500]
    return ""


def flush(rows):
    """Written after EVERY cell, so a job killed at its wall limit loses at most one cell."""
    tmp = pathlib.Path(str(ARGS.out) + ".tmp")
    with open(tmp, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=FIELDS, extrasaction="ignore"); w.writeheader(); w.writerows(rows)
    tmp.replace(ARGS.out)


def emit(rows, k, rep, status, samples_ms, note, compiler, flags, extra):
    ns = [int(round(s * 1e6)) for s in samples_ms]   # driver reports milliseconds
    rows.append({
        "kernel": k, "representation": rep, "preset": ARGS.preset, "status": status,
        "time_ns_median": int(statistics.median(ns)) if ns else "",
        "time_ns_min": min(ns) if ns else "",            # min-of-k, the protocol's reducer
        "time_ns_all": " ".join(map(str, ns)),
        "compiler": compiler, "compiler_version": VERSIONS.get(compiler, ""),
        "flags": flags, "threads": 1, "n_warmup": ARGS.warmup, "n_reps": len(ns),
        "commit_hash": COMMIT, "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S%z"), "notes": note,
        "slurm_job": f"{os.environ.get('SLURM_JOB_PARTITION', '')}/{os.environ.get('SLURM_JOB_ID', '')}",
        "node": os.uname().nodename, **extra,
    })
    flush(rows)
    print(f"    {rep:<12} {status:<12} "
          f"{'median %.3f ms' % statistics.median(samples_ms) if samples_ms else ''} {note[:90]}", flush=True)
    if status not in ("ok", "unsupported") and LAST_LOG[0]:
        d = pathlib.Path(ARGS.logs); d.mkdir(parents=True, exist_ok=True)
        (d / f"{k}.{rep}.log").write_text(LAST_LOG[0])


def main():
    kernels = ARGS.kernels.split(",")
    rows = list(csv.DictReader(open(ARGS.out))) if pathlib.Path(ARGS.out).is_file() else []
    for n, k in enumerate(kernels, 1):
        print(f"[{n}/{len(kernels)}] {k}", flush=True)
        kdir = kernel_dir(k)
        backend = kdir / "cpp_backend"
        dtypes, cfg = manifest_facts(k, kdir)
        # 0. v2: a run killed mid-cell leaves a substituted source in cpp_backend; remove any unmarked
        # fp64 source there (never the numba file: two kernels ship a hand-written numba override).
        for ext in (".c", ".cpp", ".f90"):
            f = backend / f"{k}_fp64{ext}"
            if f.is_file() and MARK not in f.read_text().splitlines()[0]:
                print(f"    removing leftover non-autogen {f.name} before generation", flush=True)
                f.unlink()
        # 1. generate the lowerings + numba sibling (an existing hand override is left as it is)
        gen = subprocess.run(
            [PY, "-c", "import sys,hpcagent_bench.autogen as A;A.ensure_native(sys.argv[1]);A.ensure(sys.argv[1],['numba_np'])", k],
            cwd=BENCH, env=env_for_run(), capture_output=True, text=True)
        if gen.returncode != 0:
            note = " | ".join((gen.stderr or "").strip().splitlines()[-3:])[:900]
            for rep in ARGS.reprs.split(","):
                emit(rows, k, rep, "build_error", [], f"autogen failed: {note}", "", "",
                     {"declared_dtypes": dtypes, "graded_config": cfg})
            continue
        # 2. snapshot the pristine generated sources
        pristine = {}
        for ext in (".c", ".cpp", ".f90"):
            src = backend / f"{k}_fp64{ext}"
            if src.is_file():
                if MARK not in src.read_text().splitlines()[0]:
                    raise SystemExit(f"{src.name}: snapshot is not autogen output; refusing to time it")
                bak = pathlib.Path(ARGS.scratch) / f"pristine.{src.name}"
                shutil.copy2(src, bak); pristine[ext] = (src, bak, sha(src))
        keep = REPO / ARGS.emitted_dir / k; keep.mkdir(parents=True, exist_ok=True)
        for f in [backend / f"{k}_fp64{e}" for e in (".c", ".cpp", ".f90")] + [kdir / f"{k}_numba_np.py"]:
            if f.is_file():
                shutil.copy2(f, keep / f.name)
        try:
            for rep in ARGS.reprs.split(","):
                fw, ext = FW[rep]
                numba_file = kdir / f"{k}_numba_np.py"
                label = "numba_hand" if (rep == "numba" and provenance(numba_file) == "hand") else rep
                if any(r["kernel"] == k and r["representation"] == label for r in rows):
                    print(f"    {label:<12} already recorded (resume)", flush=True); continue
                for e, (src, bak, want) in pristine.items():       # pristine before every cell
                    place(bak, src)
                    assert sha(src) == want, f"pristine restore mismatch for {e}"
                extra = {"declared_dtypes": dtypes, "graded_config": cfg, "adapter": ""}
                if rep == "native":
                    if k not in NATIVE:
                        emit(rows, k, rep, "unsupported", [], NATIVE_UNSUPPORTED[k], "", "",
                             {**extra, "provenance": "hand"})
                        continue
                    adapter, orig = NATIVE[k]
                    orig_path = (kdir / orig.split("/", 1)[1]) if orig.startswith("<kernel>/") else BENCH / orig
                    timed = backend / f"{k}_fp64.cpp"
                    place(REPO / adapter, timed)
                    extra.update(provenance="hand", adapter=f"extractedMatrix/{adapter}",
                                 source=str(orig_path.relative_to(BENCH)) + " via " + adapter,
                                 source_sha256=f"adapter {sha(timed)[:16]} original {sha(orig_path)[:16]}")
                elif rep == "numba":
                    timed = numba_file
                    extra.update(provenance=provenance(timed), source=str(timed.relative_to(BENCH)),
                                 source_sha256=sha(timed))
                else:
                    timed = backend / f"{k}_fp64{ext}"
                    extra.update(provenance=provenance(timed), source=str(timed.relative_to(BENCH)),
                                 source_sha256=sha(timed))
                want_sha = sha(timed)
                st, samples, note = run_framework(k, fw, ARGS.preset, ARGS.reps, ARGS.timeout)
                if sha(timed) != want_sha:
                    st, samples, note = "build_error", [], f"source in place changed during the run ({timed.name})"
                lang = {"cc": "c", "cpp": "cpp", "fortran": "fortran"}.get(fw)
                emit(rows, k, label, st, samples, note, COMPILER[fw],
                     FLAGS.get(lang, "@nb.njit as written in the numba file"), extra)
        finally:
            for e, (src, bak, want) in pristine.items():
                place(bak, src)
        flush(rows)
    print(f"\nwrote {len(rows)} rows -> {ARGS.out}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--kernels", required=True, help="comma-list")
    ap.add_argument("--reprs", default=",".join(COLUMNS))
    ap.add_argument("--preset", default="M")
    ap.add_argument("--reps", type=int, default=30)
    ap.add_argument("--warmup", type=int, default=5)
    ap.add_argument("--timeout", type=int, default=1800)
    ap.add_argument("--out", required=True)
    ap.add_argument("--scratch", required=True)
    ap.add_argument("--logs", default="logs/cells", help="full driver output of every non-ok cell")
    ap.add_argument("--emitted-dir", default="emitted_sources")
    ap.add_argument("--db-shard", type=int, required=True, help="private harness results-DB shard")
    ARGS = ap.parse_args()
    for _a in ("out", "scratch", "logs"):
        setattr(ARGS, _a, str(pathlib.Path(getattr(ARGS, _a)).resolve()))
    pathlib.Path(ARGS.scratch).mkdir(parents=True, exist_ok=True)
    COMMIT = subprocess.run(["git", "-C", str(BENCH), "rev-parse", "HEAD"], capture_output=True, text=True).stdout.strip()

    def ver(c):
        try: return subprocess.run([c, "--version"], capture_output=True, text=True).stdout.splitlines()[0]
        except Exception: return ""
    sys.path[:0] = [str(BENCH), str(BENCH / "hpcagent_bench/numpy_translators/src")]
    from hpcagent_bench import languages as _L
    DRIVERS = {n: _L.resolve_compiler(n) or n for n in ("gcc", "g++", "gfortran")}
    VERSIONS = {n: f"{ver(d)} [{d}]" for n, d in DRIVERS.items()}
    import numba as _nb  # noqa
    VERSIONS["numba"] = f"numba {_nb.__version__}"
    # flags as the build child resolves them (same numactl binding)
    _probe = subprocess.run(["numactl", "--cpunodebind=0", "--membind=0", PY, "-c",
                             "import json;from hpcagent_bench import languages as L;"
                             "print(json.dumps({l: L.baseline_flags(l) for l in ('c','cpp','fortran')}))"],
                            cwd=BENCH, env=env_for_run(), capture_output=True, text=True)
    FLAGS = json.loads(_probe.stdout.strip().splitlines()[-1])
    main()
