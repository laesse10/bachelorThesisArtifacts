# solver14 at the 120M-token / 20 h budget (snapshot 2026-10-13)

Second solver14 report, after PR #51 raised every setup's budget from 10M tokens / 10 h to 120M tokens / 20 h.
Setting as before: C, CPU (MI300A node), one agent per kernel, the 14 `solvers` kernels, temperature 1.0, each
model with no skill packet and with the `solver` packet (`+solver`). Code: hb `budget-120m-20h` (daa13ed7f).

**This is a snapshot.** Two arms are still running and four reruns of owed kernels are queued or starting (see
*Owed kernels*). Numbers for `qwen38` and `glm53 +solver` will change; the oss120b arms and glm53's first job
are final.

## Databases

- `solver14-120m.db`: everything above merged with `hpcagent-bench aggregate-db`: every `/score`, `/submit`,
  promoted score and final re-timing (`grades.kind`), plus episode records.
  - finished jobs: `677831/results.db` (oss120b), `677832/results.db` (oss120b +solver),
    `677837/results.db` (glm53);
  - jobs without a merged DB, read from SQLite backups of their judge shards (`<job>/judge/rank-*/`):
    677833 (qwen38, running), 677836 (qwen38 +solver, node failure, `MERGE_FAILED`), 677838 (glm53 +solver,
    running). These shards hold no driver episode records, so their tokens and agent wall time come from the
    agents' `tokens.json`. 677836 left no `tokens.json`, so its tokens and wall time are n/a.
- Per-job DBs stay in `../<job>/`. The 10M baselines are `../../solver14-20261010/solver14-20261010.db` (no packet)
  and `../677209/results.db`, `../677210/results.db` (`+solver`, oss120b and qwen38).
- `owed/<setup>.txt`: the runs each setup still owes (`hpcagent-bench owed collect --tag solvers`).
- `submitted/<setup>/`: every `/submit` source (`<kernel>.submit.callNN.host.c`) and the source each final
  re-timed (`<kernel>.final.<credited>x.host.c`, a `/submit` or a promoted score).
- `scored/<setup>/`: every `/score` source, `<kernel>.callNN.<correct|incorrect>.host.c`.
- `csv/`: the DB's `setups`, `episodes` and `grades` tables.

| setup | episodes | finals (correct) | geomean credited | mean credited | /score calls (correct) | submits + promoted | tokens | agent wall |
|---|---|---|---|---|---|---|---|---|
| oss120b | 14 | 12 (12) | 2.37x | 7.38x | 53 (32) | 8 + 4 | 30.8 M | 6.4 h |
| oss120b +solver | 14 | 11 (11) | 2.04x | 5.32x | 60 (40) | 8 + 4 | 27.2 M | 6.5 h |
| qwen38 (running) | 12 | 3 (3) | 24.42x | 114.08x | 44 (37) | 3 + 0 | 37.2 M | 18.6 h |
| qwen38 +solver | 7 | 0 (0) | n/a | n/a | 14 (11) | 0 + 0 | n/a | n/a |
| glm53 | 10 | 2 (2) | 1.26x | 1.43x | 23 (17) | 0 + 2 | 8.2 M | 8.3 h |
| glm53 +solver (running) | 13 | 2 (2) | 65.48x | 204.37x | 30 (24) | 2 + 1 | 31.7 M | 22.6 h |

## Owed kernels

A kernel is owed when no job graded a `/submit` or promoted score for it (`hpcagent_bench.owed`). All 52 owed
runs are class `infra` (none `budget`): no agent hit the 120M/20 h cap.

| setup | owed | why | rerun |
|---|---|---|---|
| glm53 | 12: amg_setup, bdf_newton_krylov, householder_qr, jfnk_bratu, lanczos_reorth, mg_vcycle, mixed_precision_ir, rb_sor, rk45_ensemble, rk4_ensemble, sgs_pcg, sptrsv_level | **glm53 crash.** Job 677837 died after 3 h 15 m of serving. The GDR-registration crash fix held. SGLang's pipeline-parallel scheduler (tp4 x pp4 over 4 nodes) hung on a stage-to-stage receive (`recv_object`, gloo "connection closed by peer") with the KV cache full (7.5k of 1.8M tokens free). sglang SIGQUIT the service and the job tore the agents down. 10 episodes had started: 2 finals, 6 more had correct scores. | 678947 (queued 10-13) |
| qwen38 +solver | all 14 | **Node failure.** Job 677836 ended `NODE_FAIL` after 3 h 28 m. 7 episodes had started, 11 correct scores, nothing submitted or promoted. | 678946 (queued 10-13) |
| oss120b | amg_setup, bdf_newton_krylov | The agent ended on its own (rc 0) with nothing to credit. amg_setup: 7 `/score` calls, all incorrect. bdf_newton_krylov: 135 turns and 6.9M tokens, no graded call. | 678949 (queued 10-13) |
| oss120b +solver | lanczos_reorth, rk45_ensemble | The agent ended on its own (rc 0) within minutes (8 and 21 turns), no graded call. | 678950 (queued 10-13) |
| qwen38 | 11 | Still running (job 677833); owed until its episodes finish. | n/a |
| glm53 +solver | 11 | Still running (job 677838); owed until its episodes finish. | n/a |

