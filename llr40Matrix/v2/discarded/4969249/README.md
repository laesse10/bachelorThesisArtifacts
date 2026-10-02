# Discarded: debug chain job 4969249 (2026-10-02 15:23-15:41, cancelled)

The rows themselves are well-formed, but they were measured with a different step geometry than
v1 and than the rest of v2, so they are not part of the matrix.

`chain_unit.sh` launched each unit as `srun --nodes=1 --ntasks=1 --exclusive ...` inside a
10-node allocation. That step's cgroup holds ONE cpu, so `numactl --cpunodebind=0` could not
widen it: the timed child ran pinned to 1 core instead of socket 0, and the harness resolved
`-ftree-parallelize-loops=1` (parloops off) for every Fortran build. In v1 the sweep's build
child saw more than one core: all 15 v1 Fortran libraries that call GOMP_parallel were built
inside their kernel's own v1 sweep window. So the Fortran/agent-Fortran cells here are not
comparable with v1, and task 4's two arms would have been identical.

Fixed: steps get the whole node (`--cpus-per-task=$SLURM_CPUS_ON_NODE`), and every chain job
first probes each node and refuses to measure unless the build child sees >1 core and the
resolved Fortran flag has n > 1.
