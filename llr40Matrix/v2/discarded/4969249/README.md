# Discarded: debug chain job 4969249 (2026-10-02 15:23-15:41, cancelled)

CORRECTION (2026-10-02, later the same day): the reason originally given here was wrong.

These 25 rows were measured in the 1-cpu step geometry, which is the SAME geometry as v1
(`-ftree-parallelize-loops=1`, parloops off, timed child on one core). I cancelled the job
believing v1's Fortran builds had parloops active, inferred from GOMP_parallel calls in the v1
libraries. Those calls are `*._omp_fn.N` (OpenMP from agent Fortran sources), not parloops'
`*._loopfn.N`; no v1 Fortran-column build has a `_loopfn`. The rows are not wrong; they are a
partial run superseded by the complete re-measurement in the same geometry, and are not merged so
that the v2 matrix comes from one uninterrupted configuration.
