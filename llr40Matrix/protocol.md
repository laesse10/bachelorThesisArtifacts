# LLR-40 measurement matrix — pinned protocol

Written **before** any measurement. Once pinned, no cell deviates. Any deviation forced by a
kernel is a recorded row with a status code, never a special case.

Pinned: 2026-09-07. Machine: **CSCS Alps `daint`**, NVIDIA GH200, `aarch64` (Neoverse-V2).

---

## 0. Provenance

| item | value |
|---|---|
| benchmark repo | `git@github.com:spcl/HPCAgent-Bench.git` |
| benchmark commit | `e2bceb68db2b32dc2203a6c6d4bc169dff291a49` (branch `main`, 2026-09-07) |
| agent-artifact repo | `https://github.com/ThrudPrimrose/ICLR26Reproducibility.git` |
| agent-artifact commit | `2830ed3f5285c747de53dda2b04229d6a3c0f243` |
| kernel roster | the 40 manifests carrying `taxonomy.tags: llr-focus40` |

The roster was verified two independent ways — the tag in `origin/main`, and the distinct kernels
in the `llr40v9`/`llr40v10` campaign observations. Both give the same 40; the set difference is
empty in both directions. The roster is frozen in `roster40.txt`.

**Not used:** the pre-existing local checkout on branch `comet` (2026-08-07). It predates the
roster re-cut (missing 5 of the 40) and the C-ABI fix `cd9b3345`. All work uses a detached
worktree at `origin/main` under `bench/`.

---

## 1. Representations (6 timed + 1 oracle)

NumPy is the **correctness oracle only and is not a timed column**. The timed matrix is therefore
40 kernels x 6 representations = **240 cells**.

| # | representation | source | provenance |
|---|---|---|---|
| — | `numpy` | `<k>_numpy.py` | oracle only, never timed |
| 1 | `c` | autogen lowering `<k>_fp64.c` | translator output (`numpyto_c`) |
| 2 | `c_reference` | `<k>_reference.c` | hand-ported TSVC, the file agents were shown |
| 3 | `cpp` | autogen lowering `<k>_fp64.cpp` | translator output |
| 4 | `fortran` | autogen lowering `<k>_fp64.f90` | translator output |
| 5 | `numba` | autogen `<k>_numba_np.py` | translator output |
| 6 | `agent` | campaign submission, **used exactly as delivered** | `llr40v10` arms |

`c` and `c_reference` are carried as **separate columns by design**: the corpus states the split is
deliberate — the hand-written references exist to ask whether compilers vectorize human-written C
where they fail on translator-generated C. Collapsing them would destroy that question.

### Known coverage gaps (recorded as rows, not omissions)

| column | gap |
|---|---|
| `c_reference` | 5 of 40 have no hand reference (`compact_threshold_pack`, `scan_affine_decay`, `scatter_accum_dup`, `segment_reduce_ragged`, `versioned_distance_update`) -> `unsupported` |
| `agent` | `tsvc_2_s2233` has zero submissions across both campaigns (known open harness issue) -> `unsupported` |
| `cpp` | no agent C++ arm exists with full coverage; does not affect the autogen `cpp` column |

### Agent cell selection rule

Rank that kernel's submissions across the six full-coverage `llr40v10` arms by campaign-recorded
speedup; re-validate the top candidate **here** against the NumPy oracle; on failure fall to the
next. The arm, model, job id and delivered language of the chosen submission are recorded in
`notes` for every agent cell. Campaign verdicts are **not** trusted as correctness: they were
produced on Beverin (AMD MI300A, x86_64) and do not transfer to GH200/aarch64.

---

## 2. Toolchain

GNU throughout — `gcc`/`g++`/`gfortran` **13.3.1** (SUSE, 20250313).

`clang` and `flang` do not exist on this machine and no LLVM uenv is published for `gh200` on
`daint`. Using GNU for all three compiled languages holds the compiler backend genuinely constant
across the language columns, which the prompt's own clang+gfortran fallback would not have.

