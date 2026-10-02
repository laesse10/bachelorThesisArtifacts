# Discarded: three kernels of chain job 4969572 timed a leftover agent/hand source

`tsvc_2_s115` (.f90), `tsvc_2_s311` (.f90) and `wf_triangular` (.c): the main bench tree still held
a substituted source from a cancelled job (4969228/4969249 were scancel'led mid agent cell, so
sweep.py's `finally` restore never ran). The emitter never regenerates a file without its autogen
marker, so the next sweep snapshotted that file as "pristine" and timed it as the autogen column:
v2 `s311/fortran` timed an agent OpenMP Fortran source (0.56x of v1), `wf_triangular/c` an agent
C source (1.55x of v1), `s115/fortran` an agent Fortran source (incorrect either way). Their opt
reports were built from the same snapshots. All 6 cells of each kernel are re-measured (job named
in results.csv `slurm_job`), so every kernel row comes from one run.

Fixed in sweep.py / sweep_variant.py: unmarked fp64 sources are removed before generation, and a
non-autogen snapshot aborts the kernel instead of being timed.
