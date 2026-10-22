# solver14-precond results (HPCAgent-Bench v0.2, tag 083c88e)

Snapshot: 2026-10-19T21:47:21+02:00 (687301 live). Experiment `solver14-precond` (record study `solver14`, tag `solvers`), language C, 14 solver kernels, one episode per kernel per setup, models qwen38 and oss120b, skill packets none (baseline), preconditioner-v4-single (one 565-line page) and preconditioner-v4-split (five pages), from `/capstor/scratch/cscs/ybudanaz/x86_64/external-skills/solver-v4`.

| model | variant | job(s) | state |
|---|---|---|---|
| qwen38 | none | 686052 | COMPLETED |
| qwen38 | v4-single | 686054 | COMPLETED |
| qwen38 | v4-split | 686055+687301 | 686055 NODE_FAIL after 6h36m (3 answers recorded); rerun 687301 RUNNING (snapshot: 1 of 14 episodes finished, no answer submitted yet) |
| oss120b | none | 686049 | COMPLETED |
| oss120b | v4-single | 686050 | COMPLETED |
| oss120b | v4-split | 686051 | COMPLETED |

**qwen38 / v4-split is PARTIAL.** It holds 686055's three recorded answers (ilu0, rk4_ensemble, sparse_cholesky; 687301 is re-running those kernels but has not answered them yet), 687301's finished householder_qr episode, and ten kernels still running in 687301. Its row is not comparable to the others until 687301 finishes.

## How the numbers are defined

- **Speedup** is the bench's credited task score S_i, read with the bench's own tooling: `hpcagent-bench extract` over read-only copies of the job results DBs, then `hpcagent_bench.stats.population.kernel_answers` (the rule every bench figure uses). The judge times the final answer under the final protocol `mw4x5`: 4 large inputs, 5 runs a side, one untimed warmup. Per input the ratio is median(reference time) / median(submission time), and it counts only when a one-sided Mann-Whitney test clears alpha = 0.1 (otherwise 1.0). S_i is the geometric mean over the 4 inputs.
- **Denominator.** The judge's reference is `best-of(numba,c)`: per input it uses the faster of the Numba-compiled and the C reference, not the C reference alone. Column `denominator_cells` in speedup.csv says which one won each input (e.g. `cx4`, `numbax4`, `cx2+numbax2`). A speedup against the C reference alone cannot be recovered from the DBs when Numba won, because the race stops timing the slower reference early.
- **Final answer** is the episode's last `/submit`. When the agent never submitted (it timed out or ended), the judge promotes its best correct `/score` source and grades that (`final_kind = promoted`).
- **Solved** means the final grade is credited (correct on all 4 inputs and the held-out cases, no sanitizer rejection) and the judge did not flag it suspect. Not solved scores S_i = 1.0, the bench's convention (`population.NOT_DELIVERED`):
  - `asan`: the answer was correct, but the AddressSanitizer gate rejected it. These are the 7 accepted sanitizer disqualifications. The raw (uncredited) speedup is shown in parentheses.
  - `ubsan`: credited but flagged `suspect` because of UndefinedBehaviorSanitizer findings. The bench then scores it 1.0 and treats the kernel as unanswered. This affects two qwen38/v4-single answers: rk45_ensemble (raw 60.67, misaligned `__m512d` store) and sparse_cholesky (raw 1.47, null pointer passed to a nonnull argument). See the alternative reading below.
  - `overfit`: `/submit` refused by the size-class sweep gate.
  - `no-answer`: no correct source to grade (reason per kernel below).
- **Geomean (solved)** is taken over the solved kernels only. **Geomean (all 14)** counts every unsolved or still-running kernel as 1.0. **Median (all 14)** uses the same convention.
- **Wins vs none**: per kernel, the variant's S_i compared with the same model's `none` S_i (strictly greater = win, equal = tie). Only decided kernels are compared.
- **Tokens** come from each episode's `tokens.json`, the bench's token fold (docs/token_accounting.md). `effective` = fresh input + output. `billed` card = fresh + 0.1 x cached + output. USD uses the bench's list-price cards `usd-qwen38` (0.50/0.10/3.00 per 1M fresh/cached/output) and `usd-oss120b` (0.15/0.15/0.60). Wall time is the episode's wall clock.
- **Tool calls** are counted from each episode transcript (`claude.log`, stream-json): every distinct `tool_use` id, by tool name. The harness offers Bash, Read, Edit and the MCP tools score, submit, profile and syntax_check. It has no search tool. `other` counts calls to tools that do not exist (oss120b sometimes calls `Search`/`Write`, which fail). **Skill reads** are tool calls whose input names a page under `/shared/skills/`.

