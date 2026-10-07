# Agent C against agent Fortran on GH200 (Part B)

**Question.** Do LLM agents write faster code in C or in Fortran? In the campaign data (AMD MI300A,
multi-core), C looks 1.17x faster when each side takes its BEST submission, but the difference vanishes
for the first submission (1.01x, p = 0.93) and for the best of an equal number of tries (1.12x,
p = 0.42). Here every one of the 518 submissions of the six `llr40v10` arms (3 models x {C, Fortran};
313 C, 205 Fortran, 39 kernels) was built, validated and timed on GH200 (aarch64). Every source was
sha256-checked against `../agent_picks.json` before use; there were no mismatches.

**Answer: no reliable language difference.**
- At **one thread** (T1), C and Fortran are indistinguishable. The pooled geometric mean of
  t_Fortran / t_C is 0.96-1.04 across the four statistics, and every 95% interval contains 1. Only
  the median reaches p = 0.047, and it does not survive a correction for the 16 pooled tests
  (Bonferroni 0.003).
- On **one Grace socket** (T72), C trends faster: 1.07-1.25x on the geometric mean, with intervals
  touching 1 and p = 0.06-0.67. That is not significant.
- With **GCC 14.2 for both languages** (T1), the picture is the same as the default build.
- **Portability, the clearest result:** 96% of Fortran submissions validate on aarch64, against 79%
  of C. 55 C submissions (18%) fail to build on x86 intrinsics.

## What was run

Each run is the LLR-40 v2 harness path (`hpcagent_bench.cli run --validate`, preset M, float64,
5 warm-up + 30 timed runs, min-of-k), with the submission substituted for the kernel's
`cpp_backend/<k>_fp64.{c,f90}` exactly as v2 substitutes its agent cell, used as delivered.

| run | binding | build |
|---|---|---|
| default / T1 | one core (cpu 0), memory on NUMA node 0, `--mode single_core` | harness default: C gcc 14.2.0 (SUSE), Fortran gfortran 13.3.1 |
| default / T72 | one Grace socket (`numactl --cpunodebind=0 --membind=0`), `--mode multi_core`, OMP_NUM_THREADS=72, OMP_PLACES=cores, OMP_PROC_BIND=close | same compilers; valid (T1) candidates only |
| gcc14 / T1 | as default / T1 | `uenv prgenv-gnu/25.6:v2`: Spack GCC 14.2.0 for both languages |
| translated C | T1 and T72, once per kernel | the harness's own lowering (reference) |

Rows: `results.csv`, one per (candidate, build, threads), with every timing. Jobs: pilot 4992464
(`PILOT.md`), chain 4992486 / 4992611 / 4992794 / 4993100, tail 4995283, T72 re-timing
4995282 / 4996902. Deviations: `DEVIATIONS.md`.

**T72 and the T1 binary.** The harness rebuilds the library at every run (DEVIATIONS 4), so each T72
row records whether its library is byte-identical to the default/T1 library. **All C T72 binaries
are identical. 77 of 196 Fortran T72 binaries differ**, because gfortran's
`-ftree-parallelize-loops={n}` takes n from the affinity (72 at T72) and autoparallelised some
loops. The main T72 tables therefore show the harness default at T72. A sensitivity table restricted
to identical binaries follows them.

37 C and 9 Fortran submissions set their own thread count (`sets_thread_count`); they were not patched.

## Definitions

Per (model, kernel, language), over that model's submissions in campaign order (job, then seq):
**best** = the fastest valid submission; **FIRST** = the lowest seq of the earliest job, missing if it
is not valid; **median** = the median over valid submissions; **best of first k** = the fastest valid
submission among each language's first k, with k = min(#C, #Fortran) for that (model, kernel).
Pairs need both languages. The ratio is t_Fortran / t_C (> 1: C faster). Reported: the geometric mean
with a 95% interval from the t distribution on log ratios, the Wilcoxon signed-rank p (two-sided,
`**` marks p < 0.05), and the counts C better (ratio > 1.1) / Fortran better (< 1/1.1) / within 10%.

