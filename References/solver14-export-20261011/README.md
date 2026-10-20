# solver14 results so far (2026-10-11)

14 solver kernels, C on CPU, one agent per kernel, temperature 1.0. Jobs 675848 (qwen38), 675849 (oss120b) and
675850 (glm53; the server crashed after 4 h, so its kernels are owed; rerun 676607 is running and is not in here yet).
`REPORT.md` has the summary tables and the credited speedups.

| path | what |
|---|---|
| `REPORT.md` | summary: per setup and per kernel, findings, the final submission table |
| `solver14-20261010.db` | the merged results DB (every grade, source, episode) |
| `index.csv` | one row per grade: setup, kernel, kind (`score` / `submit` / `promoted` / `final`), correct, speedup, tokens at the call, link to the source |
| `episodes.csv` | one row per agent episode: how it ended (`budget` = token cap, `submitted`, `success`), turns, wall hours, tokens |
| `submissions/<setup>/<kernel>/` | the source text of every graded call the DB stores, named `<ts_ms>_g<grade id>_<kind>_<correct>.host.c` |
| `finals/<setup>/` | the source of each credited final |
| `workspaces/<setup>/<kernel>/` | each agent's working folder at the end of its episode (text files only; no `.so` / binaries) |
| `transcripts/<setup>/<kernel>.*` | the agent's prompt, token record, attempts and full log (`claude.log.gz`) |

The DB stores no source for an incorrect `/score` (22 calls): those rows are in `index.csv` without a link, and the
agent's last version of the file is under `workspaces/`. A `final` row re-times the source of the grade in its
`of_grade_id` column. glm53 has 13 workspaces and transcripts but only 9 episodes in the DB: the other 4 had made no graded call and
written no closing record when the server died.