| column | compiler | flags (as resolved by the harness, single-core) |
|---|---|---|
| `c`, `c_reference` | `gcc` | `-O3 -march=native -fopenmp` + FP_RELAX + `-ffp-contract=fast -fstrict-aliasing -fPIC -include vecmath.h -Wall -Wextra -std=c23 -D_POSIX_C_SOURCE=199309L` |
| `cpp` | `g++` | as C, with `-std=c++23` |
| `fortran` | `gfortran` | `-O3 -march=native -fopenmp` + FP_RELAX + `-ffp-contract=fast -fstrict-aliasing -fPIC -ftree-parallelize-loops={n} -Wall -Wextra -std=f2018 -ffree-form -ffree-line-length-none` |
| `numba` | — | `@nb.njit(parallel=True, cache=True)`; **`fastmath` never set** |
| `agent` | inherits its language's line | `build` tokens dropped; `-I`/`-l` kept |

```
FP_RELAX = -fno-math-errno -fno-trapping-math -fno-signed-zeros
```

**`-ffast-math` is NOT used**, contradicting the original task text. The harness refuses it
deliberately: finite-math, reciprocal and approximate-intrinsic rewrites change the value a kernel
computes and would fail the NumPy oracle. Recorded as a deviation from the prompt, made knowingly.

**`-fassociative-math` is NOT used** (`flags.fp_associative: false`). See §5 — this is a live
threat to the C-vs-Fortran comparison and is carried openly rather than silently equalised.

---

## 3. Environment — identical for every timed run

```
OMP_NUM_THREADS=1
OMP_PLACES=cores
OMP_PROC_BIND=close
NUMBA_NUM_THREADS=1
```

`NUMBA_NUM_THREADS` is pinned in addition to the prompt's three: numba uses its own threading
layer and does **not** honour `OMP_NUM_THREADS`. Without it the numba column would silently be
the only threaded column.

Single-core is the pinned condition. Consequence, recorded not hidden: the `agent` column's
OpenMP pragmas become overhead rather than speedup, and numba's `parallel=True` yields one thread.
The matrix therefore measures **serial code quality**, not parallelization ability.

**Datatype** `float64` (the only one the campaigns used). **Preset `M`** for every cell.
Preset `S` (LEN_1D=512 / LEN_2D=64) was rejected: compiled cells land at ~450 ns, L1-resident and
dominated by loop startup and jitter.

---

## 4. Allocation and measurement

Exclusive whole node. Node: 4 sockets x 72 Neoverse-V2 cores = 288 cores, no SMT, 36 NUMA
domains, 870 GB.

```
srun --partition=normal --nodes=1 --exclusive --time=<hh:mm:ss> \
     --cpu-bind=cores --hint=nomultithread <cmd>
```

**Measurement-stability artifact -- diagnosed, bounded, and mitigated by reporting.**

An intermittent 1.6x bimodality appears in timed series: the same cell runs at ~120 ms or
~190 ms, switching in contiguous blocks *within* one 30-rep series. Reproduced deliberately on
the real kernel over 270 reps in 3 trials: **39 of 270 reps (14.4%) exceeded 150 ms**, and when
it strikes a cell's RSD goes from ~0.4% to 9-23%.

`tsvc_2_s3110` is a pure sequential scan of a 2.42 GB array, so wall time is bandwidth:
120 ms = 20.2 GB/s (fast state), 190 ms = 12.7 GB/s (slow state).

Four candidate causes were tested and **all four are excluded by direct measurement**:

| hypothesis | evidence | verdict |
|---|---|---|
| CPU frequency / thermal / power cap | `cycles/s` ratio slow:fast = **1.000** (3.26 GHz throughout); `cpuinfo_cur_freq` = 3411 MHz on every rep | excluded |
| core migration between sockets | `sched_getcpu()` = 0 on every rep; and a pinned/unpinned A/B made the **pinned** run the bad one | excluded |
| transparent-huge-page fallback | `dTLB miss %` 0.0099 -> 0.0113 (both negligible); `AnonHugePages` constant *within* each trial while reps flipped fast/slow | excluded |
| automatic NUMA page migration | `numa_balancing=0`, `numa_pte_updates=0`, `numa_pages_migrated=0` | excluded |

