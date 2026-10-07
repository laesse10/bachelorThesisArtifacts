# Checks made for the thesis

Two measurements the thesis quotes in its chapter on the preconditioning skill, beyond the seven
the skill itself carries in `../skills/*/measurements/`.

| script | establishes |
|---|---|
| `jfnk_precond_across_sizes.py` | the fast-Poisson change to `jfnk_bratu` as a multiple of the fp64 budget at N = 32 to 128, next to where the reference starts hitting its Newton cap. Self-contained. |
| `pcg_nullspace_shift.py` | that a preconditioner swap in `sgs_pcg` moves the result both through the unconverged iterate and along the operator's null space, so it misses even when both solves converge. Needs `HPCAGENT_BENCH` pointing at a benchmark checkout. |

Run with NumPy and SciPy: `python jfnk_precond_across_sizes.py`.