## Results

### Build and validation on aarch64

| build | language | model | submissions | build | validate | x86 intrinsics | other build error | incorrect | timeout |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|
| default | c | all | 313 | 255 (82%) | 246 (79%) | 55 | 3 | 8 | 1 |
| default | c | kimi27sglang | 120 | 92 (77%) | 89 (74%) | 25 | 3 | 3 | 0 |
| default | c | oss120b | 70 | 68 (97%) | 66 (94%) | 2 | 0 | 2 | 0 |
| default | c | qwen38 | 123 | 95 (77%) | 91 (74%) | 28 | 0 | 3 | 1 |
| default | fortran | all | 205 | 202 (98%) | 196 (96%) | 0 | 3 | 6 | 0 |
| default | fortran | kimi27sglang | 67 | 67 (100%) | 66 (98%) | 0 | 0 | 1 | 0 |
| default | fortran | oss120b | 67 | 65 (97%) | 63 (94%) | 0 | 2 | 2 | 0 |
| default | fortran | qwen38 | 71 | 70 (99%) | 67 (94%) | 0 | 1 | 3 | 0 |
| gcc14 | c | all | 313 | 255 (82%) | 246 (79%) | 55 | 3 | 8 | 1 |
| gcc14 | c | kimi27sglang | 120 | 92 (77%) | 89 (74%) | 25 | 3 | 3 | 0 |
| gcc14 | c | oss120b | 70 | 68 (97%) | 66 (94%) | 2 | 0 | 2 | 0 |
| gcc14 | c | qwen38 | 123 | 95 (77%) | 91 (74%) | 28 | 0 | 3 | 1 |
| gcc14 | fortran | all | 205 | 202 (98%) | 196 (96%) | 0 | 3 | 6 | 0 |
| gcc14 | fortran | kimi27sglang | 67 | 67 (100%) | 66 (98%) | 0 | 0 | 1 | 0 |
| gcc14 | fortran | oss120b | 67 | 65 (97%) | 63 (94%) | 0 | 2 | 2 | 0 |
| gcc14 | fortran | qwen38 | 71 | 70 (99%) | 67 (94%) | 0 | 1 | 3 | 0 |

### C against Fortran: default build, T1

Ratio t_Fortran / t_C per (model, kernel); > 1 means C is faster.

| statistic | model | pairs | geomean F/C | 95% interval | Wilcoxon p | C better | Fortran better | within 10% |
|---|---|---:|---:|---|---:|---:|---:|---:|
| best | pooled | 81 | 1.000 | 0.853-1.173 | 0.2675 | 8 | 15 | 58 |
| best | kimi27sglang | 22 | 0.958 | 0.694-1.323 | 0.9746 | 1 | 5 | 16 |
| best | oss120b | 32 | 0.994 | 0.736-1.342 | 0.5119 | 5 | 7 | 20 |
| best | qwen38 | 27 | 1.043 | 0.828-1.315 | 0.2291 | 2 | 3 | 22 |
| FIRST | pooled | 75 | 1.036 | 0.864-1.243 | 0.4759 | 19 | 16 | 40 |
| FIRST | kimi27sglang | 21 | 0.902 | 0.633-1.284 | 0.3205 | 3 | 7 | 11 |
| FIRST | oss120b | 30 | 1.066 | 0.729-1.558 | 0.7151 | 8 | 7 | 15 |
| FIRST | qwen38 | 24 | 1.130 | 0.956-1.337 | 0.0691 | 8 | 2 | 14 |
| median | pooled | 81 | 1.038 | 0.877-1.230 | 0.0467 ** | 19 | 16 | 46 |
| median | kimi27sglang | 22 | 0.984 | 0.702-1.379 | 0.8486 | 5 | 6 | 11 |
| median | oss120b | 32 | 1.015 | 0.730-1.412 | 0.2618 | 8 | 7 | 17 |
| median | qwen38 | 27 | 1.115 | 0.889-1.397 | 0.0362 ** | 6 | 3 | 18 |
| best of first k | pooled | 79 | 0.957 | 0.827-1.108 | 0.5741 | 11 | 17 | 51 |
| best of first k | kimi27sglang | 22 | 0.822 | 0.652-1.036 | 0.4628 | 1 | 7 | 14 |
| best of first k | oss120b | 30 | 0.990 | 0.717-1.365 | 0.7303 | 5 | 7 | 18 |
| best of first k | qwen38 | 27 | 1.044 | 0.871-1.251 | 0.2109 | 5 | 3 | 19 |

