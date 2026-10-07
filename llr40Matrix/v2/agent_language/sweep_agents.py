#!/usr/bin/env python3
"""Part B: build, validate and time every LLR-40 agent submission on GH200, C against Fortran.

One process per kernel, inside ONE whole-node srun step on an exclusive node. Every harness call
is v2's: ``hpcagent_bench.cli run --benchmark K --framework {cc,fortran} --precision fp64 --preset M
--repeat 30 --validate`` with 5 warm-up reps. The submission is substituted for the kernel's
``cpp_backend/<k>_fp64.{c,f90}`` exactly as llr40Matrix/v2/sweep.py substitutes the agent cell,
used as delivered, and checked by sha256 against agent_picks.json first. Per candidate, in this order:

  default/T1   harness default compiler for its language (C gcc 14.2.0, Fortran gfortran 13.3.1),
               --mode single_core, ``numactl --physcpubind=0 --membind=0``. The timed process sees
               ONE core on NUMA node 0, as in v2's one-core steps, so the harness resolves
               -ftree-parallelize-loops=1 as v2 did. This run BUILDS, VALIDATES and TIMES.
  default/T72  only if default/T1 validated. --mode multi_core,
               ``numactl --cpunodebind=0 --membind=0``, OMP_NUM_THREADS=72 (the harness sets the
               same from the affinity), OMP_PLACES=cores, OMP_PROC_BIND=close. It times the SAME
               library default/T1 built: the harness rebuilds only when a source is newer than
               lib<k>_<fw>.so, and nothing touches the source in between. The library's sha256 is
               recorded before and after.
  gcc14/T1     the same-compiler build: the source is touched, so the harness rebuilds, inside
               ``uenv run prgenv-gnu/25.6:v2 --view=default``, where the harness resolves gcc,
               g++ and gfortran to that uenv's GCC 14.2.0. Single core as default/T1. Builds,
               validates and times.

Per kernel, translated C (the harness's own lowering) is timed at T1 and T72 as the reference.
Each run is validated against the NumPy oracle before it is timed. A failure is a row, never a skip.
"""
import argparse, csv, hashlib, json, os, pathlib, re, shutil, statistics, subprocess, sys, time

HERE = pathlib.Path(__file__).resolve().parent                     # llr40Matrix/v2/agent_language/
BENCH = pathlib.Path(os.environ.get("LLR40_BENCH", "")).resolve()
LLR = BENCH / "hpcagent_bench/benchmarks/loop_level_reasoning"
PY = os.environ.get("LLR40_PYTHON", sys.executable)
UENV = os.environ.get("AGENT_LANG_UENV", "prgenv-gnu/25.6:v2")
MARK = "hpcagent_bench-autogen"
LANG_FW = {"c": ("cc", ".c"), "fortran": ("fortran", ".f90")}
FIELDS = ["kernel", "role", "model", "language", "arm", "job", "seq", "rank", "sha256", "campaign_speedup",
          "build", "threads", "status", "failure_class", "first_error", "time_ns_min", "time_ns_median",
          "time_ns_all", "n_warmup", "n_reps", "compiler", "compiler_version", "flags", "lib_sha256",
          "lib_rebuilt", "t72_binary_equals_t1", "uses_x86_intrinsics", "sets_thread_count", "notes", "node", "slurm_job", "timestamp"]
INTRINSICS = re.compile(r"#\s*include\s*<(immintrin|xmmintrin|emmintrin|pmmintrin|smmintrin|nmmintrin|x86intrin|"
                        r"avxintrin|tmmintrin|wmmintrin)\.h>|\b_mm(256|512)?_\w+\s*\(|\b__m(128|256|512)[di]?\b")
THREADS = re.compile(r"omp_set_num_threads|num_threads\s*\(", re.I)
COMPILE_ERR = re.compile(r"^.*:\d+(:\d+)?: (fatal )?[Ee]rror:.*$|^\s*Error: .*$|^.*undefined reference.*$", re.M)


def sha(p):
    return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()


def place(src, dst):
    """copy2 + touch (v2): the harness reuses lib<k>_<fw>.so while it is newer than every source."""
    shutil.copy2(src, dst)
    os.utime(dst)