What the counters DO show, comparing slow to fast intervals:

```
IPC                 4.638 -> 3.073   (x0.663, matching the 1.51x wall slowdown)
instructions/s      1.51e10 -> 1.00e10   (x0.663)
cycles/s            3.255e9 -> 3.257e9   (x1.000)
L1D miss / 1k insn  3.36 -> 0.515    (x0.153 -- ~10x FEWER L1 misses per second)
```

Same clock, same core, fewer memory requests issued per second, lower IPC: the core is
**stalled waiting on DRAM**. The slow state is reduced achieved memory bandwidth caused by
something outside the measured process, not a property of the code under test. The specific
source is NOT identified (heavy `pgmigrate_fail` counts on these nodes point at background
memory compaction, but that was not confirmed).

**Consequence for reporting.** The fast state is the undegraded machine; the slow state is
external interference. `results.csv` therefore carries `time_ns_all` (every rep, as the protocol
already requires), `time_ns_median`, AND `time_ns_min`. Aggregates in Phase 5 use the **minimum**,
which is robust to this artifact and is also the harness's own default reducer
(`harness/timing.py:reduce_min_of_k`, "best-of-repeat"). The median is retained and reported, but
it is contaminated whenever more than half a series falls inside a slow block -- which did happen
(`tsvc_2_s3110`/`cpp`: 13 of 30 reps slow). Every cell whose RSD exceeds 5% is flagged in
`summary.md` as bimodal-suspect.

The exact srun line of the run that produced each row is recorded alongside `results.csv`.

**Per cell: 5 warmup runs discarded, then 30 timed runs minimum.** Every individual timing is
stored in `time_ns_all`, not just the median. Delivered through the harness's own
`harness/timing.py:sampled_reps`, which is the single owner of the warmup-discard rule
(`measurement.warmup=5`, `measurement.repeat=30`).

Validation is against the NumPy reference at the same preset and datatype, using the harness
tolerance band for `float64`. Every cell is validated before it is timed.

`status` in {`ok`, `build_error`, `incorrect`, `timeout`, `unsupported`}. Non-`ok` rows carry the
error text in `notes`. **These rows are results, not gaps.**

---

## 5. Known threats to comparability — carried, not concealed

1. **gfortran reassociates, gcc-C does not.** Under the pinned `FP_RELAX` and without
   `-fassociative-math`, gfortran treats `-fno-signed-zeros -fno-trapping-math` as authorising
   reassociation while gcc-C and clang do not. The benchmark authors measured this as a **3.1x
   C-vs-Fortran handicap living entirely in the flag list** (same dot product, same vendor, same
   flags: 3.72 ms C vs 1.20 ms Fortran). **Reductions are the largest class in the roster (8 of
   40)**, so this lands on the biggest group. Any C-vs-Fortran gap on a reduction kernel is
   suspect and is flagged per row.
2. **gfortran gets `-ftree-parallelize-loops`, gcc-C/C++ do not.** Only the gfortran block
   declares `doconcurrent_ref`, and it is appended outside any build-mode check, so it is present
   even single-core. At `OMP_NUM_THREADS=1` it should be overhead-only; the pilot quantifies it.
3. **`agent` is not a like-for-like column.** Submissions were produced on different hardware
   (MI300A/x86_64) against a different baseline, and are OpenMP-parallel code being timed at one
   thread. It measures "agent-optimized source, run serially here", nothing more.
4. **`c` vs `c_reference` differ in origin, not just text** — translator output vs hand-written.
5. Optimization classes are **fragmented**: 13 classes over 40 kernels, 7 of them with <=2
   members, and `induction_variable` (expected by the task) has **zero** members. Cross-class
   comparison is statistically thin and is reported as such.
