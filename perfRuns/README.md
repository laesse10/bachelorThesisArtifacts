# Application perf runs on Alps `daint` (GH200)

Profiles of five HPC applications, taken to find the hot kernel each one contributes to the
benchmark corpus. Every run was made on one NVIDIA GH200 (aarch64 Neoverse V2 + H100) of CSCS
Alps `daint` on 2026-10-02, with uenv `prgenv-gnu/25.6:v2` (gcc 14.2, CUDA 12.9, Nsight Systems
2025.1.3). WarpX is profiled separately and is not part of this directory.

| directory | application | hot spot found |
|---|---|---|
| [`comet/`](comet/) | CoMet 2-way CCC, CUTLASS INT4 tensor-core GEMM | INT4 GEMM = 99.7% of GPU time |
| [`quatrex/`](quatrex/) | QuaTrEx NEGF/GW, carbon nanotube | no hot GPU kernel; CPU-side eigensolve and ZGEMM |
| [`spgemm/`](spgemm/) | SpBench / cuBool boolean SpGEMM | row binning + analysis ≈55%, hash passes ≈45% of GPU time |
| [`graphai/`](graphai/) | GraphAIBench triangle counting | one kernel = 100% of GPU time |
| [`anmlzoo/`](anmlzoo/) | ANMLZoo automata on VASim (CPU) | NFA frontier step, `enableChildSTEs` + `computeSTEMatches` |

## Files

Each application directory holds:

| file | what |
|---|---|
| `report_<tag>.self.txt` | `perf report`, flat self time by DSO and symbol (≥0.5%) |
| `report_<tag>.tree.txt` | `perf report` with call chains (children) |
| `flame_<tag>.svg`, `stacks_<tag>.folded` | flamegraph and the folded stacks it is drawn from |
| `nsys_<tag>.nsys-rep`, `nsys_<tag>_*.csv`, `nsys_<tag>.kern_sum.txt` | Nsight Systems trace and its kernel / memory / API summaries (GPU apps) |
| `run_*.log` | program output of each profiled run |
| `job_perf.sh` | the Slurm job that produced the directory; sources [`common.sh`](common.sh) |

perf was recorded with `perf record --call-graph dwarf,4096`: at 99 Hz for the GPU apps and the
long ANMLZoo runs, at 999 Hz for the short QuaTrEx runs. The raw `perf.data` files are not
included: they are 6–375 MB each and only resolve against the exact binaries they were recorded
with; the folded stacks carry the same call-stack samples.

For the GPU applications perf sees only the host side, which is mostly the CUDA driver waiting
in `cudaDeviceSynchronize`; the nsys trace is the GPU-side profile.

## Applications

### CoMet — 2-way CCC on the INT4 tensor-core path (`comet/`)
- `genomics_metric --tc 6 --num_kernel 10`, 24000 vectors × 40000 fields, `--checksum no`.
  The CPU checksum is a verification aid that would otherwise take ~15× the GEMM's wall time.
- nsys: the CUTLASS INT4 GEMM is 99.7% of GPU time (one 4.53 s launch); the bit-extraction
  (`tc_buf_write_kernel_`) and output-repair (`tc_repair_metrics_kernel_`) kernels ≈0.1% each.
  Compute time 5.7 s, 4.2e13 GEMM ops/s, 21 GB GPU memory.
- Host side (perf): synthetic input generation (`set_vectors_analytic_`) 21%, the rest mostly
  the driver waiting on the GEMM.
- Correctness: the same build and code path reproduce the 2-way CCC small-case checksum given in
  CoMet's `doc/Quick_Start.txt`, `0-245201878478-801640733671948288`.

### QuaTrEx — carbon-nanotube GW, 4 SCBA iterations (`quatrex/`)
- Example `examples/w90/carbon-nanotube/gw`, run configs in [`quatrex/configs/`](quatrex/configs/).
- `cnt_gw_gpu`, default CuPy backend (nsys + perf): no dominant GPU kernel — the largest,
  `getrf_pivot`, is 8.6% of GPU time among ~280k small launches. The host side is dominated by
  LAPACK's non-symmetric eigensolve on the CPU (`zlaqr5`, `zlahqr`) in the open-boundary solve.