def base_env():
    e = dict(os.environ)
    e.update({"PYTHONPATH": f"{BENCH}:{BENCH}/hpcagent_bench/numpy_translators/src",
              "OMP_PLACES": "cores", "OMP_PROC_BIND": "close", "NUMBA_NUM_THREADS": "1",
              "HPCAGENT_BENCH_MEASUREMENT_WARMUP": str(ARGS.warmup),
              "HPCAGENT_BENCH_DB_SHARD": str(ARGS.db_shard)})
    return e


def wrap(build, threads, inner):
    # recorded runs use the defaults; the env overrides exist only for login-node plumbing tests
    bind = ["numactl"] + (os.environ.get("AGENT_LANG_T1_BIND", "--physcpubind=0 --membind=0") if threads == 1 else
                          os.environ.get("AGENT_LANG_T72_BIND", "--cpunodebind=0 --membind=0")).split()
    cmd = bind + inner
    if build == "gcc14":
        cmd = ["uenv", "run", UENV, "--view=default", "--"] + cmd
    return cmd


def run_cell(kernel, fw, build, threads, timeout_s):
    """One harness call. Returns (status, samples_ms, note, log)."""
    out = pathlib.Path(ARGS.scratch) / f"{kernel}.{fw}.{build}.t{threads}.{os.getpid()}.jsonl"
    out.unlink(missing_ok=True)
    inner = [PY, "-m", "hpcagent_bench.cli", "run", "--benchmark", kernel, "--framework", fw, "--precision", "fp64",
             "--preset", ARGS.preset, "--mode", "single_core" if threads == 1 else "multi_core",
             "--repeat", str(ARGS.reps), "--validate", "--output", str(out)]
    env = base_env()
    env["OMP_NUM_THREADS"] = str(threads)
    cmd = wrap(build, threads, inner)
    try:
        p = subprocess.run(cmd, cwd=BENCH, env=env, capture_output=True, text=True, timeout=timeout_s)
    except subprocess.TimeoutExpired as e:
        return "timeout", [], f"exceeded {timeout_s}s wall budget", f"$ {' '.join(cmd)}\n[timeout]\n{e.stdout or ''}\n{e.stderr or ''}"
    log = f"$ {' '.join(cmd)}\n[exit {p.returncode}]\n--- stdout\n{p.stdout}\n--- stderr\n{p.stderr}"
    if not out.is_file():
        return "build_error", [], first_error(p.stderr or p.stdout), log
    rows = [json.loads(l) for l in out.read_text().splitlines() if l.strip()]
    rows = [r for r in rows if r.get("framework") == fw]
    if not rows:
        return "build_error", [], "driver wrote no row for this framework", log
    r = rows[-1]
    impls = r.get("impls") or {}
    if not impls:
        reason = str(r.get("reason") or "")
        return "build_error", [], (first_error(p.stderr) or f"status={r.get('status')} {reason}")[:900], log
    impl = list(impls.values())[0]
    samples = impl.get("time_python") or []
    if impl.get("validated") is False:
        cerr = COMPILE_ERR.search(p.stderr or "")
        if cerr and "_ensure_built" in (p.stderr or ""):
            return "build_error", [], cerr.group(0).strip()[:600], log
        return "incorrect", samples, mismatch(p.stdout + "\n" + p.stderr), log
    if not samples:
        return "build_error", [], f"no timings; driver status={r.get('status')}", log
    return "ok", samples, "", log


def first_error(text):
    text = text or ""
    m = COMPILE_ERR.search(text)
    if m:
        return m.group(0).strip()[:600]
    for pat in (r"Error\b", r"\berror\b", r"Traceback", r"Exception"):
        for l in text.splitlines():
            if re.search(pat, l):
                return l.strip()[:600]
    return " | ".join(text.strip().splitlines()[-3:])[:600]


def mismatch(text):
    for l in text.splitlines():
        if re.search(r"mismatch|did not validate|max (abs|rel)|tolerance|NaN position", l, re.I):
            return ("output did not match the NumPy reference | " + l.strip())[:600]
    return "output did not match the NumPy reference"


def failure_class(status, note, src_text):
    if status == "build_error":
        return "x86_intrinsics" if (re.search(r"(immintrin|xmmintrin|emmintrin|x86intrin|avxintrin|_mm\w*|__m(128|256|512))",
                                              note) or (INTRINSICS.search(src_text) and "error" in note.lower())) else "other"
    return {"incorrect": "incorrect", "timeout": "timeout"}.get(status, "")


def lib_path(kernel, fw):
    return LLR / kernel / "cpp_backend" / "build" / f"lib{kernel}_{fw}.so"


