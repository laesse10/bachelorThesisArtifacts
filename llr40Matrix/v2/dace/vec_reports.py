#!/usr/bin/env python3
"""Compile-only: which loops GCC vectorises in each emitted DaCe cell, under three flag sets.

For every ``<emit-root>/<kernel>/<pipeline>_<codegen>/src/cpu/canon_cpu.cpp`` written by
``dace_cells.py emit``, and for each compiler in --cxx, compile to an object file with
``-fopt-info-vec-all`` and record, per loop of canon_cpu.cpp,
whether it was vectorised (loops named file:line; a library reduction's loop is in a DaCe header). Flag sets:

  harness  what the dace_cpu_canonicalize column builds with: the args DaCe was pinned to (recorded in
           the emit log as cxx_args), plus the flags DaCe's CMake adds itself (-O3 -fPIC -fopenmp and
           the pinned C++ standard)
  bonsall  the flag set of Bonsall's report: -O3 -fPIC -fno-math-errno -ffinite-math-only
           -fno-signaling-nans -fstrict-aliasing -faligned-new, with GCC's unlimited cost model (the only
           setting the report kept), and -fopenmp so DaCe's pragmas mean what they mean in a DaCe build.
           One deviation: -std=c++20 instead of the report's -std=c++17, because the runtime headers of
           spcl/dace@dea0b39c use std::bit_cast and do not compile as C++17
  unsafe   bonsall plus -funsafe-math-optimizations, the flag the report found restores vectorisation
           of s453 and s314

Writes one CSV row per (kernel, pipeline, codegen, cxx, flagset) with the vectorised and missed loop
lines, and keeps every report under <out>/reports/.
"""
import argparse
import csv
import json
import pathlib
import re
import subprocess
from concurrent.futures import ThreadPoolExecutor

FLAGSETS = {
    "bonsall": "-O3 -std=c++20 -fPIC -fno-math-errno -ffinite-math-only -fno-signaling-nans -fstrict-aliasing "
               "-faligned-new -fvect-cost-model=unlimited -fopenmp",
}
FLAGSETS["unsafe"] = FLAGSETS["bonsall"] + " -funsafe-math-optimizations"
# Loops are named file:line by basename: a library reduction (dace::reduce::sum/min/max) is a loop in DaCe's
# runtime headers, inlined into the kernel, and GCC reports it at the header's line.
OPT = re.compile(r"([^\s:]+):(\d+):\d+: optimized: (.*)")
MISS = re.compile(r"([^\s:]+):(\d+):\d+: missed: (couldn't vectorize loop|not vectorized: .*)")


def harness_flags(emit_log: pathlib.Path) -> dict[tuple[str, str, str], str]:
    """``(kernel, pipeline, codegen) -> args`` from the EMIT lines; DaCe's CMake adds -O3 -fPIC, OpenMP and
    the standard (pinned to c++20 by pin_cpp_standard at this commit)."""
    out = {}
    for line in emit_log.read_text().splitlines():
        if line.startswith("EMIT "):
            row = json.loads(line[5:])
            out[(row["kernel"], row["pipeline"], row["codegen"])] = f"{row['cxx_args']} -O3 -fPIC -fopenmp -std=c++20"
    return out


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--emit-root", required=True)
    p.add_argument("--emit-log", required=True)
    p.add_argument("--dace-include", required=True, help="<dace>/dace/runtime/include")
    p.add_argument("--cxx", default="g++-13,g++-14")
    p.add_argument("--out", required=True)
    p.add_argument("--jobs", type=int, default=1, help="compilations in parallel (compile only, so this is safe)")
    args = p.parse_args()
    root, out = pathlib.Path(args.emit_root), pathlib.Path(args.out)
    (out / "reports").mkdir(parents=True, exist_ok=True)
    harness = harness_flags(pathlib.Path(args.emit_log))
    jobs = []
    for src in sorted(root.glob("*/*_*/src/cpu/canon_cpu.cpp")):
        cell = src.parents[2]
        kernel = cell.parent.name
        pipeline, codegen = cell.name.split("_", 1)
        flagsets = dict(FLAGSETS)
        if (kernel, pipeline, codegen) in harness:
            flagsets["harness"] = harness[(kernel, pipeline, codegen)]
        for cxx in args.cxx.split(","):
            for name, flags in flagsets.items():
                jobs.append((src, kernel, pipeline, codegen, cxx, name, flags))

    def compile_one(job):
        src, kernel, pipeline, codegen, cxx, name, flags = job
        stem = str(out / "reports" / f"{kernel}.{pipeline}_{codegen}.{cxx}.{name}")
        # One -fopt-info-vec-all file (GCC writes only one of two -fopt-info-vec-* files), cut down to the
        # optimized lines (.opt) and the per-loop verdicts and reasons (.missed); the full file is dropped.
        cmd = [cxx, *flags.split(), f"-I{args.dace_include}", "-c", str(src), "-o", "/dev/null",
               f"-fopt-info-vec-all={stem}.all"]
        r = subprocess.run(cmd, capture_output=True, text=True)
        full = pathlib.Path(stem + ".all")
        text = full.read_text() if full.exists() else ""
        opt = "".join(l + "\n" for l in text.splitlines() if ": optimized: " in l)
        miss = "".join(l + "\n" for l in text.splitlines()
                       if "missed: couldn't vectorize loop" in l or "missed: not vectorized:" in l)
        pathlib.Path(stem + ".opt").write_text(opt)
        pathlib.Path(stem + ".missed").write_text(miss)
        full.unlink(missing_ok=True)
        loop = lambda m: f"{pathlib.Path(m.group(1)).name}:{m.group(2)}"
        vec = sorted({loop(m) for m in OPT.finditer(opt) if "loop vectorized" in m.group(3)})
        missed = sorted({loop(m) for m in MISS.finditer(miss)} - set(vec))
        return {
            "kernel": kernel, "pipeline": pipeline, "codegen": codegen, "cxx": cxx, "flagset": name,
            "compiled": r.returncode == 0, "n_vectorized": len(vec), "vectorized_loops": " ".join(vec),
            "n_missed": len(missed), "missed_loops": " ".join(missed),
            "error": r.stderr.strip().splitlines()[-1] if r.returncode else "", "argv": " ".join(cmd),
        }

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        rows = list(pool.map(compile_one, jobs))
    with open(out / "vec_reports.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows(rows)
    print(f"{len(rows)} rows -> {out / 'vec_reports.csv'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