- `cnt_gw_cpu`, `QTX_ARRAY_MODULE=numpy`, `OPENBLAS_NUM_THREADS=1` (perf): complex ZGEMM ≈40%
  including packing, eigensolve ≈9%, pocketfft (GW energy convolution) ≈9%. The thread pin is
  needed: without it OpenBLAS's idle worker pool spins and dominates the profile.

### SpGEMM — SpBench cuBool boolean SpGEMM (`spgemm/`)
- `cubool_mult`, C = A·A on amazon-2008, web-Google, roadNet-TX, belgium_osm and roadNet-CA
  (SuiteSparse), 10 iterations each ([`spgemm5_cfg.txt`](spgemm/spgemm5_cfg.txt)).
- nsys: row analysis plus the three CUB `for_each` binning kernels ≈55% of GPU time, the hash
  count/fill kernels ≈45%, the large-row segmented radix sort 0.1%.
- `summary_cubool_mult5.txt` and `log_cubool_mult5.txt` are SpBench's own timing output; each
  holds two blocks, the first recorded under nsys, the second under perf.
- Correctness ([`spgemm/check/`](spgemm/check/)): checked against scipy, luxembourg_osm and
  roadNet-TX match exactly. web-Google differs in one row (768091): its product bound (4334)
  exceeds the largest hash bin (4096), and nsparse's large-row path (`count_nz_block_row_large`)
  appends products without de-duplicating, so that row holds 3819 duplicate columns. This is
  upstream cuBool behaviour and was left as-is so the profile measures upstream cuBool.

### GraphAIBench — triangle counting (`graphai/`)
- `tc_gpu_base` (warp-centric, edge-parallel) on SNAP com-Orkut, counting repeated 100 times
  (`TC_REPS=100`) so the profile shows the steady state rather than graph loading.
- 627,584,181 triangles (the published count); 68.3 ms per repetition.
- nsys: `triangle_bs_warp_edge` is 100% of GPU time.

### ANMLZoo — automata simulation on VASim (`anmlzoo/`)
- Four single-threaded runs (perf only): Brill, Fermi, EntityResolution and Snort automata on
  their 10 MB inputs (Fermi on its 1 MB input). Simulation times: entity10 490 s, brill10 814 s,
  snort10 890 s, fermi1 475 s; 47k–88k samples each.
- brill10 / entity10 / fermi1: the NFA frontier step — `Element::enableChildSTEs` 48–69% and
  `Automata::computeSTEMatches` 27–34% of self time.
- snort10, the only automaton with counter and gate elements: `computeSTEMatches` 14.5%,
  `SpecialElement::disable` 13.9%, `std::_Rb_tree_increment` 12.1%, `OR::calculate` 11.0%,
  `memcmp` 10.7%, `memcpy` 7.9%.

## Reproducing

The job scripts carry absolute paths to the machine they ran on. Building the applications on
GH200 needed source changes, which are in [`build/`](build/) as patches against the named
upstream commits:

| file | applies to |
|---|---|
| `comet_tcb1_deda5507.patch` | CoMet, branch `tcb1` at deda5507: an `ALPS_GH200` platform block; the Sm80 b1 16×8×256 MMA for kernel 52; a debugging kernel (num_kernel 7/177) stubbed on Sm80+, whose 8×8×128 AND MMA does not exist there; an `MPI_IN_PLACE` fix for the bundled MPI stub |
| `cubool_81573de.patch` | cuBool at 81573de: CUDA 12.9 / sm_90 build fixes (toolkit CUB, C++17, CCCL 2.x headers) and a static per-group mask for the pwarp shuffle reduction |
| `graphaibench_main_17834cb.patch` | GraphAIBench at 17834cb: `WARPS_PER_BLOCK` / `NUM_WARPS` as `constexpr` (clash with CUB 12) and the `TC_REPS` repetition loop |
| `graphaibench_tools_snap2csr.cc` | converter from a SNAP edge list to GraphAIBench's binary CSR |
| `vasim_mnrl_schema_aarch64.S` | aarch64 stand-in for MNRL's x86-only nasm schema object; VASim was also built with `-g -include cstdint` |
| `quatrex_gmsh_stub.py` | stub for `gmsh`, which has no aarch64 wheel; the GW path does not use it |

QuaTrEx ran from a Python 3.13 venv with numpy 2.5.3, scipy 1.18.1 and cupy-cuda12x 14.2.0,
mpi4py built from source against Cray MPICH, and the stub on `PYTHONPATH`.
