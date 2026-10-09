#!/usr/bin/env python3
"""DaCe follow-up: time ONE cell whose code differs from the measured DaCe or translated column in one
place, through the same path as ``../dace_cells.py time`` (the harness's ``run_one``, preset M, validated,
5 warm-up + 30 timed calls, one OpenMP thread).

A cell is LABEL with a pipeline (``canon``: the DaCe column's pipeline; ``cpp`` / ``c``: the translated
C++ / C column at the same commit) and the codegen ``new`` (the column's own configuration) plus at most
one change:

  --override-cpp PATH   (canon) after DaCe writes the program folder and before it compiles it, PATH
                        replaces the generated ``src/cpu/canon_cpu.cpp``. Hooked on
                        ``dace.codegen.compiler.generate_program_folder``; the bytes compiled are
                        re-checked against PATH after the run.
  --override-src PATH   (cpp / c) PATH is placed over the translated ``<k>_fp64.cpp`` / ``.c`` in the
                        kernel's cpp_backend (touched, so the harness rebuilds), and restored afterwards.

Every cell builds in its own DaCe build folder (``--out/build/<kernel>.<label>``), so variants never share
a cached binary. Writes ``--out/rows/<kernel>.<label>.<round>.json`` with every timing.

Environment: DACE_BENCH = the HPCAgent-Bench worktree at 5cfcb4f2 (spcl/dace@dea0b39c installed).
"""
import argparse
import hashlib
import json
import os
import pathlib
import shutil
import sys
import time

CODEGEN_NEW = ("experimental_readable", "true", "true")
BASELINES = {"c": ("cc", ".c"), "cpp": ("cpp", ".cpp")}


def sha(p):
    return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("kernel")
    p.add_argument("pipeline", choices=["canon", *BASELINES])
    p.add_argument("--label", required=True)
    p.add_argument("--round", default="1")
    p.add_argument("--override-cpp", default="")
    p.add_argument("--override-src", default="")
    p.add_argument("--out", required=True)
    p.add_argument("--preset", default="M")
    p.add_argument("--warmup", type=int, default=5)
    p.add_argument("--reps", type=int, default=30)
    p.add_argument("--timeout", type=float, default=3600.0)
    a = p.parse_args()
    out = pathlib.Path(a.out).resolve()
    over_cpp = pathlib.Path(a.override_cpp).resolve() if a.override_cpp else None
    over_src = pathlib.Path(a.override_src).resolve() if a.override_src else None
    bench = pathlib.Path(os.environ["DACE_BENCH"]).resolve()
    sys.path[:0] = [str(bench)]
    os.chdir(bench)
    build = out / "build" / f"{a.kernel}.{a.label}"
    if build.exists():
        shutil.rmtree(build)      # never reuse a binary: every run compiles what it times
    build.mkdir(parents=True)
    if a.pipeline == "canon":
        impl, tree, explicit = CODEGEN_NEW
        os.environ["DACE_compiler_cpu_implementation"] = impl
        os.environ["DACE_compiler_emit_tree_reductions"] = tree
        os.environ["DACE_compiler_cpu_explicit_copy"] = explicit
    os.environ["DACE_default_build_folder"] = str(build / "dacecache")
    os.environ["DACE_BUILD_CACHE_DIR"] = str(build / "pch")
    os.environ["HPCAGENT_BENCH_RECORD_DB_PATH"] = str(build / "results.db")
    os.environ["HPCAGENT_BENCH_MEASUREMENT_WARMUP"] = str(a.warmup)

    replaced = []
    if over_cpp:
        from dace.codegen import compiler as dc
        orig = dc.generate_program_folder

        def generate_program_folder(sdfg, code_objects, out_path, *args, **kwargs):
            r = orig(sdfg, code_objects, out_path, *args, **kwargs)
            # the column's program, and the harness's strict-FP rebuild (``canon_cpu_strict_fp``, tried only
            # after a failed validation), which gets the same code under its own symbol names
            if sdfg is not None and str(sdfg.name).startswith("canon_cpu"):
                target = pathlib.Path(out_path) / "src" / "cpu" / f"{sdfg.name}.cpp"
                assert target.is_file(), f"no generated {target}"
                text = over_cpp.read_text()
                if sdfg.name != "canon_cpu":   # the strict-FP rebuild: same code under its own symbol names
                    text = text.replace("canon_cpu", str(sdfg.name))
                target.write_text(text)
                replaced.append((str(target), hashlib.sha256(text.encode()).hexdigest()))
            return r

        dc.generate_program_folder = generate_program_folder

    restore = None
    if over_src:
        _, ext = BASELINES[a.pipeline]
        kdir = next(p.parent for p in (bench / "hpcagent_bench/benchmarks/loop_level_reasoning").glob(f"**/{a.kernel}.yaml"))
        # make sure the emitted source exists, then swap it
        import hpcagent_bench.autogen as A
        A.ensure_native(a.kernel)
        target = kdir / "cpp_backend" / f"{a.kernel}_fp64{ext}"
        bak = build / f"pristine{ext}"
        shutil.copy2(target, bak)
        shutil.copyfile(over_src, target); os.utime(target)
        restore = (bak, target)

    from hpcagent_bench.support.collect.sweep import run_one
    column = BASELINES[a.pipeline][0] if a.pipeline in BASELINES else "dace_cpu_canonicalize"
    row = {"kernel": a.kernel, "label": a.label, "round": a.round, "pipeline": a.pipeline,
           "codegen": "new" if a.pipeline == "canon" else "", "column": column,
           "override": str(over_cpp or over_src or ""), "override_sha256": sha(over_cpp or over_src) if (over_cpp or over_src) else "",
           "preset": a.preset, "warmup": a.warmup, "reps": a.reps, "node": os.uname().nodename,
           "cpus": len(os.sched_getaffinity(0)), "omp_num_threads": os.environ.get("OMP_NUM_THREADS", ""),
           "slurm_job": os.environ.get("SLURM_JOB_ID", ""), "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S%z")}
    t0 = time.time()
    try:
        result = run_one(f"loop_level_reasoning/{a.kernel}/{a.kernel}", [column], a.preset, True,
                         a.reps, a.timeout, False, "float64")
        per_impl = result.get(column) or {}
        row["impls"] = {n: {k: v for k, v in t.items() if k in ("native", "python", "validated", "failure")}
                        for n, t in per_impl.items()}
        row["status"] = "ok"
        if over_cpp:
            if not replaced:
                row["status"], row["error"] = "error", "override never applied (no program folder generated)"
            elif any(sha(path) != want for path, want in replaced):
                row["status"], row["error"] = "error", "a compiled file differs from the override"
            row["replaced"] = [path for path, _ in replaced]
    except Exception as exc:  # recorded, never dropped
        row["status"], row["error"] = "error", f"{type(exc).__name__}: {exc}"
    finally:
        if restore:
            bak, target = restore
            shutil.copy2(bak, target); os.utime(target)
    if a.pipeline == "canon":
        import dace
        row["implementation"] = dace.Config.get("compiler", "cpu", "implementation")
        row["cxx_args"] = dace.Config.get("compiler", "cpu", "args")
    row["wall_s"] = round(time.time() - t0, 1)
    (out / "rows").mkdir(parents=True, exist_ok=True)
    (out / "rows" / f"{a.kernel}.{a.label}.{a.round}.json").write_text(json.dumps(row) + "\n")
    print("TIME " + json.dumps({k: v for k, v in row.items() if k != "impls"}))
    return 0 if row["status"] == "ok" else 1


if __name__ == "__main__":
    sys.exit(main())