def main():
    k = ARGS.kernel
    kdir, backend = LLR / k, LLR / k / "cpp_backend"
    picks = json.load(open(HERE.parent / "agent_picks.json"))[k]
    rows = list(csv.DictReader(open(ARGS.out))) if pathlib.Path(ARGS.out).is_file() else []
    done = {(r["role"], r["sha256"], r["build"], r["threads"]) for r in rows}

    pending = []

    def flush_pending():
        """A candidate (or the translated-C pair) is ATOMIC: its rows are written only after all of
        its runs finished. A unit cut at a wall limit leaves no partial candidate behind, and the
        resumed job redoes it from default/T1, so default/T72 always times the library default/T1 just
        built in the one-core binding."""
        rows.extend(pending); pending.clear()
        tmp = pathlib.Path(ARGS.out + ".tmp")
        with open(tmp, "w", newline="") as fh:
            w = csv.DictWriter(fh, fieldnames=FIELDS, extrasaction="ignore"); w.writeheader(); w.writerows(rows)
        tmp.replace(ARGS.out)

    def emit(row, samples, log):
        ns = [int(round(s * 1e6)) for s in samples]
        row.update(time_ns_min=min(ns) if ns else "", time_ns_median=int(statistics.median(ns)) if ns else "",
                   time_ns_all=" ".join(map(str, ns)), n_warmup=ARGS.warmup, n_reps=len(ns),
                   node=os.uname().nodename, timestamp=time.strftime("%Y-%m-%dT%H:%M:%S%z"),
                   slurm_job=f"{os.environ.get('SLURM_JOB_PARTITION', '')}/{os.environ.get('SLURM_JOB_ID', '')}")
        pending.append(row)
        if row["status"] not in ("ok",) and log:
            d = pathlib.Path(ARGS.logs); d.mkdir(parents=True, exist_ok=True)
            (d / f"{k}.{row['sha256'][:12]}.{row['build']}.t{row['threads']}.log").write_text(log)
        print(f"  {row['role']:10s} {row['language']:8s} {row['arm'][-22:]:22s} seq={row['seq']:>3s} {row['build']:7s} "
              f"T{row['threads']:<3} {row['status']:12s} {('%.3f ms' % (min(ns) / 1e6)) if ns else '':>12s} "
              f"{row['failure_class']} {row['first_error'][:70]}", flush=True)
        return row

    # 0./1./2. as v2's sweep: drop a leftover non-autogen source, generate, snapshot the pristine lowering
    for ext in (".c", ".cpp", ".f90"):
        f = backend / f"{k}_fp64{ext}"
        if f.is_file() and MARK not in (f.read_text().splitlines() or [""])[0]:   # empty = cut copy
            print(f"  removing leftover non-autogen {f.name} before generation", flush=True); f.unlink()
    gen = subprocess.run([PY, "-c", "import sys,hpcagent_bench.autogen as A;A.ensure_native(sys.argv[1])",
                          f"loop_level_reasoning/{k}"], cwd=BENCH, env=base_env(), capture_output=True, text=True)
    if gen.returncode != 0:
        raise SystemExit(f"{k}: autogen failed: {gen.stderr[-500:]}")
    pristine = {}
    for ext in (".c", ".f90"):
        src = backend / f"{k}_fp64{ext}"
        if MARK not in src.read_text().splitlines()[0]:
            raise SystemExit(f"{src.name} is not autogen output; refusing")
        bak = pathlib.Path(ARGS.scratch) / f"pristine.{src.name}"
        shutil.copy2(src, bak); pristine[ext] = (src, bak, sha(src))

    def restore():
        for e, (src, bak, want) in pristine.items():
            place(bak, src)
            assert sha(src) == want

    try:
        # translated C, the reference, at T1 and T72 (same library)
        restore()
        meta = {"kernel": k, "role": "translated_c", "model": "", "language": "c", "arm": "translator", "job": "",
                "seq": "", "rank": "", "sha256": pristine[".c"][2], "campaign_speedup": "", "uses_x86_intrinsics": "no",
                "sets_thread_count": "no"}
        for threads in (1, 72) if ("translated_c", meta["sha256"], "default", "1") not in done else ():
            before = sha(lib_path(k, "cc")) if lib_path(k, "cc").is_file() else ""
            st, samples, note, log = run_cell(k, "cc", "default", threads, ARGS.timeout)
            after = sha(lib_path(k, "cc")) if lib_path(k, "cc").is_file() else ""
            emit(dict(meta, build="default", threads=threads, status=st, first_error=note, failure_class="",
                      compiler="gcc", compiler_version=VERSIONS["default"]["gcc"], flags=FLAGS["default"]["c"],
                      lib_sha256=after, lib_rebuilt="" if threads == 1 else str(before != after).lower()), samples, log)
        flush_pending()

        t1_lib = {}
        if ARGS.redo_t72:
            redo_t72(k, picks, rows, backend, flush_pending, emit, restore)
            return
        cands = sorted(picks, key=lambda c: (int(c["job"]), int(c["seq"])))
        if ARGS.limit:
            cands = cands[:ARGS.limit]
        for c in cands:
            lang = c["language"]
            fw, ext = LANG_FW[lang]
            text = pathlib.Path(c["path"]).read_text(errors="replace")
            meta = {"kernel": k, "role": "candidate", "model": c["arm"].split("-")[1], "language": lang,
                    "arm": c["arm"], "job": c["job"], "seq": c["seq"], "rank": picks.index(c) + 1,
                    "sha256": c["sha256"], "campaign_speedup": f"{c['campaign_speedup']:.6g}",
                    "uses_x86_intrinsics": "yes" if INTRINSICS.search(text) else "no",
                    "sets_thread_count": "yes" if THREADS.search(text) else "no"}
            if sha(c["path"]) != c["sha256"]:
                emit(dict(meta, build="default", threads=1, status="sha256_mismatch", failure_class="other",
                          first_error="source sha256 differs from agent_picks.json; not used"), [], "")
                continue
            comp = "gcc" if lang == "c" else "gfortran"
            target = backend / f"{k}_fp64{ext}"
            if ("candidate", c["sha256"], "default", "1") in done:
                print(f"  candidate {c['arm']} seq={c['seq']} already recorded (resume)", flush=True)
                continue
            valid = None
            for build, threads in (("default", 1), ("default", 72), ("gcc14", 1)):
                if build == "default" and threads == 72 and not valid:
                    continue        # T72 times VALID candidates only
                if build == "default" and threads == 1:
                    restore()                               # pristine lowerings, then this candidate
                    place(c["path"], target)
                elif build == "default" and threads == 72:
                    # nothing is placed or restored. The harness still rebuilds: every `cli run`
                    # re-emits the generated fp32 sibling, which makes it newer than the library. What
                    # matters is whether the T72 library is BYTE-IDENTICAL to default/T1's (recorded).
                    assert sha(target) == c["sha256"]
                else:
                    place(c["path"], target)               # touched: gcc14 must rebuild
                lib = lib_path(k, fw)
                before = sha(lib) if lib.is_file() else ""
                before_mt = lib.stat().st_mtime_ns if lib.is_file() else 0
                st, samples, note, log = run_cell(k, fw, build, threads, ARGS.timeout)
                after = sha(lib) if lib.is_file() else ""
                after_mt = lib.stat().st_mtime_ns if lib.is_file() else 0
                t72_same = ""
                if build == "default" and threads == 72:
                    t72_same = "yes" if (after and after == t1_lib.get(c["sha256"])) else "no"
                if sha(target) != c["sha256"]:
                    st, samples, note = "build_error", [], f"source in place changed during the run ({target.name})"
                if build == "default" and threads == 1:
                    valid = st == "ok"
                    t1_lib[c["sha256"]] = after
                if st == "build_error":
                    after = ""                              # no library of this candidate exists
                emit(dict(meta, build=build, threads=threads, status=st, first_error=note,
                          failure_class=failure_class(st, note, text), compiler=comp,
                          compiler_version=VERSIONS[build][comp], flags=FLAGS[build][lang], lib_sha256=after,
                          lib_rebuilt=str(before_mt != after_mt).lower(), t72_binary_equals_t1=t72_same,
                          notes="submission sets its own thread count (not patched)" if meta["sets_thread_count"] == "yes" else ""),
                     samples, log)
            flush_pending()
    finally:
        restore()
    print(f"wrote {len(rows)} rows -> {ARGS.out}")


