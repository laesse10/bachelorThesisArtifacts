# Extracted kernels (Lars Hulsbergen)

Kernels ported from upstream HPC applications into HPCAgent-Bench, sorted by
the 13 Berkeley dwarfs (Asanović et al., 2006). Copied from
`hpcagent_bench/benchmarks/scientific_computing/` and `tests/ports/` as of
`origin/main` commit `299e23df7`; the originals in the benchmark suite are unchanged.

| # | Dwarf | Kernel | Upstream | PR |
|---|-------|--------|----------|----|
| 1 | Dense Linear Algebra | `comet_int4_gemm` | CoMet (INT4 tensor-core GEMM) | #18 (supersedes #16) |
| 2 | Sparse Linear Algebra | `quatrex_rgf` | QuaTrEx (RGF selected solve) | #17 |
| 2 | Sparse Linear Algebra | `spgemm_hash` | SpBench (cuBool/nsparse boolean SpGEMM) | #21 || 3 | Spectral Methods | none | | |
| 4 | N-Body Methods | `warpx_boris_push` | WarpX | #11 |
| 4 | N-Body Methods | `warpx_esirkepov_deposition` | WarpX | #11 |
| 4 | N-Body Methods | `warpx_field_gather` | WarpX | #11 |
| 5 | Structured Grids | none | | |
| 6 | Unstructured Grids | none | | |
| 7 | MapReduce | none | | |
| 8 | Combinational Logic | none | | |
| 9 | Graph Traversal | `triangle_count` | GraphAIBench | #22 |
| 10 | Dynamic Programming | none | | |
| 11 | Backtrack and Branch-and-Bound | none | | |
| 12 | Graphical Models | none | | |
| 13 | Finite State Machines | `nfa_frontier` | ANMLZoo / VASim | #20 |

Each kernel folder holds the benchmark spec (`.yaml`), the input generator
(`<name>.py`, `initialize(...)`), the NumPy kernel (`<name>_numpy.py`), the
native reference (`_reference.cpp` / `.cu` / `.py`) and its tests.
