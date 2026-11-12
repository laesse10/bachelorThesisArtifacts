# solver14 rerun (2026-11-11)

A rerun of the skill campaign on the fourteen solver kernels: HPCAgent-Bench commit `3500cf44cf11`,
C, MI300A, one episode per kernel and setting. The skill pages shipped are byte-identical to
`Solvers/skills/v4-no-tools/single/` (v2-single) and `split/` (v2-split).

| setting | job | state |
|---|---|---|
| qwen38, none | 734916 | completed |
| qwen38, v2-single | 734910 | node failure after 4:20 h; no results database, 4 /score calls, no answers |
| qwen38, v2-split | 734911 | completed |
| oss120b, none | 734917 | completed |
| oss120b, v2-single | 734918 | completed |
| oss120b, v2-split | 734919 | completed |

The raw run root (`solver14-20261111.tar.xz`, 1 GB; 12 GB unpacked, with transcripts and workspaces)
is not in the repository. `summarize.py` reads it from `../solver14-20261111/` and writes
`scores.csv` (per kernel) and `summary.csv` (per setting, the rerun rows of the thesis's Table A.6).

Score: the credited speedup of the episode's final (or promoted) grade, over the faster of C and
Numba; an uncredited kernel counts as 1. Two episodes have none: oss120b none `bdf_newton_krylov`
(no correct answer) and oss120b v2-split `householder_qr` (submitted a wrong answer).
