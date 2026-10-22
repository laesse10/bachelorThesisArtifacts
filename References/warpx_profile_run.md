# The WarpX profile run (thesis Section 3.2.4, Table 3.2)

The profile that chose the three WarpX kernels comes from one run on daint: a 3D uniform plasma, scaled from the
WarpX study of Myers et al. (Parallel Computing 108, 2021), 16 min 20 s. Its parameters and its `perf` profile are
recorded in [`WarpX Perf Run .pdf`](WarpX%20Perf%20Run%20.pdf) in this folder. **The input file itself no longer
exists:** the run folder on daint was emptied by the scratch cleanup (below).

## Parameters (from the PDF)

| Parameter | Value | Input key |
|---|---|---|
| Grid | 256 × 256 × 256 cells | `my_constants.nx = 256` |
| Time steps | 500 | `my_constants.nt = 500` |
| Domain | 40 µm cube, −20 to +20 µm on each axis | `geometry.prob_lo/hi` |
| Species | 2 (electrons and protons) | `particles.species_names` |
| Particles per cell | 8 per species (2 × 2 × 2), 16 in total | `num_particles_per_cell_each_dim` |
| Total particles | about 537 million (256³ × 16) | derived |
| Density | 10²⁵ m⁻³ per species | `.density` |
| Thermal velocity | 0.5 (dimensionless βγ) | `my_constants.uth = 0.5` |
| Shape order | 3 (cubic B-spline) | `algo.particle_shape = 3` |
| Current deposition | Esirkepov | `algo.current_deposition` |
| Pusher | Boris | `algo.particle_pusher` |
| Maxwell solver | Yee (FDTD) | `algo.maxwell_solver` |
| Boundaries | periodic, fields and particles, all axes | `boundary.*` |
| Output | none | no `diagnostics.diags_names` |

Binary: `warpx.3d.NOMPI.OMP.DP.PDP.OPMD.EB.QED` (the serial OpenMP build, `build_omp_serial`).

## Profile (from the PDF, `perf`, self time)

| Function | Self |
|---|---:|
| `PhysicalParticleContainer::PushPX` (Boris push with the field gather inlined) | 42.69% |
| `doEsirkepovDepositionShapeN<3>` (current deposition) | 40.79% |
| `UpdateMomentumBoris` (Boris momentum update) | 8.73% |
| AMReX particle bookkeeping | about 2.5% |
| `FiniteDifferenceSolver::EvolveBCartesian<CartesianYeeAlgorithm>` (field solver) | 0.50% |

## Where the input file was, and why it is gone

From the shell history on daint (`~/.bash_history`):

1. `mkdir -p $SCRATCH/runs/myers2021 && cd $SCRATCH/runs/myers2021`, then
   `cp $SCRATCH/warpx/inputs_myers2021 .` and `cp $SCRATCH/warpx/job_perf.sh .`; a first run at the deck's
   original size (`my_constants.nx = 128`, `my_constants.nt = 100`; job 4207122).
2. `sed -i 's/my_constants.nx  = 128/my_constants.nx  = 256/' inputs_myers2021` and
   `sed -i 's/my_constants.nt  = 100/my_constants.nt  = 500/' inputs_myers2021`, then
   `sbatch job_perf.sh inputs_myers2021`: the profiled run. `job_perf.sh` ran the binary under `perf record` on
   one node without MPI.
3. The flame graph of `perf_4207144.data` is titled "WarpX Myers2021 replication (nx=256, nt=500)", so the
   profiled run was job 4207144.

On 2026-10-23, `/capstor/scratch/cscs/lhulsbergen/runs/myers2021/` exists but is empty (folder last modified
2026-08-14), and `$SCRATCH/warpx/inputs_myers2021` and `job_perf.sh` are gone as well; neither file exists on the
author's Mac. This matches the automatic cleanup of the scratch file system, which removes files that have not
been used for some weeks. The WarpX tree on daint (`/capstor/scratch/cscs/lhulsbergen/warpx`, commit
`9751cd175` of 2026-07-10) holds no example with these settings.

The deck can be rebuilt from the table above. Settings the table does not give (such as `amr.max_grid_size`)
would have to be chosen again, so a rebuilt deck would be a reconstruction, not the profiled input.
