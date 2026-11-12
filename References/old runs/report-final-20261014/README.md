# solver14 at the 120M-token / 20 h budget (final, regraded 2026-10-14)

C, CPU (MI300A node), one agent per kernel, the 14 `solvers` kernels, temperature 1.0; each model with no skill packet
and with the `solver` packet (`+solver`). Agents ran on hb `budget-120m-20h` (PR #51 budget). All six arms are finished.
Every final was re-timed on 2026-10-14 under the paper's grading configuration (hb `a7437a91b`, grade-under job 680631):
77/77 finals graded correct, new/old credited geomean 1.055.

## Contents

- `solver14.db`: the results DB (every `/score`, `/submit`, promoted score, final re-timing and episode record).
- `plots/`: per-kernel speedup + billed tokens (`solver14-per-kernel.*`), packet-vs-none score change (`solver14-score-change.*`)
  and the tables behind them (`*.csv`).
- `submitted/<setup>/`: every `/submit` source (`<kernel>.submit.callNN.host.c`) and the source each final re-timed
  (`<kernel>.final.<credited>x.host.c`).
- `scored/<setup>/`: every `/score` source, `<kernel>.callNN.<correct|incorrect>.host.c`.
- `csv/`: the DB's `setups`, `episodes` and `grades` tables.

## Summary

| setup | episodes | finals (correct) | geomean credited | mean credited | /score calls (correct) | submits + promoted | billed tokens | agent wall |
|---|---|---|---|---|---|---|---|---|
| qwen38 | 14 | 12 (12) | 7.38x | 44.08x | 103 (93) | 10 + 4 | 358.3 M | 207.9 h |
| qwen38 +solver | 21 | 14 (14) | 6.73x | 38.08x | 90 (77) | 11 + 3 | 325.6 M | 156.7 h |
| oss120b | 16 | 13 (13) | 2.55x | 7.08x | 70 (41) | 9 + 4 | 44.3 M | 7.0 h |
| oss120b +solver | 16 | 13 (13) | 2.53x | 6.60x | 69 (42) | 9 + 5 | 32.0 M | 7.4 h |
| glm53 | 22 | 13 (13) | 8.75x | 37.58x | 140 (128) | 6 + 7 | 364.5 M | 138.8 h |
| glm53 +solver | 14 | 12 (12) | 12.38x | 58.06x | 124 (111) | 9 + 4 | 384.6 M | 162.3 h |

## Per kernel: credited final speedup

| kernel | qwen38 | qwen38 +solver | oss120b | oss120b +solver | glm53 | glm53 +solver |
|---|---|---|---|---|---|---|
| amg_setup | owed | [9.16x](submitted/qwen38-c-solver/amg_setup.final.9.16x.host.c) | owed | [1.07x](submitted/oss120b-c-solver/amg_setup.final.1.07x.host.c) | [17.16x](submitted/glm53-c/amg_setup.final.17.16x.host.c) | [10.57x](submitted/glm53-c-solver/amg_setup.final.10.57x.host.c) |
| bdf_newton_krylov | [1.40x](submitted/qwen38-c/bdf_newton_krylov.final.1.40x.host.c) | [1.08x](submitted/qwen38-c-solver/bdf_newton_krylov.final.1.08x.host.c) | [1.65x](submitted/oss120b-c/bdf_newton_krylov.final.1.65x.host.c) | [0.52x](submitted/oss120b-c-solver/bdf_newton_krylov.final.0.52x.host.c) | [1.42x](submitted/glm53-c/bdf_newton_krylov.final.1.42x.host.c) | owed |
| householder_qr | [7.68x](submitted/qwen38-c/householder_qr.final.7.68x.host.c) | [2.75x](submitted/qwen38-c-solver/householder_qr.final.2.75x.host.c) | [0.22x](submitted/oss120b-c/householder_qr.final.0.22x.host.c) | [1.39x](submitted/oss120b-c-solver/householder_qr.final.1.39x.host.c) | owed | [24.46x](submitted/glm53-c-solver/householder_qr.final.24.46x.host.c) |
| ilu0 | [3.81x](submitted/qwen38-c/ilu0.final.3.81x.host.c) | [4.46x](submitted/qwen38-c-solver/ilu0.final.4.46x.host.c) | [1.09x](submitted/oss120b-c/ilu0.final.1.09x.host.c) | [1.11x](submitted/oss120b-c-solver/ilu0.final.1.11x.host.c) | [0.75x](submitted/glm53-c/ilu0.final.0.75x.host.c) | [3.77x](submitted/glm53-c-solver/ilu0.final.3.77x.host.c) |
| jfnk_bratu | [26.93x](submitted/qwen38-c/jfnk_bratu.final.26.93x.host.c) | [24.27x](submitted/qwen38-c-solver/jfnk_bratu.final.24.27x.host.c) | [9.51x](submitted/oss120b-c/jfnk_bratu.final.9.51x.host.c) | owed | [26.08x](submitted/glm53-c/jfnk_bratu.final.26.08x.host.c) | [17.06x](submitted/glm53-c-solver/jfnk_bratu.final.17.06x.host.c) |
| lanczos_reorth | [1.90x](submitted/qwen38-c/lanczos_reorth.final.1.90x.host.c) | [2.70x](submitted/qwen38-c-solver/lanczos_reorth.final.2.70x.host.c) | [0.80x](submitted/oss120b-c/lanczos_reorth.final.0.80x.host.c) | [1.24x](submitted/oss120b-c-solver/lanczos_reorth.final.1.24x.host.c) | [6.81x](submitted/glm53-c/lanczos_reorth.final.6.81x.host.c) | [4.41x](submitted/glm53-c-solver/lanczos_reorth.final.4.41x.host.c) |
| mg_vcycle | [5.63x](submitted/qwen38-c/mg_vcycle.final.5.63x.host.c) | [6.66x](submitted/qwen38-c-solver/mg_vcycle.final.6.66x.host.c) | [1.10x](submitted/oss120b-c/mg_vcycle.final.1.10x.host.c) | [2.20x](submitted/oss120b-c-solver/mg_vcycle.final.2.20x.host.c) | [4.86x](submitted/glm53-c/mg_vcycle.final.4.86x.host.c) | [8.60x](submitted/glm53-c-solver/mg_vcycle.final.8.60x.host.c) |
| mixed_precision_ir | [1.98x](submitted/qwen38-c/mixed_precision_ir.final.1.98x.host.c) | [3.19x](submitted/qwen38-c-solver/mixed_precision_ir.final.3.19x.host.c) | [3.73x](submitted/oss120b-c/mixed_precision_ir.final.3.73x.host.c) | [3.95x](submitted/oss120b-c-solver/mixed_precision_ir.final.3.95x.host.c) | [3.67x](submitted/glm53-c/mixed_precision_ir.final.3.67x.host.c) | [8.17x](submitted/glm53-c-solver/mixed_precision_ir.final.8.17x.host.c) |
| rb_sor | [8.04x](submitted/qwen38-c/rb_sor.final.8.04x.host.c) | [7.97x](submitted/qwen38-c-solver/rb_sor.final.7.97x.host.c) | [8.38x](submitted/oss120b-c/rb_sor.final.8.38x.host.c) | [8.52x](submitted/oss120b-c-solver/rb_sor.final.8.52x.host.c) | [30.64x](submitted/glm53-c/rb_sor.final.30.64x.host.c) | [10.17x](submitted/glm53-c-solver/rb_sor.final.10.17x.host.c) |
| rk45_ensemble | [142.11x](submitted/qwen38-c/rk45_ensemble.final.142.11x.host.c) | [32.79x](submitted/qwen38-c-solver/rk45_ensemble.final.32.79x.host.c) | [21.48x](submitted/oss120b-c/rk45_ensemble.final.21.48x.host.c) | [21.65x](submitted/oss120b-c-solver/rk45_ensemble.final.21.65x.host.c) | [134.08x](submitted/glm53-c/rk45_ensemble.final.134.08x.host.c) | [199.60x](submitted/glm53-c-solver/rk45_ensemble.final.199.60x.host.c) |
| rk4_ensemble | [325.86x](submitted/qwen38-c/rk4_ensemble.final.325.86x.host.c) | [428.43x](submitted/qwen38-c-solver/rk4_ensemble.final.428.43x.host.c) | [40.09x](submitted/oss120b-c/rk4_ensemble.final.40.09x.host.c) | [40.08x](submitted/oss120b-c-solver/rk4_ensemble.final.40.08x.host.c) | [251.76x](submitted/glm53-c/rk4_ensemble.final.251.76x.host.c) | [406.43x](submitted/glm53-c-solver/rk4_ensemble.final.406.43x.host.c) |
| sgs_pcg | [2.47x](submitted/qwen38-c/sgs_pcg.final.2.47x.host.c) | [5.62x](submitted/qwen38-c-solver/sgs_pcg.final.5.62x.host.c) | [1.70x](submitted/oss120b-c/sgs_pcg.final.1.70x.host.c) | [1.49x](submitted/oss120b-c-solver/sgs_pcg.final.1.49x.host.c) | [7.25x](submitted/glm53-c/sgs_pcg.final.7.25x.host.c) | owed |
| sparse_cholesky | owed | [2.57x](submitted/qwen38-c-solver/sparse_cholesky.final.2.57x.host.c) | [1.02x](submitted/oss120b-c/sparse_cholesky.final.1.02x.host.c) | [1.41x](submitted/oss120b-c-solver/sparse_cholesky.final.1.41x.host.c) | [2.00x](submitted/glm53-c/sparse_cholesky.final.2.00x.host.c) | [2.05x](submitted/glm53-c-solver/sparse_cholesky.final.2.05x.host.c) |
| sptrsv_level | [1.21x](submitted/qwen38-c/sptrsv_level.final.1.21x.host.c) | [1.43x](submitted/qwen38-c-solver/sptrsv_level.final.1.43x.host.c) | [1.26x](submitted/oss120b-c/sptrsv_level.final.1.26x.host.c) | [1.23x](submitted/oss120b-c-solver/sptrsv_level.final.1.23x.host.c) | [2.03x](submitted/glm53-c/sptrsv_level.final.2.03x.host.c) | [1.48x](submitted/glm53-c-solver/sptrsv_level.final.1.48x.host.c) |

## Owed kernels (no graded `/submit` or promoted score)

- solver14-qwen38-c: amg_setup, sparse_cholesky
- solver14-qwen38-c-solver: none
- solver14-oss120b-c: amg_setup
- solver14-oss120b-c-solver: jfnk_bratu
- solver14-glm53-c: householder_qr
- solver14-glm53-c-solver: bdf_newton_krylov, sgs_pcg

## Packet effect (solver vs none, paired over kernels both arms solved)

- glm53: score x1.11, token cost x1.32 over 11 kernels, not-significant (BH-adjusted p 0.79)
- oss120b: score x1.19, token cost x1.14 over 12 kernels, not-significant (BH-adjusted p 0.79)
- qwen38: score x0.94, token cost x0.84 over 11 kernels, not-significant (BH-adjusted p 0.79)
