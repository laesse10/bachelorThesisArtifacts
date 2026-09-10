# Measurements behind SKILL.md

Every number in `../SKILL.md` marked **measured** comes from one of these. They import the
operator generators of a real graded solver benchmark, so they measure real operators rather than a
model of them. The skill quotes them as evidence for general rules. These
scripts are how you check a rule still holds on whatever operator you are actually facing, and
they are meant to be RUN rather than read.

Run with an interpreter that has numpy and scipy:

```
/capstor/scratch/cscs/lhulsbergen/gaivenv/bin/python 01_level_structure_stencil.py
```

Each hardcodes `sys.path.insert(0, "/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-solver")`.
Point that at whichever checkout you want to measure, or replace the generator import with your
own operator.

| script | produces | establishes |
|---|---|---|
| `01_level_structure_stencil.py` | levels, average and maximum rows per level for a 27-point operator in lexicographic order | level structure |
| `02_preconditioner_iteration_counts.py` | CG iterations to 1e-8 for 15 preconditioners, plus the spectrum, the singularity test and the greedy colouring | operator class, ladder |
| `03_substitution_vs_graded_band.py` | how far each preconditioner swap moves a fixed-iterate PCG output, as a multiple of the fp64 budget | freedom verdict |
| `04_level_structure_suitesparse.py` | level structure of four application SPD operators | level structure |
| `05_lanczos_cgs_and_lapack_qr.py` | whether batching a Lanczos reorthogonalization into a GEMV stays in band, and whether LAPACK `dgeqrf`/`dorgqr` reproduces a textbook Householder QR | level structure |
| `06_jfnk_fast_poisson_precond.py` | Arnoldi steps saved by a DST fast-Poisson preconditioner on a Newton-Krylov solve, and the resulting budget violation | freedom verdict, ladder |
| `07_jfnk_convergence_crossover.py` | the problem size at which a Newton loop stops converging and starts hitting its iteration cap | freedom verdict |

`04` needs its matrix cache seeded or it will hit the network from inside `initialize()`:

```
export HPCAGENT_BENCH_CACHE_DIR=/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench/hpcagent_bench/.hpcagent_bench_cache
```

The two most reusable pieces, if you are adapting these to another solver:

```python
RTOL, ATOL = 1e-9, 1e-11                      # fp64 grading band
def budget(got, ref):                         # <= 1.0 passes
    return float(np.max(np.abs(got - ref) / (ATOL + RTOL * np.abs(ref))))

def levels_natural(A):                        # level[i] = 1 + max(level[j] : j < i, A[i,j] != 0)
    n = A.shape[0]; ip, ic = A.indptr, A.indices
    lev = np.zeros(n, dtype=np.int64)
    for i in range(n):
        c = ic[ip[i]:ip[i+1]]; p = c[c < i]
        lev[i] = (lev[p].max() + 1) if p.size else 0
    return lev
```
