#!/usr/bin/env python3
"""LLR-40 v2 FOLLOW-UP runs: v2/sweep_variant.py (git 632952d) plus what the four follow-up
experiments need, and nothing that changes the measurement path:

  --variant NAME         written to a `variant` column (variant rows never enter the matrix)
  --override REPR=PATH   time PATH as REPR's source (c/cpp/fortran/numba: in place of the emitted
                         lowering; c_reference: in place of <k>_reference.c). Restored afterwards,
                         and the bytes in place are re-checked after the run (not regenerated).
  --ordered              run --reprs in the order given (interleaved rounds), not the fixed order
  --round N              written to a `round` column; `position` is the cell's place in the order
  --agent-sha PREFIX     time only the agent candidate whose sha256 starts with PREFIX (v2's pick)
  --perf                 perf stat -e cycles,instructions over the TIMED reps only (perfgate/)
  --vmstat               /proc/vmstat counter deltas around each series (node memory state)

Identical measurement path to sweep.py.

LLR-40 measurement matrix: one row per (kernel, representation) cell.

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
import argparse, csv, hashlib, json, os, pathlib, re, shutil, statistics, subprocess, sys, time

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

FIELDS = ["kernel", "representation", "preset", "flags_variant", "status", "time_ns_median", "time_ns_min", "time_ns_all", "compiler",
          "compiler_version", "flags", "threads", "n_warmup", "n_reps", "commit_hash",
          "timestamp", "notes", "slurm_job",
          # follow-up columns
          "variant", "round", "position", "source_sha256", "node",
          "perf_cycles", "perf_instructions", "perf_gated_reps", "perf_raw", "vmstat_delta"]
VMSTAT_KEYS = ("compact_stall", "compact_fail", "compact_success", "pgmigrate_success", "pgmigrate_fail",
               "thp_fault_alloc", "thp_fault_fallback", "thp_collapse_alloc", "numa_pages_migrated",
               "pgfault", "pgmajfault")
LAST_EXTRA = [{}]   # perf + vmstat columns of the most recent run_framework call


def place(src, dst):
    """copy2 + touch. The harness reuses lib<k>_<fw>.so while it is NEWER than every source
    (cpp_runtime._ensure_built); copy2 keeps the source's old mtime, so without the touch a
    substituted or restored source could be shadowed by the previous cell's library."""
    shutil.copy2(src, dst)
    os.utime(dst)


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
    if ARGS.perf:
        e["PYTHONPATH"] = f"{REPO / 'perfgate'}:{e['PYTHONPATH']}"
    return e


def vmstat():
    out = {}
    try:
        for line in open("/proc/vmstat"):
            k, v = line.split()
            if k in VMSTAT_KEYS:
                out[k] = int(v)
    except OSError:
        pass
    return out


