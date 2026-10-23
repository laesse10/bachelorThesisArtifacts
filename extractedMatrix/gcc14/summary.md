# Extracted kernels with GCC 14.2 for every language (job 5121173)

Speedup over translated C (GCC 14.2), median of the 90 timed runs of three interleaved rounds per cell, one node per kernel. `cpp`/`fortran`: GCC 13.3 (the benchmark's default build); `cpp14`/`fortran14`: the same sources built by GCC 14.2. `matrix`: the same ratio in the matrix (job 4990441, medians).

| kernel | C (ms) | C++ 13 | C++ 14 | Fortran 13 | Fortran 14 | matrix C++ | matrix Fortran |
|---|---:|---:|---:|---:|---:|---:|---:|
| `comet_int4_gemm` | 0.089 | 1.00 | 1.02 | 0.23 | 0.23 | 0.99 | 0.19 |
| `quatrex_rgf` | 7.332 | 1.00 | 1.00 | 1.14 | 1.14 | 1.00 | 1.15 |
| `spgemm_hash` | 261.947 | 1.00 | 1.00 | 1.44 | 1.40 | 1.00 | 1.44 |
| `warpx_boris_push` | 3.471 | 1.02 | 0.98 | 0.95 | 0.96 | 1.03 | 0.94 |
| `warpx_esirkepov_deposition` | 2.905 | 1.08 | 1.00 | 0.97 | 0.98 | 1.01 | 0.93 |
| `warpx_field_gather` | 12.285 | 0.98 | 1.01 | 0.91 | 0.92 | 0.97 | 0.91 |
| `triangle_count` | 46.560 | 0.97 | 1.00 | 0.97 | 1.04 | 0.97 | 0.97 |
| `nfa_frontier` | 9729.835 | 1.04 | 1.01 | 1.02 | 1.02 | 1.04 | 1.01 |

| column | geomean over C | 95% CI | within 3% of C |
|---|---:|---|---:|
| cpp | 1.010 | 0.980 - 1.041 | 5 of 8 |
| cpp14 | 1.002 | 0.994 - 1.011 | 8 of 8 |
| fortran | 0.866 | 0.547 - 1.371 | 1 of 8 |
| fortran14 | 0.875 | 0.553 - 1.386 | 2 of 8 |

Cell statuses: ['ok']. Compiler stamps that do not match their label: none.
