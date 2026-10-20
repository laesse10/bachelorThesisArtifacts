# solver14 campaign, 2026-10-10 (hb c429b45f5)

DB: `solver14-20261010.db` merges the three job DBs: `675848/` (qwen38), `675849/` (oss120b) and `675850/` (glm53).
It keeps the full history: every `/score` call, every `/submit`, every score promoted at the token cap, and
every final re-timing (`grades.kind`). Setting: C, CPU, one agent per kernel, 14 solver kernels, temperature 1.0.

| setup | episodes | finals (correct) | geomean credited | mean credited | /score calls (correct) | submits + promoted | tokens | agent wall |
|---|---|---|---|---|---|---|---|---|
| oss120b | 14 | 12 (12) | 3.20x | 7.74x | 56 (42) | 10 + 3 | 35.9 M | 6.0 h |
| qwen38 | 14 | 9 (9) | 6.75x | 42.4x | 36 (30) | 1 + 9 | 132.1 M | 76.2 h |
| glm53 | 9 started | 0 | n/a | n/a | 10 (8) | 0 | 8.3 M | 5.2 h |

## Per kernel: credited final speedup

| kernel | oss120b | qwen38 |
|---|---|---|
| amg_setup | no final (3 scores, 0 correct) | no final (1 correct score, 7.9x) |
| bdf_newton_krylov | no final (best score 0.55x) | none |
| householder_qr | 2.13x | 5.68x |
| ilu0 | 1.03x | 2.27x |
| jfnk_bratu | 10.0x | 10.3x |
| lanczos_reorth | 0.78x | 3.21x |
| mg_vcycle | 1.14x | none |
| mixed_precision_ir | 5.35x | 2.08x |
| rb_sor | 6.83x | none |
| rk45_ensemble | 23.6x | 23.2x |
| rk4_ensemble | 38.3x | 330x (baseline 10.6 s, native 32 ms; not flagged suspect) |
| sgs_pcg | 1.37x | 2.80x |
| sparse_cholesky | 1.01x | none |
| sptrsv_level | 1.32x | 1.53x |

## Findings

- **oss120b**: 12/14 finals, all correct. It ends early (6 h of agent time) and spends few tokens.
- **qwen38**: 13 of 14 episodes hit the 10 M token cap (rc 125). Their best correct `/score` was promoted
  to a final, so 9 have a final, all correct. Its mean is driven by rk4_ensemble at 330x; its geomean
  beats oss120b's (6.75x vs 3.20x).
