# Supplementary: whole-node step geometry (debug chain job 4969394, 2026-10-02 15:56-16:07)

A COMPLETE, valid run of the 240 main cells, preset F, and the task-4 A/B, but measured with a
DIFFERENT step geometry from v1 and from the v2 matrix, so it is not the v2 matrix.

Here every srun step got the whole node (`--cpus-per-task=288`). That lets `numactl
--cpunodebind=0` bind the timed child to socket 0 (72 cores), and the harness resolved gfortran's
`-ftree-parallelize-loops=72`: parloops is ACTIVE (22/40 Fortran libraries have outlined
`*._loopfn.N` or agent `*._omp_fn.N` bodies; run at OMP_NUM_THREADS=1).

In v1 the step had a 1-cpu cgroup, the harness resolved `-ftree-parallelize-loops=1` (parloops
OFF) and v1 recorded exactly that: no v1 Fortran-column library contains a `_loopfn` (checked on
the C-agent kernels, whose surviving Fortran library is the Fortran column's own sweep build).
The v2 matrix is therefore re-measured in the v1 geometry. This set is kept because it is the
only measurement with the gfortran autopar flag ACTIVE -- see ftree_parallelize_ab.csv in v2/.

The FPOFF unit failed here on an argparse bug (fixed); opt reports were not produced here.
