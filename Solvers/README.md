# Solvers

Fourteen solver kernels, and the preconditioning skill written against them.

| directory | what |
|---|---|
| [`kernels/`](kernels/) | the fourteen kernels: a NumPy reference, an input generator and a manifest each |
| [`skills/`](skills/) | the preconditioning skill, in three successive versions |

## The kernels

Each kernel directory holds three files: `<name>_numpy.py` is the correctness reference written as
explicit loops, `<name>.py` generates deterministic inputs, and `<name>.yaml` is the manifest
declaring problem sizes, array shapes and which outputs are graded.

| kernel | what it computes |
|---|---|
| `sgs_pcg` | preconditioned CG with a symmetric Gauss-Seidel sweep as the preconditioner |
| `mg_vcycle` | geometric multigrid V-cycle on a cell-centred 3-D grid |
| `amg_setup` | smoothed-aggregation algebraic multigrid setup |
| `sptrsv_level` | sparse triangular solve with level scheduling |
| `ilu0` | incomplete LU with zero fill-in |
| `rb_sor` | red-black Gauss-Seidel and SOR relaxation |
| `sparse_cholesky` | sparse direct Cholesky, supernodal |
| `lanczos_reorth` | Lanczos with full reorthogonalization |
| `householder_qr` | Householder QR and least squares |
| `mixed_precision_ir` | mixed-precision iterative refinement |
| `jfnk_bratu` | Jacobian-free Newton-Krylov on the Bratu problem |
| `bdf_newton_krylov` | implicit BDF time integration with a Newton-Krylov inner solve |
| `rk4_ensemble` | fixed-step Runge-Kutta over an ensemble |
| `rk45_ensemble` | adaptive Runge-Kutta with embedded error control |

Sources and licences are in each file's header. They are reimplementations from published
algorithms, GPL-3.0-or-later.

## The skill

`skills/` holds three versions of a skill that tells an agent whether a solver kernel's
preconditioner may be changed at all, and what to do in either case. `skills/README.md` has the
version table and what changed between them.

The short version: the literature judges a preconditioner by iteration count, while these kernels
are graded elementwise against a reference computation, and those two standards disagree by eight
or nine orders of magnitude. Most of the fourteen kernels turn out to freeze the preconditioner
entirely, so the work that pays off is making the given preconditioner faster rather than choosing
a better one.

## Running the measurement scripts

`skills/*/measurements/` holds the scripts behind every measured figure the skill quotes. They
import the benchmark's own operator generators through an absolute path near the top of each file:

```python
sys.path.insert(0, "/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-solver")
```

Change that to your own checkout before running. One script also needs a seeded matrix cache; its
README gives the environment variable. Only `07_jfnk_convergence_crossover.py` is self-contained
and runs anywhere with NumPy and SciPy.
