# DaCe follow-up: every difference between the DaCe column and translated C++, tested

The DaCe column (`canon` pipeline, `new` codegen, g++ 13) differs from translated C++ (g++ 13, the same
compiler) by more than 3% on 23 of the 40 LLR-40 kernels (`../results.csv`, `speedup_vs_cpp`). For each, this
follow-up changes ONE thing and measures again: either DaCe's own generated code, edited after DaCe writes it
and before it compiles it, or the translated C++, with DaCe's change applied. Same benchmark commit
(`5cfcb4f2`), DaCe `dea0b39c`, harness `run_one`, preset M, validation against the NumPy oracle, 5 warm-up +
30 timed calls, one core of one exclusive node, NUMA node 0, one OpenMP thread (as `../dace_cells.py time`).
The cells of a kernel run in three rounds in alternating order (as listed, reversed, as listed) on one node.
All 255 timed cells validate.

**Result: one rule explains all 23.** Every gain is a restructuring that does not depend on the thread count
(interchange, distribution, fusion, the closed form of an induction variable, rematerialised scalars, tiling,
a library recurrence that keeps its value in a register); applied to C++ it makes C++ as fast as DaCe, undone
in DaCe it makes DaCe at least as slow as C++. Every loss is a form that pays off only with several threads
(a scan-based compaction, recurrences turned into scans, a library prefix sum, atomic updates, OpenMP regions
with worksharing loops); replaced by the sequential form it runs as fast as C++. Thesis: evaluation chapter,
Table "Every difference of more than 3% between DaCe and translated C++" (`tab:dace-causes`).

## How a cell changes one thing

- `dace_gap.py --override-cpp V` (DaCe side): hooks `dace.codegen.compiler.generate_program_folder`; after
  DaCe writes `src/cpu/canon_cpu.cpp`, `V` replaces it, then DaCe compiles. The harness's strict-FP rebuild
  (`canon_cpu_strict_fp`, tried only after a failed validation) gets the same code under its own symbol names.
  Every row records the replaced files and checks their bytes after the run. Each cell builds in its own,
  freshly deleted build folder.
- `dace_gap.py --override-src V` (C++ side): `V` is placed over the translated `cpp_backend/<k>_fp64.cpp`
  (touched, so the harness rebuilds) and restored afterwards.
- `variants/`: written by `make_dace_variants.py` from `../emitted/<k>/canon_new/src/cpu/canon_cpu.cpp`
  (DaCe side, `<k>_<name>.cpp`) and from `base_cpp/` (the translated C++ at `5cfcb4f2`, `<k>_<name>_fp64.cpp`).
  Every edit is asserted to apply exactly once; each file's header states it.

## Results (`summary_table.md`, `summary_table_round3.md`)

Fastest timed call (ms) over the three rounds.