def redo_t72(k, picks, rows, backend, flush_pending, emit, restore):
    """Re-time default/T72 for every valid candidate whose T72 row is missing or was refused by the
    earlier mtime rule ("library rebuilt ..."). The refused rows are replaced, and T1/gcc14 rows are
    left as they are. The harness rebuilds at T72 in any case (see the T72 branch). Whether that
    library is byte-identical to the recorded default/T1 library is recorded."""
    t1 = {r["sha256"]: r for r in rows if r["role"] == "candidate" and r["build"] == "default" and r["threads"] == "1"}
    for c in sorted(picks, key=lambda c: (int(c["job"]), int(c["seq"]))):
        r1 = t1.get(c["sha256"])
        if not r1 or r1["status"] != "ok":
            continue
        old = [r for r in rows if r["role"] == "candidate" and r["sha256"] == c["sha256"] and r["build"] == "default"
               and r["threads"] == "72"]
        if old and "library rebuilt" not in old[-1]["first_error"]:
            continue
        lang = c["language"]; fw, ext = LANG_FW[lang]
        target = backend / f"{k}_fp64{ext}"
        restore(); place(c["path"], target)
        lib = lib_path(k, fw)
        before_mt = lib.stat().st_mtime_ns if lib.is_file() else 0
        st, samples, note, log = run_cell(k, fw, "default", 72, ARGS.timeout)
        after = sha(lib) if lib.is_file() and st != "build_error" else ""
        after_mt = lib.stat().st_mtime_ns if lib.is_file() else 0
        for r in old:
            rows.remove(r)
        text = pathlib.Path(c["path"]).read_text(errors="replace")
        meta = {k2: r1[k2] for k2 in ("kernel", "role", "model", "language", "arm", "job", "seq", "rank", "sha256",
                                       "campaign_speedup", "uses_x86_intrinsics", "sets_thread_count")}
        emit(dict(meta, build="default", threads=72, status=st, first_error=note,
                  failure_class=failure_class(st, note, text), compiler=r1["compiler"],
                  compiler_version=r1["compiler_version"], flags=r1["flags"], lib_sha256=after,
                  lib_rebuilt=str(before_mt != after_mt).lower(),
                  t72_binary_equals_t1="yes" if (after and after == r1["lib_sha256"]) else "no",
                  notes=("re-timed in a separate T72 pass (the first T72 run was refused by an mtime rule); "
                         + ("submission sets its own thread count (not patched)" if meta["sets_thread_count"] == "yes" else ""))),
             samples, log)
        flush_pending()
    restore()


