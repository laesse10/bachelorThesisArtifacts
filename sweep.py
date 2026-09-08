#!/usr/bin/env python3
"""LLR-40 measurement matrix: one row per (kernel, representation) cell.

Protocol is pinned in protocol.md and is NOT re-decided here. Every representation is timed by
the SAME harness path (hpcagent_bench.cli run -> frameworks.measure -> warmup+repeat, warmup
discarded), so the C-family columns differ only in which source bytes are in place.

c / cpp / fortran / numba : the autogen lowerings, as emitted.
c_reference               : the hand-ported TSVC <k>_reference.c, substituted into cpp_backend.
agent                     : the campaign submission, substituted the same way, used AS DELIVERED.

Source substitution is safe because a file lacking the ``hpcagent_bench-autogen`` marker is
treated by the emitter as a hand override and is never regenerated (autogen.py). The pristine
autogen bytes are snapshotted and restored around every substitution, and verified by sha256.
"""
import argparse, csv, hashlib, json, os, pathlib, shutil, statistics, subprocess, sys, time

REPO = pathlib.Path(__file__).resolve().parent
# Overridable so this runs off-site. Defaults reproduce the recorded runs on CSCS daint.
#   LLR40_BENCH  -- checkout of spcl/HPCAgent-Bench at the pinned commit (see REPRODUCE.md)
#   LLR40_PYTHON -- interpreter with requirements-frozen.txt installed
BENCH = pathlib.Path(os.environ.get("LLR40_BENCH", REPO / "bench")).resolve()
BENCHMARKS = BENCH / "hpcagent_bench/benchmarks/loop_level_reasoning"
PY = os.environ.get("LLR40_PYTHON", sys.executable)

# representation -> (framework name, extension of the source it consumes)
REPRS = {
    "c":           ("cc",      ".c"),
    "c_reference": ("cc",      ".c"),
    "cpp":         ("cpp",     ".cpp"),
    "fortran":     ("fortran", ".f90"),
    "numba":       ("numba",   None),
    "agent":       (None,      None),   # framework/ext depend on the delivered language
}
LANG_FW = {"c": ("cc", ".c"), "cpp": ("cpp", ".cpp"), "fortran": ("fortran", ".f90")}

FIELDS = ["kernel", "representation", "preset", "status", "time_ns_median", "time_ns_min", "time_ns_all", "compiler",
          "compiler_version", "flags", "threads", "n_warmup", "n_reps", "commit_hash",
          "timestamp", "notes"]


def sha(p):
    return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()


def env_for_run():
    e = dict(os.environ)
    e.update({
        "PYTHONPATH": f"{BENCH}:{BENCH}/hpcagent_bench/numpy_translators/src",
        "OMP_NUM_THREADS": "1", "OMP_PLACES": "cores", "OMP_PROC_BIND": "close",
        "NUMBA_NUM_THREADS": "1",
        "HPCAGENT_BENCH_MEASUREMENT_WARMUP": str(ARGS.warmup),
    })
    return e


def run_framework(kernel, framework, preset, reps, timeout_s):
    """Invoke the harness driver for one cell. Returns (status, samples_ms, note)."""
    out = pathlib.Path(ARGS.scratch) / f"{kernel}.{framework}.{os.getpid()}.jsonl"
    out.unlink(missing_ok=True)
    # Pin to ONE Grace socket (NUMA node 0: cpus 0-71, ~122 GB local). Without this the single
    # timing process migrates across the 4 sockets mid-series and the 2-3 GB working set becomes
    # remote: the pilot saw contiguous blocks of samples jump 120 ms -> 190 ms (RSD 23%). That is
    # a binding artifact, not a property of the representation. OMP_PLACES/OMP_PROC_BIND cannot
    # fix it -- a single-threaded call into a .so is not an OpenMP parallel region.
    cmd = ["numactl", "--cpunodebind=0", "--membind=0",
           PY, "-m", "hpcagent_bench.cli", "run", "--benchmark", kernel,
           "--framework", framework, "--precision", "fp64", "--preset", preset,
           "--mode", "single_core", "--repeat", str(reps), "--validate",
           "--output", str(out)]
    try:
        p = subprocess.run(cmd, cwd=BENCH, env=env_for_run(), capture_output=True,
                           text=True, timeout=timeout_s)
    except subprocess.TimeoutExpired:
        return "timeout", [], f"exceeded {timeout_s}s wall budget"
    if not out.is_file():
        tail = (p.stderr or p.stdout or "").strip().splitlines()
        return "build_error", [], " | ".join(tail[-4:])[:900] or f"exit {p.returncode}, no output"
    rows = [json.loads(l) for l in out.read_text().splitlines() if l.strip()]
    rows = [r for r in rows if r.get("framework") == framework]
    if not rows:
        return "build_error", [], "driver wrote no row for this framework"
    r = rows[-1]
    impls = r.get("impls") or {}
    if not impls:
        tail = (p.stderr or "").strip().splitlines()
        return ("build_error", [], " | ".join(tail[-4:])[:900] or f"status={r.get('status')}")
    impl = list(impls.values())[0]
    samples = impl.get("time_python") or []
    if impl.get("validated") is False:
        return "incorrect", samples, "output did not match the NumPy reference"
    if not samples:
        return "build_error", [], f"no timings; driver status={r.get('status')}"
    return "ok", samples, ""