def parse_perf(path):
    """`perf stat -x,` lines: value,unit,event,run-time,pct,... -> ({event: value}, raw)."""
    got, raw = {}, []
    for line in pathlib.Path(path).read_text().splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        raw.append(line.strip())
        f = line.split(",")
        if len(f) >= 3:
            ev = f[2].split(":")[0]
            try:
                got[ev] = int(float(f[0]))
            except ValueError:
                got[ev] = f[0]
    return got, " | ".join(raw)


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
    env = env_for_run()
    LAST_EXTRA[0] = {}
    if ARGS.perf:
        # perf stat prepends /usr/lib/perf-core:/usr/bin to its child's PATH, where `g++` is GCC 7.5
        # (diagnostics/diagnose_bimodal.py). `env PATH=...` hands the harness the PATH it has
        # everywhere else, so it resolves the same compilers as every recorded row.
        tag = f"{kernel}.{framework}.{os.getpid()}"
        ctl, ack = (pathlib.Path(ARGS.scratch) / f"perf.{tag}.{x}.fifo" for x in ("ctl", "ack"))
        pout, plog = (pathlib.Path(ARGS.scratch) / f"perf.{tag}.{x}" for x in ("csv", "gate.log"))
        for f in (ctl, ack, pout, plog):
            f.unlink(missing_ok=True)
        os.mkfifo(ctl); os.mkfifo(ack)
        env.update({"FOLLOWUP_PERF_CTL": str(ctl), "FOLLOWUP_PERF_ACK": str(ack),
                    "FOLLOWUP_PERF_LOG": str(plog)})
        cmd = ["perf", "stat", "-x,", "-e", "cycles,instructions", "--delay=-1",
               f"--control=fifo:{ctl},{ack}", "-o", str(pout), "--",
               "env", f"PATH={env['PATH']}"] + cmd
    vm0 = vmstat() if ARGS.vmstat else {}
    try:
        p = subprocess.run(cmd, cwd=BENCH, env=env, capture_output=True,
                           text=True, timeout=timeout_s)
    except subprocess.TimeoutExpired as e:
        LAST_LOG[0] = f"$ {' '.join(cmd)}\n[timeout after {timeout_s}s]\n{e.stdout or ''}\n{e.stderr or ''}"
        return "timeout", [], f"exceeded {timeout_s}s wall budget"
    if ARGS.vmstat:
        vm1 = vmstat()
        LAST_EXTRA[0]["vmstat_delta"] = " ".join(f"{k}={vm1[k] - vm0[k]}" for k in VMSTAT_KEYS
                                                 if k in vm0 and k in vm1)
    if ARGS.perf:
        if pout.is_file():
            got, raw = parse_perf(pout)
            LAST_EXTRA[0].update(perf_cycles=got.get("cycles", ""), perf_instructions=got.get("instructions", ""),
                                 perf_raw=raw)
        gl = plog.read_text().strip().splitlines() if plog.is_file() else []
        LAST_EXTRA[0]["perf_gated_reps"] = ";".join(re.search(r"gated=(\d+)", l).group(1) for l in gl) or "0"
        for f in (ctl, ack):
            f.unlink(missing_ok=True)
    LAST_LOG[0] = f"$ {' '.join(cmd)}\n[exit {p.returncode}]\n--- stdout\n{p.stdout}\n--- stderr\n{p.stderr}"
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
        # v2: the driver also reports validated=False when the library never BUILT (it catches the
        # compiler's CalledProcessError and moves on). v1 recorded those as `incorrect`; the
        # protocol's own status for them is `build_error`.
        cerr = COMPILE_ERR.search(p.stderr or "")
        if cerr and "_ensure_built" in (p.stderr or ""):
            return "build_error", [], f"{cerr.group(0).strip()[:600]} (v1 rule would record: incorrect)"
        return "incorrect", samples, "output did not match the NumPy reference"
    if not samples:
        return "build_error", [], f"no timings; driver status={r.get('status')}"
    return "ok", samples, ""


COMPILE_ERR = re.compile(r"^.*:\d+(:\d+)?: (fatal )?[Ee]rror:.*$|^\s*Error: .*$|^.*undefined reference.*$", re.M)
ATTEMPT_FIELDS = ["kernel", "rank", "arm", "job", "seq", "lang", "campaign_speedup", "sha256",
                  "status", "first_error_line", "chosen", "log"]
LAST_LOG = [""]   # full stdout+stderr of the most recent run_framework call


def first_error_line(status, note, log):
    """The first line of the driver output that says WHY, not the last lines of a traceback."""
    log = log.replace(str(BENCHMARKS) + "/", "")
    if status == "ok":
        return ""
    m = COMPILE_ERR.search(log)
    if m:
        return m.group(0).strip()[:400]
    lines = [l.strip() for l in log.splitlines() if l.strip()]
    pats = {"build_error": (r"\berror\b", r"Error\b", r"undefined reference", r"Traceback"),
            "incorrect": (r"mismatch", r"differ", r"max(imum)?[ _]abs", r"tolerance", r"allclose", r"valid"),
            "timeout": (r"timeout",)}.get(status, (r"error",))
    for pat in pats:
        for l in lines:
            if l.startswith("$ ") or l.startswith("---") or l.startswith("[exit"):
                continue
            if re.search(pat, l, re.I):
                return l[:400]
    return note[:400]


def log_attempt(kernel, rank, c, status, note, chosen):
    logname = ""
    if status != "ok":
        d = pathlib.Path(ARGS.attempt_logs); d.mkdir(parents=True, exist_ok=True)
        logname = f"{kernel}.rank{rank:02d}.{c['sha256'][:12]}.log"
        (d / logname).write_text(LAST_LOG[0])
    ATTEMPTS.append({
        "kernel": kernel, "rank": rank, "arm": c["arm"], "job": c["job"], "seq": c["seq"],
        "lang": c["language"], "campaign_speedup": f"{c['campaign_speedup']:.3f}",
        "sha256": c["sha256"], "status": status,
        "first_error_line": first_error_line(status, note, LAST_LOG[0]) if status != "skipped_language" else note,
        "chosen": int(chosen), "log": logname,
    })
    with open(ARGS.attempts_out, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=ATTEMPT_FIELDS); w.writeheader(); w.writerows(ATTEMPTS)