## Summary per model and variant

| model | variant | solved / 14 | geomean (solved) | geomean (all 14, fail = 1.0) | median (solved) | median (all 14) | wins / ties / losses vs none | unsolved breakdown |
|---|---|---|---|---|---|---|---|---|
| qwen38 | none | 10 | 6.79 | 3.93 | 5.10 | 2.25 |  | 3 asan, 1 overfit |
| qwen38 | v4-single | 9 | 6.64 | 3.38 | 7.69 | 1.47 | 8 / 1 / 5 | 2 asan, 2 ubsan, 1 no-answer |
| qwen38 | v4-split (partial) | 3 | 11.31 | 1.68 | 4.12 | 1.00 | 1 / 0 / 3 | 1 no-answer, 10 running |
| oss120b | none | 13 | 2.42 | 2.27 | 1.37 | 1.34 |  | 1 asan |
| oss120b | v4-single | 12 | 1.74 | 1.61 | 1.31 | 1.16 | 5 / 1 / 8 | 2 no-answer |
| oss120b | v4-split | 12 | 2.43 | 2.14 | 1.42 | 1.08 | 6 / 0 / 8 | 1 asan, 1 no-answer |

If the two UBSan-flagged qwen38/v4-single answers were credited at their raw final ratio, that setup would have 11 solved kernels. Geomean (solved) would be 7.08 and geomean (all 14) 4.65. This is not the bench's rule.

## Per-kernel speedup (S_i, credited final grade vs best-of(numba,c))

Bold = solved. `1.00 <tag>` = not solved, scored 1.0 (raw ratio in parentheses where one was measured). `[686055]` / `[687301]` = the job the qwen38/v4-split value comes from.

| kernel | qwen38 none | qwen38 v4-single | qwen38 v4-split | oss120b none | oss120b v4-single | oss120b v4-split |
|---|---|---|---|---|---|---|
| amg_setup | **9.15** | **11.18** | running | **1.32** | 1.00 no-answer | 1.00 no-answer |
| bdf_newton_krylov | **1.30** | **1.00** | running | 1.00 asan (raw 0.52) | 1.00 no-answer | **0.51** |
| householder_qr | **12.88** | 1.00 no-answer | 1.00 no-answer [687301] | **0.45** | **0.42** | **1.73** |
| ilu0 | **0.28** | 1.00 asan (raw 4.81) | **4.12** [686055] | **0.99** | **1.02** | **1.04** |
| jfnk_bratu | 1.00 overfit | **28.87** | running | **5.58** | **7.57** | **9.60** |
| lanczos_reorth | **2.70** | **9.48** | running | **1.02** | **0.13** | **1.12** |
| mg_vcycle | 1.00 asan (raw 2.78) | 1.00 asan (raw 2.35) | running | **0.64** | **0.39** | **0.64** |
| mixed_precision_ir | **1.80** | **2.57** | running | **5.14** | **1.62** | **2.23** |
| rb_sor | **6.47** | **7.69** | running | **6.79** | **6.76** | **6.40** |
| rk45_ensemble | **155.34** | 1.00 ubsan (raw 60.67) | running | **21.20** | **23.09** | **20.98** |
| rk4_ensemble | **264.09** | **208.99** | **232.34** [686055] | **22.13** | **30.71** | **23.41** |
| sgs_pcg | 1.00 asan (raw 1.46) | **1.05** | running | **1.57** | **1.29** | 1.00 asan (raw 1.34) |
| sparse_cholesky | **3.74** | 1.00 ubsan (raw 1.47) | **1.51** [686055] | **1.37** | **0.36** | **1.01** |
| sptrsv_level | 1.00 asan (raw 2.48) | **1.89** | running | **1.29** | **1.32** | **0.94** |

## Unsolved and partial episodes