## Per kernel: credited final speedup
: credited final speedup

| kernel | oss120b | oss120b +solver | qwen38 | qwen38 +solver | glm53 | glm53 +solver |
|---|---|---|---|---|---|---|
| amg_setup | no final | 1.14x | no final (best score 11.04x) | not started | no final (best score 9.66x) | no final (best score 9.50x) |
| bdf_newton_krylov | no final | 0.33x | no final (best score 1.01x) | not started | no final (best score 1.08x) | no final (best score 1.22x) |
| householder_qr | 0.08x | 1.36x | no final (best score 6.93x) | no final (best score 7.54x) | not started | no final (best score 7.50x) |
| ilu0 | 1.02x | 1.06x | 4.54x | not started | 0.76x | no final (best score 1.00x) |
| jfnk_bratu | 9.68x | no final (best score 0.88x) | no final (best score 0.50x) | no final (best score 8.01x) | no final | no final (best score 12.17x) |
| lanczos_reorth | 0.78x | no final | not started | no final (best score 0.90x) | not started | no final (best score 2.26x) |
| mg_vcycle | 1.12x | 1.67x | not started | not started | not started | no final (best score 2.78x) |
| mixed_precision_ir | 3.74x | 3.93x | no final (best score 1.93x) | not started | no final | no final (best score 6.03x) |
| rb_sor | 6.73x | 6.86x | 9.79x | no final (best score 8.19x) | no final (best score 12.81x) | 10.77x |
| rk45_ensemble | 21.39x | no final | no final (best score 27.55x) | no final (best score 29.63x) | no final (best score 27.62x) | no final (best score 23.24x) |
| rk4_ensemble | 40.07x | 37.95x | 327.92x | no final (best score 319.88x) | no final (best score 273.42x) | 397.96x |
| sgs_pcg | 1.64x | 1.47x | no final (best score 1.36x) | no final (best score 0.97x) | not started | not started |
| sparse_cholesky | 1.01x | 1.41x | no final (best score 0.99x) | not started | 2.11x | no final (best score 0.32x) |
| sptrsv_level | 1.35x | 1.32x | no final (best score 0.08x) | not started | no final (best score 0.75x) | no final (best score 1.35x) |

## Improvement over the 10M / 10h campaign

Common kernels = kernels with a correct credited final in both campaigns; ratio = geomean(120M / 10M) over them.

| setup | 10M finals | 10M geomean | 120M finals | 120M geomean | common kernels | per-kernel ratio |
|---|---|---|---|---|---|---|
| oss120b | 12 | 3.20x | 12 | 2.37x | 12 | 0.74x |
| oss120b +solver | 11 | 2.03x | 11 | 2.04x | 9 | 1.09x |
| qwen38 | 9 | 6.75x | 3 | 24.42x | 2 | 1.41x |
| qwen38 +solver | 7 | 6.37x | 0 | n/a | 0 | n/a |
| glm53 | 0 | n/a | 2 | 1.26x | 0 | n/a |

Findings:

- **oss120b (no packet) did not gain from the larger budget**: 12 finals either way, geomean 3.20x -> 2.37x, and
  0.74x per kernel on the 12 it finished in both. oss120b stops long before either cap (6.4 h, 30.8M tokens over
  14 agents), so the budget is not what limits it and the difference is sampling variance. The largest drop is
  householder_qr: a promoted score at 0.08x against a 2.13x `/submit` at 10M.
- **oss120b +solver**: unchanged (2.03x -> 2.04x, 1.09x per kernel on 9 common kernels). At 120M the packet beats
  no packet by 1.48x per kernel on their 9 common kernels. Most of that is householder_qr (1.36x vs 0.08x).
  Without it, no packet is ahead by a geomean of 2.37x vs 2.04x.