| kernel | C++ | DaCe | cause | the one change | changed (ms) |
|---|---:|---:|---|---|---:|
| `s2233` | 415-421 | 36.5 | interchange + fusion of the two nests (cc read once) | C++ interchanged + fused (`cppinterfused`) | 35.8 (interchange alone 67.1) |
| `s231` | 507-681 | 66.6 | interchange | C++ interchanged | 68.5 |
| `s235` | 532 | 68.3 | distribution + interchange | C++ | 68.5 |
| `s2275` | 669 | 73.6 | distribution + interchange | C++ | 73.7 |
| `s1232` | 38.9 | 7.16 | interchange | C++ | 7.18 |
| `s233` | 564-721 | 362-408 | interchange of the aa statement only | C++ | 401-405 |
| `fuse_diamond` | 166 | 30.0 | fusion | C++ fused | 30.3 |
| `fuse_stencil_through_transient` | 112 | 50.1 | fusion | C++ fused | 47.5 |
| `s152` | 63.9 | 52.0 | fusion | C++ fused | 51.9 |
| `s453` | 104 | 64.8 | closed form of the induction variable | C++ closed form | 64.9 |
| `s255` | 76.5 | 65.3 | carried x, y rematerialised from b[i-1], b[i-2] | C++ rematerialised | 65.2 |
| `s119` | 69.3 | 55.1 | DaCe's OpenMP structure (parallel region, `omp for` on the inner loop) | C++ in the same structure; DaCe without its pragmas | 55.3; 69.4 |
| `s311` | 80.0 | 45.5 | `dace::reduce::sum` (`omp parallel for simd reduction`) reorders the sum | DaCe with an in-order loop | 80.1 |
| `wf_triangular` | 601 | 492 | skewed 64x64 tiling | DaCe untiled | 600 |
| `versioned_distance_update` | 272 | 219 | `inclusive_affine_strided`: each residue class's recurrence in a register | DaCe with the recurrence through memory | 384 |
| `compact_threshold_pack` | 102 | 232 | mask pass, exclusive scan, sum, scatter pass | DaCe: one sequential loop; four passes with plain loops | 98.2; 180 |
| `s323` | 78.6 | 160 | map, library scan, second map | DaCe: one sequential loop; library scan replaced only | 78.6; 149 |
| `s3112` | 79.7 | 99.6 | `dace::scan::inclusive_sum` (`omp simd` inscan) | DaCe with a plain loop | 79.7 |
| `scatter_accum_dup` | 344 | 508 | one `reduce_atomic` per element | DaCe with a plain update | 339 |
| `s115` | 52.5 | 56.2 | OpenMP region, worksharing inner loop | DaCe without pragmas | 52.9 |
| `wf_diff_skew` | 59.1 | 65.9 | OpenMP region, worksharing inner loop | DaCe without pragmas | 59.2 |
| `fuse_move_ifs` | 65.3 | 70.8 | fusion (one pass over src) gains, OpenMP region loses | DaCe without pragmas; C++ fused | 51.6; 50.7 |
| `s275` | 337-430 | 154 | interchange, but the band loop's `py_floor` bounds in the loop condition | DaCe with bounds hoisted; without pragmas; C++ interchanged | 91.4; 91.4; 91.2 |

Notes:
- Ranges: run-to-run variation of the bandwidth-sensitive kernels (the thesis's unreliable five, and `s233`).
  `s233`'s DaCe cell had one fast round (362 ms); its other rounds equal the changed C++ (408 vs 401-405).
- `versioned_distance_update`: written through memory, DaCe's code (with its two extra passes for the scan's
  inputs) is 0.71x of C++; the register recurrence turns that into 1.24x.
- `s275`: hoisting the bounds and removing the OpenMP region each recover the whole loss; GCC evidently does
  not hoist the bound computation inside the outlined region.

## Configuration claims (from `../`, no new runs)

The old-vs-new claims of the thesis rest on configurations that differ in one place: `canon_old` vs
`canon_legacy` differ only in tree reductions (atomics: 12.4x on `s3111`, `s313`, `vdotr`, 6.0x on `s319`);
`none_old` vs `none_legacy` on `s313`/`vdotr` differ only in `CopyND` (3 calls, no atomics in either: 4.8x);
`legacy` vs `new` differ only in the generator (same vectorisation in all 528 compiles, 1.00x on LLR-40).

## Files and runs

| path | what |
|---|---|
| `dace_gap.py` | one cell (the hook) |
| `run_dace_gap.sh`, `dace_gap.sbatch`, `units.txt` | one unit per node, three alternating rounds |
| `make_dace_variants.py`, `variants/`, `base_cpp/` | the variants and their sources |
| `analyze_dace_gap.py` | the tables |
| `out/rows/` | jobs 5011984, 5012055, 5012122, 5012237; `out/round3/rows/`: job 5012423 |
| `logs/` | Slurm, unit and per-cell harness logs; `srun_lines.txt` every srun |

Incidents: job 5012423 (`s2233b`, `fmib`, `s275b`) re-timed kernel/label pairs of the first jobs and wrote over
their rows on daint; the earlier rows had been copied off before, and its own 39 rows are kept separately in
`out/round3/rows/`. A per-unit output option was drafted for `run_dace_gap.sh` while that job was running and
reverted unused (the scripts here are the ones that ran).