### C against Fortran: default build, T72

Ratio t_Fortran / t_C per (model, kernel); > 1 means C is faster.

| statistic | model | pairs | geomean F/C | 95% interval | Wilcoxon p | C better | Fortran better | within 10% |
|---|---|---:|---:|---|---:|---:|---:|---:|
| best | pooled | 81 | 1.190 | 0.988-1.434 | 0.0562 | 20 | 10 | 51 |
| best | kimi27sglang | 22 | 1.447 | 1.050-1.993 | 0.0156 ** | 8 | 0 | 14 |
| best | oss120b | 32 | 1.043 | 0.793-1.371 | 0.8034 | 6 | 7 | 19 |
| best | qwen38 | 27 | 1.188 | 0.797-1.772 | 0.1414 | 6 | 3 | 18 |
| FIRST | pooled | 75 | 1.070 | 0.790-1.449 | 0.6650 | 19 | 16 | 40 |
| FIRST | kimi27sglang | 21 | 1.323 | 0.792-2.212 | 0.3554 | 5 | 2 | 14 |
| FIRST | oss120b | 30 | 0.919 | 0.631-1.337 | 0.8872 | 7 | 9 | 14 |
| FIRST | qwen38 | 24 | 1.076 | 0.509-2.273 | 0.8774 | 7 | 5 | 12 |
| median | pooled | 81 | 1.215 | 1.001-1.474 | 0.1254 | 23 | 13 | 45 |
| median | kimi27sglang | 22 | 1.735 | 1.142-2.635 | 0.0083 ** | 8 | 0 | 14 |
| median | oss120b | 32 | 0.942 | 0.708-1.253 | 0.8609 | 7 | 9 | 16 |
| median | qwen38 | 27 | 1.228 | 0.880-1.713 | 0.5460 | 8 | 4 | 15 |
| best of first k | pooled | 79 | 1.249 | 0.997-1.565 | 0.1637 | 21 | 12 | 46 |
| best of first k | kimi27sglang | 22 | 1.510 | 1.037-2.199 | 0.1982 | 7 | 1 | 14 |
| best of first k | oss120b | 30 | 1.062 | 0.785-1.436 | 0.9838 | 6 | 7 | 17 |
| best of first k | qwen38 | 27 | 1.283 | 0.770-2.137 | 0.2792 | 8 | 4 | 15 |

### C against Fortran: default build, T72, only T72 binary identical to T1

Ratio t_Fortran / t_C per (model, kernel); > 1 means C is faster.