def flush(rows):
    """Written after EVERY cell: a chained debug job may be killed at its wall limit."""
    tmp = pathlib.Path(str(ARGS.out) + ".tmp")
    with open(tmp, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=FIELDS, extrasaction="ignore"); w.writeheader(); w.writerows(rows)
    tmp.replace(ARGS.out)


def emit(rows, kernel, repr_name, status, samples_ms, note, compiler, flags, position="", src_sha=""):
    ns = [int(round(s * 1e6)) for s in samples_ms]   # driver reports milliseconds
    rows.append({
        "kernel": kernel, "representation": repr_name, "preset": ARGS.preset_label or ARGS.preset,
        "flags_variant": ARGS.flags_variant, "status": status,
        "time_ns_median": int(statistics.median(ns)) if ns else "",
        # min-of-k: robust to the diagnosed external-bandwidth artifact (see protocol.md).
        "time_ns_min": min(ns) if ns else "",
        "time_ns_all": " ".join(map(str, ns)),
        "compiler": compiler, "compiler_version": VERSIONS.get(compiler, ""),
        "flags": flags, "threads": 1, "n_warmup": ARGS.warmup, "n_reps": len(ns),
        "commit_hash": COMMIT, "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
        "notes": note,
        "slurm_job": f"{os.environ.get('SLURM_JOB_PARTITION', '')}/{os.environ.get('SLURM_JOB_ID', '')}",
        "variant": ARGS.variant if repr_name in OVERRIDES or not OVERRIDES else "",
        "round": ARGS.round, "position": position, "source_sha256": src_sha, "node": os.uname().nodename,
        **LAST_EXTRA[0],
    })
    LAST_EXTRA[0] = {}
    flush(rows)
    print(f"    {repr_name:<12} {status:<12} "
          f"{'median %.3f ms' % (statistics.median(samples_ms)) if samples_ms else ''} {note[:70]}",
          flush=True)


