# scicomp40, C, HPCAgent-Bench v0.3.0 (second campaign of thesis Section 6.4)

Qwen3.8-27B (job 695497) and GPT-OSS-120B (job 695498) on the 40 kernels of the scicomp40 study, C only,
one agent per kernel with a 20 h budget, preset S plus fuzzed grading, one AMD MI300A node. Benchmark commit
`f729e1e5`. Run 2026-10-26; report by Y. Budanaz, 2026-10-27.

- `scicomp40-20261026-report.zip`: the report as received: `SUMMARY.md` (results and the issues found),
  `plots/` (per-kernel and per-run speedups), `extract/` (observations and every graded source), `tool_calls/`,
  and the results and judge databases of both jobs. The transcripts (`scicomp40-20261026-transcripts.zip`,
  414 MB) are too large for this repository.
- `own_code.py`: leaves out the graded answers that are the staged C reference, byte for byte, and prints the
  numbers the thesis quotes.

The report's grades are final as far as this run goes: per its author, the run has all the grades it can have.
The tooling cut the agents' feedback (score calls timing out at 1800 s, 29 runs without a final grade), so the
thesis uses this campaign only to check the ranking of the two models, and keeps the v0.2.1 campaign
(`../hb-baselines-v021/`) as the main data of Section 6.4.