| statistic | model | pairs | geomean F/C | 95% interval | Wilcoxon p | C better | Fortran better | within 10% |
|---|---|---:|---:|---|---:|---:|---:|---:|
| best | pooled | 50 | 1.297 | 1.000-1.684 | 0.2491 | 14 | 8 | 28 |
| best | kimi27sglang | 13 | 1.844 | 1.090-3.119 | 0.0105 ** | 6 | 0 | 7 |
| best | oss120b | 21 | 0.979 | 0.664-1.442 | 0.4120 | 4 | 6 | 11 |
| best | qwen38 | 16 | 1.411 | 0.839-2.373 | 0.5282 | 4 | 2 | 10 |
| FIRST | pooled | 43 | 1.268 | 0.801-2.006 | 0.3785 | 14 | 11 | 18 |
| FIRST | kimi27sglang | 12 | 1.651 | 0.645-4.229 | 0.1514 | 5 | 1 | 6 |
| FIRST | oss120b | 19 | 0.696 | 0.413-1.171 | 0.1688 | 3 | 9 | 7 |
| FIRST | qwen38 | 12 | 2.518 | 0.811-7.819 | 0.0923 | 6 | 1 | 5 |
| median | pooled | 50 | 1.212 | 0.900-1.632 | 0.5786 | 15 | 12 | 23 |
| median | kimi27sglang | 13 | 2.328 | 1.227-4.417 | 0.0105 ** | 8 | 0 | 5 |
| median | oss120b | 21 | 0.770 | 0.523-1.133 | 0.0646 | 2 | 9 | 10 |
| median | qwen38 | 16 | 1.293 | 0.747-2.239 | 0.5282 | 5 | 3 | 8 |
| best of first k | pooled | 48 | 1.536 | 1.125-2.098 | 0.0431 ** | 17 | 7 | 24 |
| best of first k | kimi27sglang | 13 | 2.018 | 1.092-3.730 | 0.0479 ** | 6 | 0 | 7 |
| best of first k | oss120b | 20 | 1.057 | 0.682-1.637 | 1.0000 | 5 | 6 | 9 |
| best of first k | qwen38 | 15 | 1.998 | 1.026-3.892 | 0.0730 | 6 | 1 | 8 |

### C against Fortran: GCC 14.2 for both languages, T1

Ratio t_Fortran / t_C per (model, kernel); > 1 means C is faster.

| statistic | model | pairs | geomean F/C | 95% interval | Wilcoxon p | C better | Fortran better | within 10% |
|---|---|---:|---:|---|---:|---:|---:|---:|
| best | pooled | 81 | 1.014 | 0.863-1.192 | 0.8488 | 8 | 15 | 58 |
| best | kimi27sglang | 22 | 1.007 | 0.697-1.454 | 0.4826 | 2 | 5 | 15 |
| best | oss120b | 32 | 0.969 | 0.716-1.313 | 0.9485 | 4 | 8 | 20 |
| best | qwen38 | 27 | 1.077 | 0.891-1.301 | 0.2901 | 2 | 2 | 23 |
| FIRST | pooled | 75 | 1.049 | 0.876-1.255 | 0.7918 | 17 | 16 | 42 |
| FIRST | kimi27sglang | 21 | 0.900 | 0.639-1.268 | 0.0384 ** | 3 | 6 | 12 |
| FIRST | oss120b | 30 | 1.104 | 0.760-1.605 | 0.8553 | 7 | 8 | 15 |
| FIRST | qwen38 | 24 | 1.124 | 0.943-1.340 | 0.2897 | 7 | 2 | 15 |
| median | pooled | 81 | 1.086 | 0.918-1.285 | 0.0351 ** | 23 | 13 | 45 |
| median | kimi27sglang | 22 | 1.052 | 0.731-1.515 | 0.8736 | 7 | 4 | 11 |
| median | oss120b | 32 | 1.039 | 0.752-1.437 | 0.2781 | 8 | 7 | 17 |
| median | qwen38 | 27 | 1.173 | 0.957-1.438 | 0.0245 ** | 8 | 2 | 17 |
| best of first k | pooled | 79 | 0.983 | 0.845-1.144 | 0.7582 | 11 | 17 | 51 |
| best of first k | kimi27sglang | 22 | 0.865 | 0.652-1.148 | 0.2099 | 2 | 7 | 13 |
| best of first k | oss120b | 30 | 0.964 | 0.696-1.333 | 0.5978 | 4 | 8 | 18 |
| best of first k | qwen38 | 27 | 1.116 | 0.949-1.312 | 0.1937 | 5 | 2 | 20 |

### Campaign speedup (MI300A) against GH200 speedup over translated C, per arm