def main():
    kernels = [l.strip() for l in open(ARGS.kernels) if l.strip()] if pathlib.Path(ARGS.kernels).is_file() \
        else ARGS.kernels.split(",")
    picks = json.load(open(REPO / "agent_picks.json"))
    # v2 resume (chained 30-min debug jobs): cells already in --out are kept and skipped; a cell
    # interrupted mid-series left no row and is re-run from scratch (fresh warmup).
    rows = list(csv.DictReader(open(ARGS.out))) if pathlib.Path(ARGS.out).is_file() else []
    if pathlib.Path(ARGS.attempts_out).is_file():
        ATTEMPTS.extend(csv.DictReader(open(ARGS.attempts_out)))
    for n, k in enumerate(kernels, 1):
        print(f"[{n}/{len(kernels)}] {k}", flush=True)
        kdir, backend = BENCHMARKS / k, BENCHMARKS / k / "cpp_backend"
        # 0. v2: a previous run killed mid-cell (scancel / wall limit) never reaches the `finally`
        # restore, so a substituted hand/agent source can be left in cpp_backend. The emitter never
        # regenerates a file without its autogen marker, so it would be snapshotted as "pristine"
        # and timed as the c/cpp/fortran column (happened: s115/s311 .f90, wf_triangular .c).
        # Remove any unmarked fp64 source first; the emitter then writes the real lowering.
        # (follow-up: the numba sibling too, since --override can substitute it.)
        for f in [backend / f"{k}_fp64{ext}" for ext in (".c", ".cpp", ".f90")] + [kdir / f"{k}_numba_np.py"]:
            if f.is_file() and "hpcagent_bench-autogen" not in f.read_text().splitlines()[0]:
                print(f"    removing leftover non-autogen {f.name} before generation", flush=True)
                f.unlink()
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
        # 2. snapshot pristine autogen bytes (follow-up: + the numba sibling)
        pristine = {}
        for src in [backend / f"{k}_fp64{e}" for e in (".c", ".cpp", ".f90")] + [kdir / f"{k}_numba_np.py"]:
            if src.is_file():
                bak = pathlib.Path(ARGS.scratch) / f"pristine.{src.name}"
                if "hpcagent_bench-autogen" not in src.read_text().splitlines()[0]:
                    raise SystemExit(f"{src.name}: snapshot is not autogen output; refusing to time it")
                shutil.copy2(src, bak); pristine[src.name] = (src, bak, sha(src))
        # v2: keep the exact emitted sources that were timed (diff_vs_v1, opt reports)
        keep = REPO / ARGS.emitted_dir / k; keep.mkdir(parents=True, exist_ok=True)
        for f in [backend / f"{k}_fp64{e}" for e in (".c", ".cpp", ".f90")] + [kdir / f"{k}_numba_np.py"]:
            if f.is_file():
                shutil.copy2(f, keep / f.name)
        order = ARGS.reprs.split(",") if ARGS.ordered else \
            [r for r in ("c", "cpp", "fortran", "numba", "c_reference", "agent") if r in ARGS.reprs.split(",")]
        try:
            for pos, rep in enumerate(order, 1):
                if any(r["kernel"] == k and r["representation"] == rep for r in rows):
                    print(f"    {rep:<12} already recorded (resume)", flush=True); continue
                fw, ext = REPRS[rep]
                note, compiler = "", {"cc": "gcc", "cpp": "g++", "fortran": "gfortran",
                                      "numba": "numba"}.get(fw, "")
                # --- restore pristine before every cell, and verify it ---
                for e, (src, bak, want) in pristine.items():
                    place(bak, src)
                    assert sha(src) == want, f"pristine restore mismatch for {e}"
                ov = OVERRIDES.get(rep)
                timed = None   # the file whose bytes this cell times
                if rep == "c_reference":
                    hand = ov or kdir / f"{k}_reference.c"
                    if not hand.is_file():
                        emit(rows, k, rep, "unsupported", [], "no hand-written _reference.c in the corpus", "gcc", FLAGS["c"], pos); continue
                    place(hand, backend / f"{k}_fp64.c")
                    timed = backend / f"{k}_fp64.c"
                    note = f"source={hand.name} sha256={sha(hand)[:16]}"
                elif rep in ("c", "cpp", "fortran"):
                    timed = backend / f"{k}_fp64{ext}"
                    if ov:
                        place(ov, timed)
                        note = f"source={ov.name} sha256={sha(ov)[:16]}"
                elif rep == "numba":
                    timed = kdir / f"{k}_numba_np.py"
                    if ov:
                        place(ov, timed)
                        note = f"source={ov.name} sha256={sha(ov)[:16]}"
                elif rep == "agent":
                    allc = picks.get(k) or []
                    cands = [c for c in allc if not ARGS.agent_sha or c["sha256"].startswith(ARGS.agent_sha)]
                    if not cands:
                        emit(rows, k, rep, "unsupported", [], "no agent submission in either campaign", "", "", pos); continue
                    chosen = last = None
                    for c in cands[:ARGS.agent_tries]:
                        rank = allc.index(c) + 1
                        lang = c["language"]
                        prior = [a for a in ATTEMPTS if a["kernel"] == k and str(a["rank"]) == str(rank)
                                 and a["status"] != "ok"]
                        if prior:   # rejected in an earlier chained job: keep its record, move on
                            last = (c, prior[-1]["status"], [], prior[-1]["first_error_line"])
                            continue
                        if lang not in LANG_FW:
                            # v2: recorded, not silently skipped
                            LAST_LOG[0] = ""
                            log_attempt(k, rank, c, "skipped_language",
                                        f"delivered language {lang!r} has no column", False)
                            continue
                        fw, ext = LANG_FW[lang]
                        place(c["path"], backend / f"{k}_fp64{ext}")
                        st, samples, nt = run_framework(k, fw, ARGS.preset, ARGS.reps, ARGS.timeout)
                        log_attempt(k, rank, c, st, nt, st == "ok")
                        if st == "ok":
                            chosen = (c, st, samples, nt); break
                        last = (c, st, samples, nt)
                        # restore before trying the next candidate
                        for e, (src, bak, want) in pristine.items():
                            place(bak, src)
                    if chosen is None and last is None:
                        # every candidate was in a language this matrix has no column for
                        emit(rows, k, rep, "unsupported", [],
                             f"{len(cands)} candidate(s), none in a supported language", "", "", pos)
                        continue
                    c, st, samples, nt = chosen or last
                    crank = allc.index(c) + 1
                    compiler = {"c": "gcc", "cpp": "g++", "fortran": "gfortran"}[c["language"]]
                    note = (f"rank={crank} arm={c['arm']} job={c['job']} seq={c['seq']} lang={c['language']} "
                            f"campaign_speedup={c['campaign_speedup']:.3f} sha256={c['sha256'][:16]}"
                            + (f" | {nt}" if nt else ""))
                    emit(rows, k, rep, st, samples, note, compiler, FLAGS[c["language"]], pos, c["sha256"])
                    continue
                want_sha = sha(timed)
                st, samples, nt = run_framework(k, fw, ARGS.preset, ARGS.reps, ARGS.timeout)
                if sha(timed) != want_sha:
                    # the harness regenerated the file mid-run: this cell did not time what it says
                    st, samples, nt = "build_error", [], f"source in place changed during the run ({timed.name})"
                lang = {"cc": "c", "cpp": "cpp", "fortran": "fortran"}.get(fw, "numba")
                emit(rows, k, rep, st, samples, "; ".join(x for x in (note, nt) if x),
                     compiler, FLAGS.get(lang, "@nb.njit(parallel=True, cache=True)"), pos, want_sha)
        finally:
            for e, (src, bak, want) in pristine.items():
                place(bak, src)
        flush(rows)
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
    # v2: every attempted agent candidate, not just the chosen one
    ap.add_argument("--attempts-out", default="agent_attempts.csv")
    # variant runs (tasks 3 and 4): the bench checkout carries ONE recorded patch (variants/*.patch)
    ap.add_argument("--preset-label", default="", help="value written to the preset column (F)")
    ap.add_argument("--flags-variant", default="", help="value written to the flags_variant column")
    ap.add_argument("--reprs", default="c,cpp,fortran,numba,c_reference,agent")
    ap.add_argument("--emitted-dir", default="emitted_sources_variants")
    ap.add_argument("--attempt-logs", default="agent_attempt_logs")
    # follow-up
    ap.add_argument("--variant", default="", help="value written to the variant column")
    ap.add_argument("--override", action="append", default=[], metavar="REPR=PATH")
    ap.add_argument("--ordered", action="store_true", help="run --reprs in the given order")
    ap.add_argument("--round", default="", help="value written to the round column")
    ap.add_argument("--agent-sha", default="", help="only the agent candidate with this sha256 prefix")
    ap.add_argument("--perf", action="store_true", help="perf stat cycles,instructions over the timed reps")
    ap.add_argument("--vmstat", action="store_true", help="/proc/vmstat deltas around each series")
    ARGS = ap.parse_args()
    # The driver runs with cwd=BENCH, so a relative --scratch would make it write its JSONL under
    # the bench tree while this process looks under its own cwd (first debug job, 4969228: every
    # cell recorded build_error). Resolve every path argument here, once.
    for _a in ("out", "scratch", "attempts_out", "attempt_logs"):
        setattr(ARGS, _a, str(pathlib.Path(getattr(ARGS, _a)).resolve()))
    OVERRIDES = {}
    for _o in ARGS.override:
        _r, _p = _o.split("=", 1)
        assert _r in ("c", "cpp", "fortran", "numba", "c_reference"), _o
        _p = pathlib.Path(_p).resolve()
        assert _p.is_file(), _p
        # an override must be a hand file to the harness, or the emitter could regenerate it
        assert "hpcagent_bench-autogen" not in _p.read_text().splitlines()[0][:40], f"{_p}: carries the autogen marker"
        OVERRIDES[_r] = _p
    ATTEMPTS = []
    pathlib.Path(ARGS.scratch).mkdir(parents=True, exist_ok=True)
    COMMIT = subprocess.run(["git", "-C", str(BENCH), "rev-parse", "HEAD"],
                            capture_output=True, text=True).stdout.strip()
    def ver(c):
        try: return subprocess.run([c, "--version"], capture_output=True, text=True).stdout.splitlines()[0]
        except Exception: return ""
    # v2: the harness resolves drivers with a version floor (languages.COMPILER_MIN_MAJOR: gcc>=14,
    # since the C block pins -std=c23), so `gcc` on PATH is NOT what builds the C columns.
    sys.path[:0] = [str(BENCH), str(BENCH / "hpcagent_bench/numpy_translators/src")]
    from hpcagent_bench import languages as _L
    DRIVERS = {n: _L.resolve_compiler(n) or n for n in ("gcc", "g++", "gfortran")}
    VERSIONS = {n: f"{ver(d)} [{d}]" for n, d in DRIVERS.items()}
    import numba as _nb  # noqa
    VERSIONS["numba"] = f"numba {_nb.__version__}"
    sys.path[:0] = [str(BENCH), str(BENCH / "hpcagent_bench/numpy_translators/src")]
    from hpcagent_bench import languages
    # v2: resolve the flag string under the SAME binding as the build child (numactl node 0), not
    # this srun-bound process: -ftree-parallelize-loops={n} takes n from the caller's affinity, so
    # v1 recorded n=1 while the builds ran with node 0's core count.
    _probe = subprocess.run(
        ["numactl", "--cpunodebind=0", "--membind=0", PY, "-c",
         "import json;from hpcagent_bench import languages as L;"
         "print(json.dumps({l: L.baseline_flags(l) for l in ('c','cpp','fortran')}))"],
        cwd=BENCH, env=env_for_run(), capture_output=True, text=True)
    FLAGS = json.loads(_probe.stdout.strip().splitlines()[-1])
    main()