- **qwen38 is where the budget matters**: at 10M, 13 of 14 episodes hit the token cap. At 120M its finished
  kernels submit after 8-15M tokens. 3 finals so far (ilu0 4.54x vs 2.27x, rb_sor 9.79x, rk4_ensemble 327.9x vs 330.2x).
  That is 1.41x per kernel on the 2 common kernels; the arm is still running.
- **glm53 now produces finals**: 0 at 10M (the job crashed), 2 so far without the packet. With the packet it has
  rb_sor 10.77x and rk4_ensemble 397.96x, and the arm is still running.
- None of the finals carries a timing-audit verdict (`suspect` is NULL). The rk4_ensemble finals of 328-398x need
  the same check the 10M report flagged for qwen38's 330x.

## Final submissions

Each final re-times one graded source: a `/submit`, or the best correct `/score` promoted when the episode
ended without one. A promoted score has no token count recorded at its call (0.0 M).

| setup | kernel | origin | call # | tokens at call | credited | correct | source |
|---|---|---|---|---|---|---|---|
| oss120b | householder_qr | promoted | 4 | 0.0 M | 0.08x | yes | [`submitted/oss120b-c/householder_qr.final.0.08x.host.c`](submitted/oss120b-c/householder_qr.final.0.08x.host.c) |
| oss120b | ilu0 | promoted | 8 | 0.0 M | 1.02x | yes | [`submitted/oss120b-c/ilu0.final.1.02x.host.c`](submitted/oss120b-c/ilu0.final.1.02x.host.c) |
| oss120b | jfnk_bratu | submit | 8 | 2.2 M | 9.68x | yes | [`submitted/oss120b-c/jfnk_bratu.final.9.68x.host.c`](submitted/oss120b-c/jfnk_bratu.final.9.68x.host.c) |
| oss120b | lanczos_reorth | submit | 6 | 0.5 M | 0.78x | yes | [`submitted/oss120b-c/lanczos_reorth.final.0.78x.host.c`](submitted/oss120b-c/lanczos_reorth.final.0.78x.host.c) |
| oss120b | mg_vcycle | promoted | 4 | 0.0 M | 1.12x | yes | [`submitted/oss120b-c/mg_vcycle.final.1.12x.host.c`](submitted/oss120b-c/mg_vcycle.final.1.12x.host.c) |
| oss120b | mixed_precision_ir | submit | 5 | 1.1 M | 3.74x | yes | [`submitted/oss120b-c/mixed_precision_ir.final.3.74x.host.c`](submitted/oss120b-c/mixed_precision_ir.final.3.74x.host.c) |
| oss120b | rb_sor | submit | 3 | 0.3 M | 6.73x | yes | [`submitted/oss120b-c/rb_sor.final.6.73x.host.c`](submitted/oss120b-c/rb_sor.final.6.73x.host.c) |
| oss120b | rk45_ensemble | submit | 2 | 0.5 M | 21.39x | yes | [`submitted/oss120b-c/rk45_ensemble.final.21.39x.host.c`](submitted/oss120b-c/rk45_ensemble.final.21.39x.host.c) |
| oss120b | rk4_ensemble | submit | 2 | 0.1 M | 40.07x | yes | [`submitted/oss120b-c/rk4_ensemble.final.40.07x.host.c`](submitted/oss120b-c/rk4_ensemble.final.40.07x.host.c) |
| oss120b | sgs_pcg | promoted | 7 | 0.0 M | 1.64x | yes | [`submitted/oss120b-c/sgs_pcg.final.1.64x.host.c`](submitted/oss120b-c/sgs_pcg.final.1.64x.host.c) |
| oss120b | sparse_cholesky | submit | 5 | 2.0 M | 1.01x | yes | [`submitted/oss120b-c/sparse_cholesky.final.1.01x.host.c`](submitted/oss120b-c/sparse_cholesky.final.1.01x.host.c) |
| oss120b | sptrsv_level | submit | 4 | 0.8 M | 1.35x | yes | [`submitted/oss120b-c/sptrsv_level.final.1.35x.host.c`](submitted/oss120b-c/sptrsv_level.final.1.35x.host.c) |
| oss120b +solver | amg_setup | submit | 3 | 1.1 M | 1.14x | yes | [`submitted/oss120b-c-solver/amg_setup.final.1.14x.host.c`](submitted/oss120b-c-solver/amg_setup.final.1.14x.host.c) |
| oss120b +solver | bdf_newton_krylov | promoted | 5 | 0.0 M | 0.33x | yes | [`submitted/oss120b-c-solver/bdf_newton_krylov.final.0.33x.host.c`](submitted/oss120b-c-solver/bdf_newton_krylov.final.0.33x.host.c) |
| oss120b +solver | householder_qr | promoted | 12 | 0.0 M | 1.36x | yes | [`submitted/oss120b-c-solver/householder_qr.final.1.36x.host.c`](submitted/oss120b-c-solver/householder_qr.final.1.36x.host.c) |
| oss120b +solver | ilu0 | submit | 8 | 0.8 M | 1.06x | yes | [`submitted/oss120b-c-solver/ilu0.final.1.06x.host.c`](submitted/oss120b-c-solver/ilu0.final.1.06x.host.c) |
| oss120b +solver | mg_vcycle | promoted | 15 | 0.0 M | 1.67x | yes | [`submitted/oss120b-c-solver/mg_vcycle.final.1.67x.host.c`](submitted/oss120b-c-solver/mg_vcycle.final.1.67x.host.c) |
| oss120b +solver | mixed_precision_ir | submit | 3 | 1.1 M | 3.93x | yes | [`submitted/oss120b-c-solver/mixed_precision_ir.final.3.93x.host.c`](submitted/oss120b-c-solver/mixed_precision_ir.final.3.93x.host.c) |
| oss120b +solver | rb_sor | submit | 5 | 1.9 M | 6.86x | yes | [`submitted/oss120b-c-solver/rb_sor.final.6.86x.host.c`](submitted/oss120b-c-solver/rb_sor.final.6.86x.host.c) |
| oss120b +solver | rk4_ensemble | submit | 3 | 0.3 M | 37.95x | yes | [`submitted/oss120b-c-solver/rk4_ensemble.final.37.95x.host.c`](submitted/oss120b-c-solver/rk4_ensemble.final.37.95x.host.c) |
| oss120b +solver | sgs_pcg | submit | 5 | 0.7 M | 1.47x | yes | [`submitted/oss120b-c-solver/sgs_pcg.final.1.47x.host.c`](submitted/oss120b-c-solver/sgs_pcg.final.1.47x.host.c) |
| oss120b +solver | sparse_cholesky | submit | 4 | 1.1 M | 1.41x | yes | [`submitted/oss120b-c-solver/sparse_cholesky.final.1.41x.host.c`](submitted/oss120b-c-solver/sparse_cholesky.final.1.41x.host.c) |
| oss120b +solver | sptrsv_level | promoted | 3 | 0.0 M | 1.32x | yes | [`submitted/oss120b-c-solver/sptrsv_level.final.1.32x.host.c`](submitted/oss120b-c-solver/sptrsv_level.final.1.32x.host.c) |
| qwen38 | ilu0 | submit | 12 | 14.2 M | 4.54x | yes | [`submitted/qwen38-c/ilu0.final.4.54x.host.c`](submitted/qwen38-c/ilu0.final.4.54x.host.c) |
| qwen38 | rb_sor | submit | 7 | 15.1 M | 9.79x | yes | [`submitted/qwen38-c/rb_sor.final.9.79x.host.c`](submitted/qwen38-c/rb_sor.final.9.79x.host.c) |
| qwen38 | rk4_ensemble | submit | 6 | 7.9 M | 327.92x | yes | [`submitted/qwen38-c/rk4_ensemble.final.327.92x.host.c`](submitted/qwen38-c/rk4_ensemble.final.327.92x.host.c) |
| glm53 | ilu0 | promoted | 2 | 0.0 M | 0.76x | yes | [`submitted/glm53-c/ilu0.final.0.76x.host.c`](submitted/glm53-c/ilu0.final.0.76x.host.c) |
| glm53 | sparse_cholesky | promoted | 3 | 0.0 M | 2.11x | yes | [`submitted/glm53-c/sparse_cholesky.final.2.11x.host.c`](submitted/glm53-c/sparse_cholesky.final.2.11x.host.c) |
| glm53 +solver | rb_sor | submit | 4 | 9.8 M | 10.77x | yes | [`submitted/glm53-c-solver/rb_sor.final.10.77x.host.c`](submitted/glm53-c-solver/rb_sor.final.10.77x.host.c) |
| glm53 +solver | rk4_ensemble | submit | 2 | 13.8 M | 397.96x | yes | [`submitted/glm53-c-solver/rk4_ensemble.final.397.96x.host.c`](submitted/glm53-c-solver/rk4_ensemble.final.397.96x.host.c) |
