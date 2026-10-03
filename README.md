# Bachelor thesis artifacts

Three independent artifacts.

| directory | what |
|---|---|
| **[`llr40Matrix/`](llr40Matrix/)** | the LLR-40 measurement matrix: six code representations of 40 loop-level-reasoning kernels, built, validated against a NumPy oracle and timed on CSCS Alps `daint` (NVIDIA GH200, aarch64). 240 cells plus 18 preset-S rows, every individual timing retained. **Current version: [`llr40Matrix/v2/`](llr40Matrix/v2/)**; the top level is v1, kept as the historical record. |
| **[`Solvers/`](Solvers/)** | fourteen solver kernels, and the preconditioning skill written against them: three successive versions plus the measurement scripts behind every figure it quotes. |
| **[`perfRuns/`](perfRuns/)** | perf and Nsight Systems profiles of five applications on `daint` (CoMet, QuaTrEx, SpBench/cuBool SpGEMM, GraphAIBench, ANMLZoo/VASim): the runs that identify each application's hot kernel, with the job scripts and the build patches needed on GH200. |

Each directory has its own README and stands on its own. `llr40Matrix/README.md` carries the
headline results and caveats for the measurement matrix; `Solvers/README.md` describes the kernels
and the skill; `perfRuns/README.md` summarises each application's profile.

## Reproducing

`llr40Matrix/REPRODUCE.md` regenerates the matrix and its figures from source. It needs an external
pinned checkout of the benchmark corpus, which is not vendored here.

The scripts under `Solvers/skills/*/measurements/` regenerate the figures quoted in the skill. They
carry absolute paths to the machine they were run on; see `Solvers/README.md`.

`perfRuns/*/job_perf.sh` re-run the profiles; they too carry absolute paths, and the application
builds need the patches in `perfRuns/build/` (see `perfRuns/README.md`).
