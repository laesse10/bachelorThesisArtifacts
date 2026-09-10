# Bachelor thesis artifacts

Two independent artifacts.

| directory | what |
|---|---|
| **[`llr40Matrix/`](llr40Matrix/)** | the LLR-40 measurement matrix: six code representations of 40 loop-level-reasoning kernels, built, validated against a NumPy oracle and timed on CSCS Alps `daint` (NVIDIA GH200, aarch64). 240 cells plus 18 preset-S rows, every individual timing retained. |
| **[`Solvers/`](Solvers/)** | fourteen solver kernels, and the preconditioning skill written against them: three successive versions plus the measurement scripts behind every figure it quotes. |

Each directory has its own README and stands on its own. `llr40Matrix/README.md` carries the
headline results and caveats for the measurement matrix; `Solvers/README.md` describes the kernels
and the skill.

## Reproducing

`llr40Matrix/REPRODUCE.md` regenerates the matrix and its figures from source. It needs an external
pinned checkout of the benchmark corpus, which is not vendored here.

The scripts under `Solvers/skills/*/measurements/` regenerate the figures quoted in the skill. They
carry absolute paths to the machine they were run on; see `Solvers/README.md`.
