#!/usr/bin/env python3
"""DaCe's old and new CPU code generators on the LLR-40 kernels and on the four kernels of Bonsall's
report (s313, vdotr, s453, s314).

A cell is (kernel, pipeline, codegen):

  pipeline  canon  the fork's canonicalize pipeline, exactly as the ``dace_cpu_canonicalize`` column runs
                   it. This is what ``codegen-20261006/`` holds (its directories are ``cpu-canon``).
            none   the parsed SDFG, simplified only: no optimising pipeline. Bonsall's setting ("the C++
                   that DaCe emits, without running the DaCe auto-vectorization pipeline").
  codegen   old     the classic configuration (``CLASSIC_CODEGEN``, what the parallel, autoopt and
                    loop2map columns build with): legacy generator, tree reductions off, explicit copies
                    off. This is the dump's ``old_codegen``.
            legacy  the legacy generator under the canon column's other settings (tree reductions on,
                    explicit copies on): old -> legacy isolates those two settings, legacy -> new the
                    generator itself.
            new     the canon column's own configuration (``READABLE_CODEGEN``): experimental_readable
                    generator, tree reductions on, explicit copies on. The dump's ``new_codegen``.

``emit`` checks the old and new canon cells against the dump byte for byte.

  emit    write every cell's generated program folder (src/cpu/canon_cpu.cpp, include/) under --out, and
          compare canon cells with codegen-20261006/
  time    validate and time ONE cell through the harness's own ``run_one`` (the measurement path of
          ``hpcagent-bench run-benchmark``), --warmup + --reps. Pipeline ``c`` times the translated C
          column (``cc``, gcc 14) instead and ``cpp`` the translated C++ column (``cpp``, g++ 13, the
          compiler DaCe's code is built with here): the two baselines. Writes one JSON row.

Environment: DACE_BENCH = the HPCAgent-Bench worktree at 5cfcb4f2 (spcl/dace@dea0b39c installed in
the interpreter). The cell's codegen goes through the DACE_compiler_* environment variables, which the
harness's ``apply_pipeline_config`` deliberately leaves alone (its "codegen A/B" hook); each cell builds in its
own DaCe build folder so old and new never share a cached binary.
"""
import argparse
import copy
import filecmp
import json
import os
import pathlib
import sys
import time

#: codegen -> (compiler.cpu.implementation, compiler.emit_tree_reductions, compiler.cpu.explicit_copy)
CODEGEN = {
    "old": ("legacy", "false", "false"),
    "legacy": ("legacy", "true", "true"),
    "new": ("experimental_readable", "true", "true"),
}
#: baseline "pipelines": the translated C and C++ columns at the same commit
BASELINES = {"c": "cc", "cpp": "cpp"}
HERE = pathlib.Path(__file__).resolve().parent
DUMP = HERE / "codegen-20261006" / "kernels" / "loop_level_reasoning"


def setup(pipeline: str, codegen: str, build_root: pathlib.Path) -> None:
    """Point the process at the cell BEFORE dace is imported: codegen, build folder, PCH cache."""
    bench = pathlib.Path(os.environ["DACE_BENCH"]).resolve()
    sys.path[:0] = [str(bench)]
    os.chdir(bench)
    if pipeline not in BASELINES:
        impl, tree, explicit = CODEGEN[codegen]
        os.environ["DACE_compiler_cpu_implementation"] = impl
        os.environ["DACE_compiler_emit_tree_reductions"] = tree
        os.environ["DACE_compiler_cpu_explicit_copy"] = explicit
    build_root.mkdir(parents=True, exist_ok=True)
    os.environ["DACE_default_build_folder"] = str(build_root / "dacecache")
    os.environ["DACE_BUILD_CACHE_DIR"] = str(build_root / "pch")
    # The harness also records every run into a SQLite DB, sharded by SLURM_PROCID -- 0 in every
    # single-task step, so concurrent cells on different nodes would share (and lock) one file. Each
    # cell gets its own; the timings this script reports come from run_one's return value, not the DB.
    os.environ["HPCAGENT_BENCH_RECORD_DB_PATH"] = str(build_root / "results.db")


def patch_pipeline(pipeline: str) -> None:
    """``none``: the column's pipeline slot runs ``simplify`` only, under the column's codegen config."""
    if pipeline != "none":
        return
    from hpcagent_bench.frameworks import dace_framework as df

    canon = df.PIPELINES_BY_NAME["canon_cpu"]
    df.PIPELINES_BY_NAME["canon_cpu"] = df.SdfgPipeline("canon_cpu", lambda sdfg, ctx: sdfg.simplify(), config=canon.config)


