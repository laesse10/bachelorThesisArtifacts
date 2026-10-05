# Plotting scripts from HPCAgent-Bench

HPCAgent-Bench's plotting layer, vendored so the thesis figures can be drawn in the same style as
the benchmark's own papers, from this repository alone: no HPCAgent-Bench checkout or install is
needed.

| | |
|---|---|
| source | <https://github.com/spcl/HPCAgent-Bench> |
| commit | `26a4f0cfc1540cead109dfc8d736f1c45263e390` (`main`, 2026-09-28), the same commit `llr40Matrix/v2` was measured at |
| licence | GPL-3.0-or-later, see [`LICENSE`](LICENSE); every file keeps its upstream copyright header |

Every file copied from upstream is **unmodified** and sits at its upstream path, so it can be
diffed against its upstream twin. Only three files are new here: [`run.py`](run.py),
[`tests/conftest.py`](tests/conftest.py) and [`requirements.txt`](requirements.txt).

## Running a script

```bash
python3 -m venv venv && ./venv/bin/pip install -r plotting/requirements.txt
./venv/bin/python plotting/run.py plot_llr40_compilers --help
./venv/bin/python plotting/run.py plot_score_change observations.csv --experiment llr40v9 --out fig
```

`run.py` puts the vendored `hpcagent_bench` and `numpyto_common` packages on `sys.path` (upstream
installs them with pip) and runs the named script from `statistics/`. On a non-Linux machine it also
defines five Linux namespace constants as 0: `hpcagent_bench/seal.py`, the benchmark's sandbox,
reads them at import time, and plotting reaches it through the harness imports of the scaling,
speedup and transfer figures but never enters the sandbox.

## Testing the copy

```bash
./venv/bin/python -m pytest plotting/tests -q
```

The 29 test files are upstream's tests for the figures, scripts, style, palette and statistics,
run unmodified. Result on the commit above: **864 passed, 2 skipped**. The two skips check files
outside the plotting layer (campaign `.env` files, `statistics/ablation_stats.py`); the reason is
printed in every run. Upstream's own `tests/conftest.py` starts the judge service, so a minimal one
replaces it.

## What is here

| path | what |
|---|---|
| `statistics/plot_*.py`, `statistics/check_paper_figures.py` | the nine command-line scripts; they only parse arguments |
| `hpcagent_bench/stats/` | the figure builders (`figures/`), `style.py`, `palette.py`, `summary.py` and the rest of the statistics |
| `hpcagent_bench/*.py`, `hpcagent_bench/harness/`, `frameworks/`, `support/` | the modules the plotting layer imports, found by following its imports from upstream and confirmed by running every script and test |
| `hpcagent_bench/numpy_translators/src/numpyto_common/` | `dtypes` and `naming`, the two modules of the translator package the vendored code imports |
| `hpcagent_bench/benchmarks/**/*.yaml` | the 699 kernel manifests, YAML only: per-kernel figures label kernels with their manifest short names, and the observation reader uses the track |
| `hpcagent_bench/config.yaml`, `hpcagent_bench/envs/*.yaml`, `experiments/tags.yaml` | data the modules read at import time: config, compilers, cost cards, libraries, toolset, registry, tags |
| `experiments/token_cost.py` | the token fold `observations_extract.py` loads when it reads raw run directories |
| `docs/plotting.md`, `docs/measurement_statistics.md` | the figure design contract and the statistics behind it |
| `tests/` | the upstream tests described above |

The scripts, by what they draw:

| script | figure |
|---|---|
| `plot_llr40_compilers.py` | llr-focus40: DaCe, the polyhedral compilers and every model's CPF arm, speedup over numba |
| `plot_transfer.py` | MI300A → GH200 transfer of the LLR-40 final answers, speedup on each machine |
| `plot_speedup.py` | per-kernel median speedup as signed change, in order-of-magnitude bands, from a results DB |
| `plot_canon_speedup.py` | median speedup over the track baseline, one bar per framework |
| `plot_score_change.py` | efficacy: speedup and token cost of an intervention against its control |
| `plot_arm_summary.py` | geomean speedup and median spend per arm, with and without the skills packet |
| `plot_cost_weighting.py` | an intervention's token-cost ratio under each cost weighting |
| `plot_scaling.py` | weak and strong scaling for the distributed track |
| `check_paper_figures.py` | checks that a LaTeX paper places every figure at the width it was drawn at |

## Limits

- **`plot_results.py` is not included.** Upstream it is a shim for the benchmark's command line
  (`hpcagent-bench plot`), whose import chain is the agent runner, prompt templates and web search,
  i.e. most of the benchmark. The figure builder behind it, `hpcagent_bench/stats/figures/results.py`,
  is included and tested.
- **Input data must match this commit's format.** The scripts read observations extracted by
  `python -m hpcagent_bench.experiments` at a compatible commit. The older LLR-40 observations in
  ICLR26Reproducibility (commit `2830ed3f`) lack the three token components every cost card needs,
  so `plot_score_change` refuses them with "re-extract the observations", exactly as upstream would.
- **Updating.** To move to a newer upstream commit, re-copy the same paths with
  `git archive <commit> <paths> | tar -x -C plotting`, then rerun the tests: a new import surfaces
  there as a `ModuleNotFoundError`.