def emit(rows, kernel, repr_name, status, samples_ms, note, compiler, flags):
    ns = [int(round(s * 1e6)) for s in samples_ms]   # driver reports milliseconds
    rows.append({
        "kernel": kernel, "representation": repr_name, "preset": ARGS.preset, "status": status,
        "time_ns_median": int(statistics.median(ns)) if ns else "",
        # min-of-k: robust to the diagnosed external-bandwidth artifact (see protocol.md).
        "time_ns_min": min(ns) if ns else "",
        "time_ns_all": " ".join(map(str, ns)),
        "compiler": compiler, "compiler_version": VERSIONS.get(compiler, ""),
        "flags": flags, "threads": 1, "n_warmup": ARGS.warmup, "n_reps": len(ns),
        "commit_hash": COMMIT, "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
        "notes": note,
    })
    print(f"    {repr_name:<12} {status:<12} "
          f"{'median %.3f ms' % (statistics.median(samples_ms)) if samples_ms else ''} {note[:70]}",
          flush=True)


def main():
    kernels = [l.strip() for l in open(ARGS.kernels) if l.strip()] if pathlib.Path(ARGS.kernels).is_file() \
        else ARGS.kernels.split(",")
    picks = json.load(open(REPO / "agent_picks.json"))
    rows = []
    for n, k in enumerate(kernels, 1):
        print(f"[{n}/{len(kernels)}] {k}", flush=True)
        kdir, backend = BENCHMARKS / k, BENCHMARKS / k / "cpp_backend"
        # 1. generate the lowerings + numba sibling
        gen = subprocess.run(
            [PY, "-c", "import sys,hpcagent_bench.autogen as A;"
                       "A.ensure_native(sys.argv[1]);A.ensure(sys.argv[1],['numba_np'])",
             f"loop_level_reasoning/{k}"],
            cwd=BENCH, env=env_for_run(), capture_output=True, text=True)
        if gen.returncode != 0:
            note = " | ".join((gen.stderr or "").strip().splitlines()[-3:])[:900]
            for r in REPRS:
                emit(rows, k, r, "build_error", [], f"autogen failed: {note}", "", "")
            continue
        # 2. snapshot pristine autogen bytes
        pristine = {}
        for ext in (".c", ".cpp", ".f90"):
            src = backend / f"{k}_fp64{ext}"
            if src.is_file():
                bak = pathlib.Path(ARGS.scratch) / f"pristine.{k}_fp64{ext}"
                shutil.copy2(src, bak); pristine[ext] = (src, bak, sha(src))
        try:
            for rep in ("c", "cpp", "fortran", "numba", "c_reference", "agent"):
                fw, ext = REPRS[rep]
                note, compiler = "", {"cc": "gcc", "cpp": "g++", "fortran": "gfortran",
                                      "numba": "numba"}.get(fw, "")
                # --- restore pristine before every cell, and verify it ---
                for e, (src, bak, want) in pristine.items():
                    shutil.copy2(bak, src)
                    assert sha(src) == want, f"pristine restore mismatch for {e}"
                if rep == "c_reference":
                    hand = kdir / f"{k}_reference.c"
                    if not hand.is_file():
                        emit(rows, k, rep, "unsupported", [], "no hand-written _reference.c in the corpus", "gcc", FLAGS["c"]); continue
                    shutil.copy2(hand, backend / f"{k}_fp64.c")
                    note = f"source={hand.name} sha256={sha(hand)[:16]}"
                elif rep == "agent":
                    cands = picks.get(k) or []
                    if not cands:
                        emit(rows, k, rep, "unsupported", [], "no agent submission in either campaign", "", ""); continue
                    chosen = last = None
                    for c in cands[:ARGS.agent_tries]:
                        lang = c["language"]
                        if lang not in LANG_FW:
                            continue
                        fw, ext = LANG_FW[lang]
                        shutil.copy2(c["path"], backend / f"{k}_fp64{ext}")
                        st, samples, nt = run_framework(k, fw, ARGS.preset, ARGS.reps, ARGS.timeout)
                        if st == "ok":
                            chosen = (c, st, samples, nt); break
                        last = (c, st, samples, nt)
                        # restore before trying the next candidate
                        for e, (src, bak, want) in pristine.items():
                            shutil.copy2(bak, src)
                    if chosen is None and last is None:
                        # every candidate was in a language this matrix has no column for
                        emit(rows, k, rep, "unsupported", [],
                             f"{len(cands)} candidate(s), none in a supported language", "", "")
                        continue
                    c, st, samples, nt = chosen or last
                    compiler = {"c": "gcc", "cpp": "g++", "fortran": "gfortran"}[c["language"]]
                    note = (f"arm={c['arm']} job={c['job']} seq={c['seq']} lang={c['language']} "
                            f"campaign_speedup={c['campaign_speedup']:.3f} sha256={c['sha256'][:16]}"
                            + (f" | {nt}" if nt else ""))
                    emit(rows, k, rep, st, samples, note, compiler, FLAGS[c["language"]])
                    continue
                st, samples, nt = run_framework(k, fw, ARGS.preset, ARGS.reps, ARGS.timeout)
                lang = {"cc": "c", "cpp": "cpp", "fortran": "fortran"}.get(fw, "numba")
                emit(rows, k, rep, st, samples, "; ".join(x for x in (note, nt) if x),
                     compiler, FLAGS.get(lang, "@nb.njit(parallel=True, cache=True)"))
        finally:
            for e, (src, bak, want) in pristine.items():
                shutil.copy2(bak, src)
        # flush after every kernel: the sweep must survive a wall-clock kill
        with open(ARGS.out, "w", newline="") as fh:
            w = csv.DictWriter(fh, fieldnames=FIELDS); w.writeheader(); w.writerows(rows)
    print(f"\nwrote {len(rows)} rows -> {ARGS.out}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--kernels", required=True, help="roster file or comma-list")
    ap.add_argument("--preset", default="M")
    ap.add_argument("--reps", type=int, default=30)
    ap.add_argument("--warmup", type=int, default=5)
    ap.add_argument("--timeout", type=int, default=1800)
    ap.add_argument("--agent-tries", type=int, default=5)
    ap.add_argument("--out", default="results.csv")
    ap.add_argument("--scratch", default="/tmp/claude-3042/sweep")
    ARGS = ap.parse_args()
    pathlib.Path(ARGS.scratch).mkdir(parents=True, exist_ok=True)
    COMMIT = subprocess.run(["git", "-C", str(BENCH), "rev-parse", "HEAD"],
                            capture_output=True, text=True).stdout.strip()
    def ver(c):
        try: return subprocess.run([c, "--version"], capture_output=True, text=True).stdout.splitlines()[0]
        except Exception: return ""
    VERSIONS = {"gcc": ver("gcc"), "g++": ver("g++"), "gfortran": ver("gfortran")}
    import numba as _nb  # noqa
    VERSIONS["numba"] = f"numba {_nb.__version__}"
    sys.path[:0] = [str(BENCH), str(BENCH / "hpcagent_bench/numpy_translators/src")]
    from hpcagent_bench import languages
    FLAGS = {l: languages.baseline_flags(l) for l in ("c", "cpp", "fortran")}
    main()
