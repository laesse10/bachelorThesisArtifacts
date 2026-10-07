# Part B pilot: one kernel, every candidate, steps 2 and 3

Kernel `tsvc_2_s3110`: 14 candidates (7 C, 7 Fortran, three models), job 4992464 (debug partition,
node nid006547, 2026-10-06 20:56-21:11). Every candidate went through all of step 2 (build and
validate, harness default and GCC 14.2) and step 3 (default build at T1 and T72, GCC 14.2 build at T1),
plus translated C at T1 and T72. Rows: `parts/tsvc_2_s3110.csv` (41).

## What the pilot showed

- **Geometry checks pass:** at T1 the build child sees 1 core and resolves
  `-ftree-parallelize-loops=1`; at T72 it sees 72 cores.
- **x86 intrinsics:** 3 of 7 C candidates fail to build, all three classified `x86_intrinsics`
  (`immintrin.h`), with both GCCs. All 7 Fortran candidates build and validate with both compilers.
- **The 11 valid candidates validate at T72 too.** In every case the T72 library is byte-identical
  to the default/T1 library (sha256), so T72 times the binary T1 validated (see DEVIATIONS.md item 4).
- **Compiler effects are visible already:** one kimi Fortran submission runs at 122.4 ms with
  gfortran 13.3.1 and 420.0 ms with gfortran 14.2.0 at T1.

## Time per candidate

Measured from the row timestamps (each harness call includes the oracle, the build, validation,
5 warm-up and 30 timed runs):

| run | n | mean | max |
|---|---:|---:|---:|
| default / T1, valid | 11 | 28.0 s | 42 s |
| default / T72 (valid only) | 11 | 12.8 s | 14 s |
| gcc14 / T1, valid | 11 | 28.2 s | 37 s |
| default / T1, build error | 3 | 5.7 s | 8 s |
| gcc14 / T1, build error | 3 | 6.7 s | 10 s |
| translated C, T1 + T72 (once per kernel) | 1 | 100 s | -- |

That is about 69 s per valid candidate and 12 s per candidate that fails to build: **57 s per
candidate on average** (796 s for 14), plus about 100 s of translated-C reference and 60-90 s of unit
start-up (generation, geometry probes) per kernel.

## Extrapolated total

518 candidates x 57 s = **8.2 node-hours**, plus 39 kernels x about 2.5 min = 1.6 node-hours, so
**about 10 node-hours**. Kernels whose per-call time differs from `tsvc_2_s3110`'s (about 0.1-0.8 s per
call at T1) will move this up or down. On 10 exclusive debug nodes that is about 1 hour of wall time,
run as 2-3 chained 30-minute jobs (`agents_chain.sbatch`). A unit longer than a job resumes in the
next one without losing more than the candidate that was cut.

Decision: continue with the full sweep as planned. The chain started at 21:11 (job 4992486), right
after the pilot, and its first job runs the ten largest kernels.
