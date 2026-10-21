#!/usr/bin/env python3
"""Lines of code of NumPy references, for the size comparison in the thesis's Chapter 3.

Counts lines that hold code: no blank lines, comments, docstrings or other bare string statements.
Compares the NPBench kernels of HPCAgent-Bench (tag `npbench`) with the extracted kernels.

    python3 plotting/thesis/sloc.py <HPCAgent-Bench tree> extracted_kernels_lars

Thesis numbers (2026-10-21): HPCAgent-Bench release-v0.2.1 305559cc9; tag npbench has 56 kernels, 54 with
a <k>_numpy.py (gemm_long_k and gemm_tall_skinny have none): median 7.5, mean 14.3, range 2-123. The eight
extracted kernels: median 101, mean 183.4, range 10-532.
"""
import io, pathlib, statistics, sys, tokenize
def sloc(path):
    src = path.read_text(); lines = set(); prev = tokenize.INDENT
    for tok in tokenize.generate_tokens(io.StringIO(src).readline):
        if tok.type == tokenize.STRING and prev in (tokenize.INDENT, tokenize.NEWLINE, tokenize.DEDENT, tokenize.NL):
            prev = tok.type; continue                       # a docstring / bare string statement
        if tok.type not in (tokenize.COMMENT, tokenize.NL, tokenize.NEWLINE, tokenize.INDENT, tokenize.DEDENT, tokenize.ENDMARKER):
            lines.update(range(tok.start[0], tok.end[0] + 1))
        if tok.type not in (tokenize.COMMENT, tokenize.NL):
            prev = tok.type
    return len(lines)
bench, ours = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
tag = [l.strip() for l in (bench / "hpcagent_bench/tags/npbench.txt").read_text().splitlines() if l.strip() and not l.startswith("#")]
sc = bench / "hpcagent_bench/benchmarks"
npb = {}
for k in tag:
    hits = [p for p in sc.glob(f"**/{k}_numpy.py") if ".cache" not in p.parts]
    if hits: npb[k] = sloc(hits[0])
missing = [k for k in tag if k not in npb]
v = sorted(npb.values())
print(f"NPBench tag: {len(tag)} kernels, {len(npb)} with a _numpy.py; missing {missing}")
print(f"  SLOC median {statistics.median(v)}, mean {statistics.mean(v):.1f}, min {v[0]}, max {v[-1]}")
mine = {p.parent.name: sloc(p) for p in sorted(ours.glob("*/*/*_numpy.py"))}
for k, n in mine.items(): print(f"  {k:28s} {n}")
w = sorted(mine.values())
print(f"ours: median {statistics.median(w)}, mean {statistics.mean(w):.1f}, min {w[0]}, max {w[-1]}")
print("NPBench kernels at or above our median:", sum(x >= statistics.median(w) for x in v), "of", len(v))