- **glm53 (675850) is incomplete**: the job died after 4 h. SGLang's pipeline-parallel scheduler (tp4 pp4
  over 4 nodes) failed on an RCCL send between stages (`ncclInternalError` / gloo "connection closed by
  peer"). The service step exited and the job tore the agents down. 9 episodes had started, none
  submitted. All 14 kernels are owed a rerun. The same crash ended the three temperature3 glm53 jobs.

## Final submissions

Each final re-times one graded source. Its origin is a `/submit`, or the best correct `/score` promoted when the
episode ended without one (token cap). The source text is exported under `finals/<setup>/`.

| setup | kernel | origin | call # | tokens at call | credited | correct | source |
|---|---|---|---|---|---|---|---|
| oss120b | householder_qr | submit | 8 | 2.5 M | 2.13x | yes | [`finals/oss120b-c/householder_qr.host.c`](finals/oss120b-c/householder_qr.host.c) |
| oss120b | ilu0 | submit | 1 | 0.2 M | 1.03x | yes | [`finals/oss120b-c/ilu0.host.c`](finals/oss120b-c/ilu0.host.c) |
| oss120b | jfnk_bratu | submit | 3 | 0.8 M | 10.04x | yes | [`finals/oss120b-c/jfnk_bratu.host.c`](finals/oss120b-c/jfnk_bratu.host.c) |
| oss120b | lanczos_reorth | submit | 13 | 9.4 M | 0.78x | yes | [`finals/oss120b-c/lanczos_reorth.host.c`](finals/oss120b-c/lanczos_reorth.host.c) |
| oss120b | mg_vcycle | submit | 9 | 6.0 M | 1.14x | yes | [`finals/oss120b-c/mg_vcycle.host.c`](finals/oss120b-c/mg_vcycle.host.c) |
| oss120b | mixed_precision_ir | submit | 2 | 0.6 M | 5.35x | yes | [`finals/oss120b-c/mixed_precision_ir.host.c`](finals/oss120b-c/mixed_precision_ir.host.c) |
| oss120b | rb_sor | submit | 2 | 1.2 M | 6.83x | yes | [`finals/oss120b-c/rb_sor.host.c`](finals/oss120b-c/rb_sor.host.c) |
| oss120b | rk45_ensemble | promoted | 5 | 0.0 M | 23.55x | yes | [`finals/oss120b-c/rk45_ensemble.host.c`](finals/oss120b-c/rk45_ensemble.host.c) |
| oss120b | rk4_ensemble | submit | 3 | 0.2 M | 38.30x | yes | [`finals/oss120b-c/rk4_ensemble.host.c`](finals/oss120b-c/rk4_ensemble.host.c) |
| oss120b | sgs_pcg | submit | 2 | 0.1 M | 1.37x | yes | [`finals/oss120b-c/sgs_pcg.host.c`](finals/oss120b-c/sgs_pcg.host.c) |
| oss120b | sparse_cholesky | submit | 4 | 0.5 M | 1.01x | yes | [`finals/oss120b-c/sparse_cholesky.host.c`](finals/oss120b-c/sparse_cholesky.host.c) |
| oss120b | sptrsv_level | promoted | 4 | 0.0 M | 1.32x | yes | [`finals/oss120b-c/sptrsv_level.host.c`](finals/oss120b-c/sptrsv_level.host.c) |
| qwen38 | householder_qr | promoted | 2 | 0.0 M | 5.68x | yes | [`finals/qwen38-c/householder_qr.host.c`](finals/qwen38-c/householder_qr.host.c) |
| qwen38 | ilu0 | promoted | 16 | 0.0 M | 2.27x | yes | [`finals/qwen38-c/ilu0.host.c`](finals/qwen38-c/ilu0.host.c) |
| qwen38 | jfnk_bratu | promoted | 3 | 0.0 M | 10.25x | yes | [`finals/qwen38-c/jfnk_bratu.host.c`](finals/qwen38-c/jfnk_bratu.host.c) |
| qwen38 | lanczos_reorth | promoted | 3 | 0.0 M | 3.21x | yes | [`finals/qwen38-c/lanczos_reorth.host.c`](finals/qwen38-c/lanczos_reorth.host.c) |
| qwen38 | mixed_precision_ir | promoted | 2 | 0.0 M | 2.08x | yes | [`finals/qwen38-c/mixed_precision_ir.host.c`](finals/qwen38-c/mixed_precision_ir.host.c) |
| qwen38 | rk45_ensemble | promoted | 5 | 0.0 M | 23.23x | yes | [`finals/qwen38-c/rk45_ensemble.host.c`](finals/qwen38-c/rk45_ensemble.host.c) |
| qwen38 | rk4_ensemble | submit | 5 | 6.5 M | 330.24x | yes | [`finals/qwen38-c/rk4_ensemble.host.c`](finals/qwen38-c/rk4_ensemble.host.c) |
| qwen38 | sgs_pcg | promoted | 4 | 0.0 M | 2.80x | yes | [`finals/qwen38-c/sgs_pcg.host.c`](finals/qwen38-c/sgs_pcg.host.c) |
| qwen38 | sptrsv_level | promoted | 4 | 0.0 M | 1.53x | yes | [`finals/qwen38-c/sptrsv_level.host.c`](finals/qwen38-c/sptrsv_level.host.c) |

## DB fixes applied (hb, rebuilt into this DB)

- Episodes the driver ended now name how: `budget` (rc 125, 12 qwen38 episodes) and `submitted` (rc 123, 1).
- The 6 glm53 episodes the crash cut off take their kernel from their grades. Their `result` stays empty: the job
  died before their driver wrote a record.