def probe(build):
    """compiler path + version and the flag strings, as the build child resolves them (1-core binding)."""
    code = ("import json,subprocess;from hpcagent_bench import languages as L\n"
            "d={n: L.resolve_compiler(n) or n for n in ('gcc','g++','gfortran')}\n"
            "v={n: subprocess.run([p,'--version'],capture_output=True,text=True).stdout.splitlines()[0]+' ['+p+']' for n,p in d.items()}\n"
            "print(json.dumps({'versions':v,'flags':{l: L.baseline_flags(l) for l in ('c','fortran')}}))")
    p = subprocess.run(wrap(build, 1, [PY, "-c", code]), cwd=BENCH, env=base_env(), capture_output=True, text=True)
    return json.loads(p.stdout.strip().splitlines()[-1])


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--kernel", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--scratch", required=True)
    ap.add_argument("--logs", required=True)
    ap.add_argument("--db-shard", type=int, required=True)
    ap.add_argument("--reps", type=int, default=30)
    ap.add_argument("--warmup", type=int, default=5)
    ap.add_argument("--timeout", type=int, default=1800)
    ap.add_argument("--preset", default="M", help="M for every recorded run (S only for plumbing tests)")
    ap.add_argument("--limit", type=int, default=0, help="first N candidates only (plumbing tests)")
    ap.add_argument("--redo-t72", action="store_true", help="only re-time refused/missing default T72 cells")
    ARGS = ap.parse_args()
    for _a in ("out", "scratch", "logs"):
        setattr(ARGS, _a, str(pathlib.Path(getattr(ARGS, _a)).resolve()))
    pathlib.Path(ARGS.scratch).mkdir(parents=True, exist_ok=True)
    P = {b: probe(b) for b in ("default", "gcc14")}
    VERSIONS = {b: P[b]["versions"] for b in P}
    FLAGS = {b: P[b]["flags"] for b in P}
    print(json.dumps({"versions": VERSIONS, "flags": FLAGS}, indent=1), flush=True)
    main()
