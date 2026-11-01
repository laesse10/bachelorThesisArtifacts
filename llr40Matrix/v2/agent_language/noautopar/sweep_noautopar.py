#!/usr/bin/env python3
"""Re-time every valid agent submission at T72 without gfortran's automatic parallelization.

The default/T72 runs of ``../sweep_agents.py`` let the harness size ``-ftree-parallelize-loops`` from the
affinity, so gfortran built 77 of the 196 valid Fortran libraries with automatic parallelization at 72
threads, while the C build has no such step. This sweep repeats default/T72 for BOTH languages with a
harness copy whose only change is that ``-ftree-parallelize-loops`` is fixed at 1
(``languages.py``, ``format(n=1)``; see ``run_noautopar.sh``), as in the T1 build. OpenMP stays on, with
the same binding and environment as the original T72 runs (``sweep_agents.wrap``).

Valid candidates and their T1 rows come from the original ``../parts/<kernel>.csv``. Per kernel: the
translated C at T72, then every candidate whose default/T1 row is ``ok``, in the original order. Rows go
to ``parts/<kernel>.csv`` in this directory, with the original fields; ``build`` is ``noautopar``.
"""
import argparse, csv, json, os, pathlib, shutil, subprocess, sys

HERE = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
import sweep_agents as S  # noqa: E402  (LLR40_BENCH must point at the patched copy before this import)


