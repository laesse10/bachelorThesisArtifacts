---
name: solver-preconditioners-samples
description: "NumPy formulations of runs already done on solver kernels -- preconditioner swaps in a fixed-sweep PCG, level-scheduled triangular solves, locating a binding iteration cap, a singularity check, batched reorthogonalization -- each with its outcome and whether the change was good or bad."
when: "you want a worked example of how a solver-kernel change was checked and how it turned out"
---

# Samples: runs already done

A graded solver kernel is judged elementwise against one reference computation, not by iteration count. A preconditioner is **frozen** when the output is a fixed iterate (a fixed sweep, cycle or step count), when any graded output is a count, flag or history, or when a tolerance loop's iteration cap binds anywhere in the input range. The page `solver-preconditioners-verdict` decides which case you are in.

NumPy formulations of the runs behind the tables of this skill, each with what was run, what came out, and
whether the change was good or bad. They are patterns to read and translate into the kernel's own
language, not scripts to call.

### Sample 1: the budget ratio

```python
RTOL, ATOL = 1e-9, 1e-11                       # the task's tolerances; these are fp64 examples
def budget(got, ref):                          # <= 1.0 passes
    return float(np.max(np.abs(got - ref) / (ATOL + RTOL * np.abs(ref))))
```

Every number on this page marked "of the budget" or "times the budget" is this function.

### Sample 2: swapping the preconditioner inside a fixed-sweep PCG

```python
def pcg_fixed(A, b, niter, M):                 # the reference recurrence, x0 = 0, FIXED niter
    x = np.zeros_like(b); r = b.copy()
    z = M(r); p = z.copy(); rz = r @ z
    for _ in range(niter):
        q = A @ p; alpha = rz / (p @ q)
        x += alpha * p; r -= alpha * q
        z = M(r); rz_new = r @ z
        p = z + (rz_new / rz) * p; rz = rz_new
    return x

# sgs_seq: sequential symmetric Gauss-Seidel, row by row (the reference);
# sgs_levels: the same two sweeps, level-scheduled (Sample 3);
# gs2(r, k): k inner Jacobi sweeps per triangle; mcsgs: greedy-coloured SGS
ref = pcg_fixed(A, b, niter, sgs_seq)
for name, M in [("level-scheduled SGS", sgs_levels), ("Jacobi", lambda r: r / d),
                ("two-stage SGS, 3 inner", lambda r: gs2(r, 3)),
                ("two-stage SGS, 10 inner", lambda r: gs2(r, 10)), ("multicolour SGS", mcsgs)]:
    print(name, budget(pcg_fixed(A, b, niter, M), ref))
```

Ran on a 9-point grid operator with edge weights log-uniform on [1, 100], 64² with 25 sweeps and
128² with 50. Outcome: level-scheduled SGS 1.5e-4 and 6.1e-4 (**good**, passes); Jacobi 5.0e+10,
two-stage with 3 inner sweeps 2.7e+9, with 10 inner sweeps 2.4e+6, multicolour 6.7e+9 (**bad**, all
fail), although two-stage with 10 inner sweeps needs exactly the reference's iteration count to
converge.

### Sample 3: level-scheduled triangular solve, the legal parallel form

```python
def levels(A):                                 # level[i] = 1 + max(level[j] : j < i, A[i,j] != 0)
    lev = np.zeros(A.shape[0], dtype=np.int64)
    for i in range(A.shape[0]):
        cols = A.indices[A.indptr[i]:A.indptr[i+1]]
        lower = cols[cols < i]
        lev[i] = lev[lower].max() + 1 if lower.size else 0
    return lev

def solve_lower_by_levels(L_strict, d, b, lev):
    x = np.zeros_like(b)
    for l in range(lev.max() + 1):             # levels run in order: each is a barrier
        rows = np.where(lev == l)[0]           # rows inside a level are independent: parallel
        x[rows] = (b[rows] - L_strict[rows] @ x) / d[rows]
    return x
# the backward sweep is the same with the upper triangle and levels computed from j > i
```

Ran as the forward and backward sweep of SGS in Sample 2. Outcome: identical values up to
reassociation inside each row, 1.5e-4 of the budget (**good**). The level counts it produced are the
`3k - 2` table on the page `solver-preconditioners-frozen`. Contrast multicolouring, which also gives parallel groups but changes
which values each row reads (**bad**, 6.7e+9).

### Sample 4: locating where an iteration cap binds

```python
def newton(N, f, cap=20, prec=None):           # stop at ||F|| <= 1e-12 ||F0||, at most `cap` steps
    u = np.zeros((N, N)); F = residual(u, N, f); f0 = norm(F); steps = 0
    for _ in range(cap):
        if norm(F) <= 1e-12 * f0: break
        du = gmres_one_cycle(u, F, restart=30, tol=1e-4, prec=prec)   # Jacobian-free J·v
        u += du; F = residual(u, N, f); steps += 1
    return u, steps, norm(F) / f0

for N in (16, 32, 48, 64, 96, 128):            # span the whole declared size range
    ref, steps, ratio = newton(N, f)
    fast, _, _ = newton(N, f, prec=fast_poisson(N))
    print(N, steps, ratio, budget(fast, ref))
```

Ran on `-Lap(u) + u^3 = f`, unit square, zero boundary. Outcome: the reference converges at N up to
64 (7 to 17 Newton steps) and is capped at N=96 and N=128 (20 of 20). The fast-Poisson
preconditioned solve needs about 30 Jacobian applications instead of 170 to 600 at every N, and
agrees with the reference within 0.18 of the budget up to N=64 (**good** there), but misses by
1.6e+4 at N=96 and 1.8e+7 at N=128 (**bad** there). Testing one small size would have called it
legal.

### Sample 5: the singularity check, one matvec

```python
ones = np.ones(A.shape[0])
print(np.abs(A @ ones).max() / abs(A).sum(axis=1).max())   # ||A·1||_inf / ||A||_inf
```

Ran on an 8-neighbour grid graph Laplacian (diagonal = sum of the edge weights). Outcome: 1.0e-16,
singular to machine precision with the constant vector in its null space (**bad** news for anything
that assumes definiteness: incomplete factorizations can break down, CG drifts in the null space).

### Sample 6: batching a reorthogonalization into a GEMV

```python
for _ in range(2):                             # the reference reorthogonalizes twice
    # reference, modified Gram-Schmidt: w changes inside the loop
    #   for p in range(j + 1): w -= (Q[:, p] @ w) * Q[:, p]
    # batched, classical Gram-Schmidt: one GEMV pair against the same w
    dots = Q[:, :j+1].T @ w
    w -= Q[:, :j+1] @ dots
```

Ran on a Lanczos iteration over the 9-point operator, 64² grid. Outcome: the basis within 4.7e-5 of
the budget at m=50 (**good**) but 1.1e-3 at m=100, still passing, with the drift growing far faster
than m: check the largest m the task allows. This was measured only with the pass done twice; where
the reference does a single pass, do not assume the same margin, because classical Gram-Schmidt in
one pass is known to lose orthogonality.

## Other pages of this skill

- `solver-preconditioners-verdict`: the first page to read for any solver kernel: whether the preconditioner may change
- `solver-preconditioners-frozen`: the preconditioner is frozen: make it fast without changing what it computes
- `solver-preconditioners-ladder`: the preconditioner may change: which one to choose
- `solver-preconditioners-verify`: checking agreement and triaging a wrong answer