| arm | threads | n | Spearman rho | p |
|---|---:|---:|---:|---:|
| llr40v10-kimi27sglang-c | T1 | 89 | 0.333 | 0.001446 |
| llr40v10-kimi27sglang-c | T72 | 88 | 0.411 | 6.813e-05 |
| llr40v10-kimi27sglang-fortran | T1 | 66 | 0.120 | 0.337 |
| llr40v10-kimi27sglang-fortran | T72 | 66 | 0.510 | 1.198e-05 |
| llr40v10-oss120b-c | T1 | 66 | 0.655 | 2.362e-09 |
| llr40v10-oss120b-c | T72 | 66 | 0.875 | 7.451e-22 |
| llr40v10-oss120b-fortran | T1 | 63 | 0.477 | 7.648e-05 |
| llr40v10-oss120b-fortran | T72 | 63 | 0.883 | 9.79e-22 |
| llr40v10-qwen38-c | T1 | 91 | 0.444 | 1.021e-05 |
| llr40v10-qwen38-c | T72 | 91 | 0.581 | 1.546e-09 |
| llr40v10-qwen38-fortran | T1 | 67 | 0.344 | 0.004424 |
| llr40v10-qwen38-fortran | T72 | 67 | 0.641 | 4.995e-09 |

## Significance, stated plainly

- 16 pooled C-against-Fortran tests (4 statistics x 4 settings) were run. Three have p < 0.05:
  - median at T1, default build (1.038, p = 0.047);
  - median at T1, GCC 14.2 (1.086, p = 0.035);
  - best of first k at T72, identical binaries only (1.536, p = 0.043, n = 48).
  None survives Bonferroni (0.05/16 = 0.003). Their intervals reach 1 or come close (1.125-2.098 for
  the last). These are **not significant** once multiple testing is accounted for. The campaign's
  BEST-submission advantage for C does not reproduce at T1 on GH200 (1.000, p = 0.27). At T72 it
  appears as a trend of similar size (1.19, p = 0.056).
- **Per model (48 more tests, 8 with p < 0.05, none below 0.003).** The one consistent pattern is
  **kimi27sglang at T72: its C beats its Fortran on every pair where they differ by more than 10%.**
  best 1.45x (1.05-1.99, p = 0.016, 8 C better / 0 Fortran better / 14 within 10%), median 1.74x
  (p = 0.008). The identical-binary subset agrees (best 1.84x, p = 0.011, n = 13). At T1 the same
  model shows no C advantage (best not significant). Under GCC 14.2, its FIRST submissions even lean
  Fortran (0.90, p = 0.038). qwen38's median favours C at T1 (1.12, p = 0.036; GCC 14.2: 1.17,
  p = 0.025). oss120b shows nothing at any setting. With 64 tests in all, a few p < 0.05 are expected
  by chance. Only the kimi27sglang T72 effect is consistent across statistics.
- **Portability is significant and large:** Fortran validates 96% (196/205) against C 79%
  (246/313). The C losses are mostly x86 intrinsics (55); there are 8 incorrect outputs and 1 timeout
  at the 1800 s per-cell limit (`tsvc_2_s2710`, qwen38-c, with both compilers). Fortran loses 3
  to build errors (gfortran rejecting the source: nested work-sharing regions, IMPORT outside an
  interface) and 6 to incorrect outputs.
- **Campaign speedups transfer in ranking.** For every arm, the Spearman correlation between the
  MI300A campaign speedup and the GH200 speedup over translated C is positive, and significant
  except kimi27sglang-fortran at T1. It is stronger at T72 (rho 0.41-0.88) than at T1 (0.12-0.66):
  the campaign was multi-core.

## Notes

- One T72 cell (`tsvc_2_s1232`, kimi27sglang-c, seq 4) is recorded as `build_error` with no
  timings. Its harness log shows a validated run (median 0.933 ms), so the runner misclassified it
  (DEVIATIONS 12). It is excluded from the T72 statistics.
- Translated C validates on 38 of 39 kernels. `tsvc_2_s115` is `incorrect` there, as in the v2
  matrix (FMA contraction).