| model | variant | kernel | job | outcome | reason | best /score preview (mw2x5) |
|---|---|---|---|---|---|---|
| qwen38 | none | jfnk_bratu | 686052 | overfit | rejected at /submit: size_class_sweep: overfit | 27.45 |
| qwen38 | none | sgs_pcg | 686052 | asan | ASan disqualified (correct output): AddressSanitizer: SEGV on unknown address 0x000000000000 (pc 0x154452f | 1.48 |
| qwen38 | none | sptrsv_level | 686052 | asan | ASan disqualified (correct output): AddressSanitizer: SEGV on unknown address 0x107c0fb5075b (pc 0x107c0fb | 2.56 |
| qwen38 | none | mg_vcycle | 686052 | asan | ASan disqualified (correct output): AddressSanitizer: heap-buffer-overflow on address 0x1067c9fa27f8 at pc | 2.37 |
| qwen38 | v4-single | householder_qr | 686054 | nofinal | no answer: model output cap (32768 tokens) ended the episode |  |
| qwen38 | v4-single | rk45_ensemble | 686054 | suspect | correct; judge flagged suspect (sanitizers: undefined behaviour: runtime error: store to misaligned address 0x10dd2d3f0320 for type '__m512d',) -> bench S=1.0 | 46.52 |
| qwen38 | v4-single | ilu0 | 686054 | asan | ASan disqualified (correct output): AddressSanitizer: SEGV on unknown address (pc 0x1484df24c384 bp 0x7ffd | 5.00 |
| qwen38 | v4-single | sparse_cholesky | 686054 | suspect | correct; judge flagged suspect (sanitizers: undefined behaviour: runtime error: null pointer passed as argument 1, which is declared to never ) -> bench S=1.0 | 1.77 |
| qwen38 | v4-single | mg_vcycle | 686054 | asan | ASan disqualified (correct output): AddressSanitizer: heap-buffer-overflow on address 0x10f89cd6f7f8 at pc | 1.73 |
| qwen38 | v4-split | householder_qr | 687301 | nofinal | no answer: model output cap (32768 tokens) ended the episode |  |
| qwen38 | v4-split | mixed_precision_ir | 687301 | running | running (no answer yet) | 0.48 |
| qwen38 | v4-split | rk45_ensemble | 687301 | running | running (no answer yet) | 24.44 |
| qwen38 | v4-split | amg_setup | 687301 | running | running (no answer yet) |  |
| qwen38 | v4-split | jfnk_bratu | 687301 | running | running (no answer yet) |  |
| qwen38 | v4-split | lanczos_reorth | 687301 | running | running (no answer yet) |  |
| qwen38 | v4-split | sgs_pcg | 687301 | running | running (no answer yet) |  |
| qwen38 | v4-split | sptrsv_level | 687301 | running | running (no answer yet) |  |
| qwen38 | v4-split | bdf_newton_krylov | 687301 | running | running (no answer yet) |  |
| qwen38 | v4-split | mg_vcycle | 687301 | running | running (no answer yet) | 0.80 |
| qwen38 | v4-split | rb_sor | 687301 | running | running (no answer yet) |  |
| oss120b | none | bdf_newton_krylov | 686049 | asan | ASan disqualified (correct output): AddressSanitizer: heap-buffer-overflow on address 0x1116384e5df8 at pc | 0.44 |
| oss120b | v4-single | amg_setup | 686050 | nofinal | no answer: agent ended with no correct /score (1 score calls, none correct) |  |
| oss120b | v4-single | bdf_newton_krylov | 686050 | nofinal | no answer: agent ended with no correct /score (4 score calls, none correct) |  |
| oss120b | v4-split | amg_setup | 686051 | nofinal | no answer: malformed tool call (API 400 JSON parse) ended the episode |  |
| oss120b | v4-split | sgs_pcg | 686051 | asan | ASan disqualified (correct output): AddressSanitizer: attempting free on address which was not malloc()-ed | 1.41 |

Accepted outcomes, per the user's decisions:

- 686051 amg_setup (oss120b, v4-split): a real failure. A malformed tool call (API 400, JSON parse) ended the episode.
- 686054 householder_qr (qwen38, v4-single): a real failure. The episode hit the 32768-token output cap. 687301 householder_qr (qwen38, v4-split) ended the same way.
- The 7 `asan` rows are correct answers that the ASan gate rejected.
- 686050 amg_setup and bdf_newton_krylov (oss120b, v4-single): no correct /score. This is the agent's result, not an infrastructure fault.

## Cost and time per episode

Episodes with a finished `tokens.json`; qwen38/v4-split covers the 4 finished episodes only (3 from 686055, 1 from 687301).

| model | variant | episodes | effective tokens median | effective mean | billed-card median | billed-card total | USD mean / episode | USD total | wall min median | wall min mean | turns median |
|---|---|---|---|---|---|---|---|---|---|---|---|
| qwen38 | none | 14 | 1.47M | 1.31M | 4.32M | 57.17M | 4.86 | 68.07 | 1090.0 | 945.1 | 282 |
| qwen38 | v4-single | 14 | 1.05M | 1.10M | 3.55M | 48.72M | 4.12 | 57.70 | 754.0 | 783.1 | 238 |
| qwen38 | v4-split | 4 | 0.40M | 0.41M | 1.14M | 4.92M | 1.51 | 6.03 | 238.3 | 243.3 | 84 |
| oss120b | none | 14 | 70k | 83k | 0.16M | 3.73M | 0.30 | 4.22 | 26.1 | 28.3 | 38 |
| oss120b | v4-single | 14 | 93k | 0.13M | 0.26M | 5.60M | 0.44 | 6.14 | 32.4 | 38.5 | 40 |
| oss120b | v4-split | 14 | 69k | 78k | 0.16M | 3.39M | 0.27 | 3.77 | 23.4 | 25.2 | 31 |

qwen38 episodes run for hours: 6 of 14 (none) and 2 of 14 (v4-single) hit the 20 h episode wall-clock limit (`episode_result` = timeout; the judge then promoted the best correct /score). The longest ran 1201 min. No oss120b episode timed out. They took 3.5 to 170 min, with a median of 23 to 32 min.

## Tool calls (totals over the 14 episodes)

For qwen38/v4-split the counts come from the reported episode of each kernel: 686055 for the 3 answered kernels, 687301 for the rest, including the ten running 687301 transcripts as read at the snapshot.

| model | variant | score | submit | profile | syntax_check | Bash | Read | Edit | other (invalid) | total | per episode | episodes reading a skill page | skill-page reads |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| qwen38 | none | 80 | 8 | 56 | 30 | 3302 | 199 | 247 | 0 | 3922 | 280.1 | 0 / 14 | 0 |
| qwen38 | v4-single | 79 | 11 | 73 | 29 | 2796 | 168 | 170 | 2 | 3328 | 237.7 | 14 / 14 | 17 |
| qwen38 | v4-split | 21 | 3 | 19 | 3 | 845 | 123 | 22 | 0 | 1036 | 74.0 | 14 / 14 | 69 |
| oss120b | none | 56 | 10 | 1 | 52 | 269 | 136 | 106 | 13 | 643 | 45.9 | 0 / 14 | 0 |
| oss120b | v4-single | 55 | 7 | 5 | 64 | 325 | 187 | 180 | 9 | 832 | 59.4 | 10 / 14 | 14 |
| oss120b | v4-split | 56 | 11 | 3 | 53 | 194 | 159 | 99 | 4 | 579 | 41.4 | 8 / 14 | 37 |

Per-episode counts by tool, tokens, wall time and status are in speedup.csv (`calls_*`, `tokens_*`, `wall_min`, `turns`, `skill_pages_read`).

## Data gaps

- **687301 is still running.** 10 of 14 qwen38/v4-split episodes are unfinished, and householder_qr finished without an answer (output cap). 687301 had no merged results.db at the snapshot (`MERGE_FAILED` marker: not merged yet). Its judge shards rank-0/1/3 were copied as they stood, and rank-2 was empty. Transcripts, workspaces and shards in the zip are a snapshot of a live run.
- **686055 (NODE_FAIL)** never merged its results.db (`MERGE_FAILED`). Its 4 judge shards hold the 3 recorded answers. Its 11 unfinished episodes have transcripts but no `tokens.json`. They are in the zip under `<kernel>/job-686055/` but in no table, because the tables use 687301 for those kernels. Both jobs' episodes are packaged for every kernel. OUTCOME.json says which one the tables report (`reported`).
- **No C-only speedup.** The credited denominator is best-of(numba,c) (see definitions).
- **Workspaces** in the zip keep text files of 2 MB or less. Binary inputs and outputs (npz/npy/bin/dat/.o/.so, core files, up to 1.3 GB each) were left out and are listed in each `workspace/SKIPPED.tsv`.

## Package layout (solver14-precond.zip)

```
solver14-precond/
  README.md  report.md  report.html  speedup.csv  summary.csv
  observations/          bench extract output: observations.db + .csv, sources_index.csv
  skills/solver-v4/      the shipped skill pages (single, split) + skills.yaml
  jobs/<job>/            results.db (merged, if any), judge/rank-*/*.db shards + upstream logs, services-<job>.out/.err,
                         prepare_job.sh, inference.json, agent-folders.jsonl, problems.jsonl, monitor/, shared prompt+skills, tasks/
  <model>/<variant>/<kernel>/job-<job>/
      OUTCOME.json       outcome, reason, final and best-score grade summaries
      final/             source of the final answer (submit or promoted grade) + final_grade.json
      best_scored/       best correct /score source, only if it differs from the final one
      graded/            every source the judge graded, in call order (NN_<kind>_<status>_<speedup>.c)
      judge_grades.csv   every judge request of the episode (score/submit/promoted/final), with detail
      judge_cells.csv    per-input cells of those grades (reference that won, times, ratio, p-value)
      episode/           claude.log (full stream-json transcript), prompt.txt, tokens.json, attempts.jsonl,
                         mcp.json, submission-spent.json, home/ (Claude Code session files)
      workspace/         the agent's /shared/agent-N text files; SKIPPED.tsv lists binaries left out
```