def emit(args: argparse.Namespace) -> int:
    out = pathlib.Path(args.out).resolve() / args.kernel / f"{args.pipeline}_{args.codegen}"
    setup(args.pipeline, args.codegen, out.parent / "build" / f"{args.pipeline}_{args.codegen}")
    patch_pipeline(args.pipeline)
    import dace
    from dace.codegen import compiler as dace_compiler

    from hpcagent_bench.frameworks import Benchmark, generate_framework
    from hpcagent_bench.frameworks import dace_framework as df

    fw = generate_framework("dace_cpu_canonicalize")
    fw.set_datatype("float64")
    bench = Benchmark(f"loop_level_reasoning/{args.kernel}/{args.kernel}")
    program = fw._import_kernel(bench)
    base = fw.build_with_cache(bench, fw._device_tag(), lambda: program.to_sdfg(simplify=False))
    df.pin_cpp_standard("cpu")
    df.pin_host_compiler()
    pipe = df.pipeline_named("canon_cpu")
    df.apply_pipeline_config(pipe)
    sdfg = copy.deepcopy(base)
    sdfg.name = pipe.name
    t0 = time.time()
    pipe.transform(sdfg, fw._build_context())
    code = sdfg.generate_code()
    dace_compiler.generate_program_folder(sdfg, code, str(out))
    row = {
        "kernel": args.kernel, "pipeline": args.pipeline, "codegen": args.codegen,
        "implementation": dace.Config.get("compiler", "cpu", "implementation"),
        "emit_tree_reductions": dace.Config.get("compiler", "emit_tree_reductions"),
        "explicit_copy": dace.Config.get("compiler", "cpu", "explicit_copy"),
        "cxx": dace.Config.get("compiler", "cpu", "executable"), "cxx_args": dace.Config.get("compiler", "cpu", "args"),
        "transform_s": round(time.time() - t0, 2),
    }
    if args.pipeline == "canon" and args.codegen in ("old", "new"):
        sub = "old_codegen" if args.codegen == "old" else "new_codegen"
        ref = DUMP / args.kernel / "cpu-canon" / sub
        files = ["src/cpu/canon_cpu.cpp", "include/canon_cpu.h", "include/hash.h"]
        row["dump_identical"] = {f: (ref / f).exists() and filecmp.cmp(out / f, ref / f, shallow=False) for f in files}
        # The same lines in another order: transient declarations come out in Python's (randomised) set
        # order, which is not part of the code's meaning.
        cpp = ref / files[0]
        row["dump_same_lines"] = cpp.exists() and sorted((out / files[0]).read_text().splitlines()) == sorted(
            cpp.read_text().splitlines())
    print("EMIT " + json.dumps(row))
    return 0


def time_cell(args: argparse.Namespace) -> int:
    out = pathlib.Path(args.out).resolve()
    cell = f"{args.kernel}.{args.pipeline}_{args.codegen}"
    setup(args.pipeline, args.codegen, out / "build" / cell)
    os.environ["HPCAGENT_BENCH_MEASUREMENT_WARMUP"] = str(args.warmup)
    patch_pipeline(args.pipeline)
    from hpcagent_bench.support.collect.sweep import run_one

    column = BASELINES.get(args.pipeline, "dace_cpu_canonicalize")
    row = {
        "kernel": args.kernel, "pipeline": args.pipeline, "codegen": "" if args.pipeline in BASELINES else args.codegen,
        "column": column, "preset": args.preset, "warmup": args.warmup, "reps": args.reps,
        "node": os.uname().nodename, "cpus": len(os.sched_getaffinity(0)),
        "omp_num_threads": os.environ.get("OMP_NUM_THREADS", ""), "slurm_job": os.environ.get("SLURM_JOB_ID", ""),
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
    }
    t0 = time.time()
    try:
        result = run_one(f"loop_level_reasoning/{args.kernel}/{args.kernel}", [column], args.preset, True,
                         args.reps, args.timeout, False, "float64")
        per_impl = result.get(column) or {}
        row["impls"] = {name: {k: v for k, v in timing.items() if k in ("native", "python", "validated", "failure")}
                        for name, timing in per_impl.items()}
        row["status"] = "ok"
    except Exception as exc:  # recorded, never dropped
        row["status"] = "error"
        row["error"] = f"{type(exc).__name__}: {exc}"
    if args.pipeline not in BASELINES:
        import dace

        row["implementation"] = dace.Config.get("compiler", "cpu", "implementation")
        row["emit_tree_reductions"] = dace.Config.get("compiler", "emit_tree_reductions")
        row["explicit_copy"] = dace.Config.get("compiler", "cpu", "explicit_copy")
        row["cxx_args"] = dace.Config.get("compiler", "cpu", "args")
    row["wall_s"] = round(time.time() - t0, 1)
    (out / "rows").mkdir(parents=True, exist_ok=True)
    (out / "rows" / f"{cell}.json").write_text(json.dumps(row) + "\n")
    print("TIME " + json.dumps({k: v for k, v in row.items() if k != "impls"}))
    return 0 if row["status"] == "ok" else 1


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("mode", choices=["emit", "time"])
    p.add_argument("kernel")
    p.add_argument("pipeline", choices=["canon", "none", *BASELINES])
    p.add_argument("codegen", choices=[*CODEGEN, "-"])
    p.add_argument("--out", required=True)
    p.add_argument("--preset", default="M")
    p.add_argument("--warmup", type=int, default=5)
    p.add_argument("--reps", type=int, default=30)
    p.add_argument("--timeout", type=float, default=3600.0)
    args = p.parse_args()
    return emit(args) if args.mode == "emit" else time_cell(args)


if __name__ == "__main__":
    sys.exit(main())
