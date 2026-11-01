# Checks made for the thesis

Measurements the thesis quotes in its chapter on the preconditioning skill, beyond the seven
the skill itself carries in `../skills/*/measurements/`.

| script | establishes |
|---|---|
| `jfnk_precond_across_sizes.py` | the fast-Poisson change to `jfnk_bratu` as a multiple of the fp64 budget at N = 32 to 128, next to where the reference starts hitting its Newton cap. Self-contained. Superseded: it assumes a Newton tolerance of 1e-10, which the reference has not had since 2026-09-09 (44dd51b9c), and the fp64 band. |
| `jfnk_precond_campaign.py` | the same change against the benchmark's own reference at 083c88e (Newton to 1e-14, so always 20 steps; GMRES capped at 50), lambda = 6 and 3, N = 32 to 704, in the fp64 band and in the campaign's raised tolerance (rtol = atol = 1e-5); also a reordering of the reference's sums. Output in `.out`. Table C.2 since 2026-11-01. Needs `HPCAGENT_BENCH`. |
| `skill_score_sensitivity.py` | the skill comparison of Section 6.6 restricted to kernels credited in both settings, and with every score below 1 raised to 1. Output in `.out`. |
| `pcg_nullspace_shift.py` | that a preconditioner swap in `sgs_pcg` moves the result both through the unconverged iterate and along the operator's null space, so it misses even when both solves converge. Needs `HPCAGENT_BENCH` pointing at a benchmark checkout. |

Run with NumPy and SciPy: `python jfnk_precond_across_sizes.py`.