def main(args) -> None:
    k = args.kernel
    backend = S.LLR / k / "cpp_backend"
    picks = json.load(open(HERE.parent.parent / "agent_picks.json"))[k]
    orig = list(csv.DictReader(open(HERE.parent / "parts" / f"{k}.csv")))
    t1 = {r["sha256"]: r for r in orig if r["role"] == "candidate" and r["build"] == "default" and r["threads"] == "1"}
    rows = list(csv.DictReader(open(args.out))) if pathlib.Path(args.out).is_file() else []
    done = {(r["role"], r["sha256"]) for r in rows}
    pending = []

    def flush():
        rows.extend(pending); pending.clear()
        tmp = pathlib.Path(args.out + ".tmp")
        with open(tmp, "w", newline="") as fh:
            w = csv.DictWriter(fh, fieldnames=S.FIELDS, extrasaction="ignore"); w.writeheader(); w.writerows(rows)
        tmp.replace(args.out)

    def emit(row, samples, log):
        ns = [int(round(s * 1e6)) for s in samples]
        row.update(time_ns_min=min(ns) if ns else "", time_ns_median=int(S.statistics.median(ns)) if ns else "",
                   time_ns_all=" ".join(map(str, ns)), n_warmup=args.warmup, n_reps=len(ns),
                   node=os.uname().nodename, timestamp=S.time.strftime("%Y-%m-%dT%H:%M:%S%z"),
                   slurm_job=f"{os.environ.get('SLURM_JOB_PARTITION', '')}/{os.environ.get('SLURM_JOB_ID', '')}")
        pending.append(row)
        if row["status"] != "ok" and log:
            d = pathlib.Path(args.logs); d.mkdir(parents=True, exist_ok=True)
            (d / f"{k}.{row['sha256'][:12]}.noautopar.t72.log").write_text(log)
        print(f"  {row['role']:12s} {row['language']:8s} {row['arm'][-22:]:22s} seq={row['seq']:>3s} T72 "
              f"{row['status']:12s} {('%.3f ms' % (min(ns) / 1e6)) if ns else '':>12s} {row['first_error'][:70]}",
              flush=True)

    # as ../sweep_agents.py: generate, snapshot the pristine lowerings, restore them before every candidate
    for ext in (".c", ".cpp", ".f90"):
        f = backend / f"{k}_fp64{ext}"
        if f.is_file() and S.MARK not in (f.read_text().splitlines() or [""])[0]:
            f.unlink()
    gen = subprocess.run([S.PY, "-c", "import sys,hpcagent_bench.autogen as A;A.ensure_native(sys.argv[1])",
                          f"loop_level_reasoning/{k}"], cwd=S.BENCH, env=S.base_env(), capture_output=True, text=True)
    if gen.returncode != 0:
        raise SystemExit(f"{k}: autogen failed: {gen.stderr[-500:]}")
    pristine = {}
    for ext in (".c", ".f90"):
        src = backend / f"{k}_fp64{ext}"
        bak = pathlib.Path(args.scratch) / f"pristine.{src.name}"
        shutil.copy2(src, bak); pristine[ext] = (src, bak, S.sha(src))

    def restore():
        for src, bak, want in pristine.values():
            S.place(bak, src)
            assert S.sha(src) == want

    try:
        restore()
        if ("translated_c", pristine[".c"][2]) not in done:
            st, samples, note, log = S.run_cell(k, "cc", "default", 72, args.timeout)
            lib = S.lib_path(k, "cc")
            emit({"kernel": k, "role": "translated_c", "model": "", "language": "c", "arm": "translator", "job": "",
                  "seq": "", "rank": "", "sha256": pristine[".c"][2], "campaign_speedup": "", "build": "noautopar",
                  "threads": 72, "status": st, "first_error": note, "failure_class": "", "compiler": "gcc",
                  "compiler_version": S.VERSIONS["default"]["gcc"], "flags": FLAGS72["c"],
                  "lib_sha256": S.sha(lib) if lib.is_file() and st != "build_error" else "",
                  "uses_x86_intrinsics": "no", "sets_thread_count": "no"}, samples, log)
            flush()
        for c in sorted(picks, key=lambda c: (int(c["job"]), int(c["seq"]))):
            r1 = t1.get(c["sha256"])
            if not r1 or r1["status"] != "ok" or ("candidate", c["sha256"]) in done:
                continue
            fw, ext = S.LANG_FW[c["language"]]
            target = backend / f"{k}_fp64{ext}"
            restore(); S.place(c["path"], target)
            st, samples, note, log = S.run_cell(k, fw, "default", 72, args.timeout)
            if S.sha(target) != c["sha256"]:
                st, samples, note = "build_error", [], f"source in place changed during the run ({target.name})"
            lib = S.lib_path(k, fw)
            text = pathlib.Path(c["path"]).read_text(errors="replace")
            meta = {f: r1[f] for f in ("kernel", "role", "model", "language", "arm", "job", "seq", "rank", "sha256",
                                       "campaign_speedup", "uses_x86_intrinsics", "sets_thread_count")}
            emit(dict(meta, build="noautopar", threads=72, status=st, first_error=note,
                      failure_class=S.failure_class(st, note, text), compiler=r1["compiler"],
                      compiler_version=r1["compiler_version"], flags=FLAGS72[c["language"]],
                      lib_sha256=S.sha(lib) if lib.is_file() and st != "build_error" else "",
                      notes="T1 library sha256 " + r1["lib_sha256"][:16]), samples, log)
            flush()
    finally:
        restore()
    print(f"wrote {len(rows)} rows -> {args.out}")


def probe72():
    """The flag strings as the build child resolves them in the T72 binding: must say parallelize-loops=1."""
    code = ("import json;from hpcagent_bench import languages as L\n"
            "print(json.dumps({l: L.baseline_flags(l) for l in ('c','fortran')}))")
    p = subprocess.run(S.wrap("default", 72, [S.PY, "-c", code]), cwd=S.BENCH, env=S.base_env(),
                       capture_output=True, text=True)
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
    ap.add_argument("--preset", default="M")
    args = ap.parse_args()
    for a in ("out", "scratch", "logs"):
        setattr(args, a, str(pathlib.Path(getattr(args, a)).resolve()))
    pathlib.Path(args.scratch).mkdir(parents=True, exist_ok=True)
    S.ARGS = args
    P = {b: S.probe(b) for b in ("default",)}
    S.VERSIONS = {b: P[b]["versions"] for b in P}
    S.FLAGS = {b: P[b]["flags"] for b in P}
    FLAGS72 = probe72()
    if "parallelize-loops=1" not in FLAGS72["fortran"]:
        raise SystemExit(f"T72 Fortran flags still autoparallelize: {FLAGS72['fortran']}")
    print(json.dumps({"versions": S.VERSIONS, "flags_t72": FLAGS72}, indent=1), flush=True)
    main(args)
