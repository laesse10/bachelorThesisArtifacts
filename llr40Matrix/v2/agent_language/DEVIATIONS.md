# agent_language (Part B): deviations and decisions

Every way this experiment departs from the task text or from the LLR-40 v2 protocol, with the reason.

1. **Started from scratch.** Part 0 found no earlier run (`extractedMatrix/PART0_agent_language_status.md`):
   no job, no directory, no partial result. Nothing was resumed.
2. **One whole-node srun step per kernel, with numactl choosing the binding per run.** v2 timed in
   one-core srun steps. Here each kernel's unit is one `srun --exclusive --cpus-per-task=288` step on
   an exclusive node, and every harness call is bound by `numactl`:
   - T1: `--physcpubind=0 --membind=0`, one core on NUMA node 0 (the core v2's one-core steps got);
   - T72: `--cpunodebind=0 --membind=0`, one Grace socket.

   The timed process therefore sees exactly v2's one-core binding at T1. The unit checks this before
   measuring: the build child sees 1 core and `-ftree-parallelize-loops=1` at T1, and 72 cores at T72
   (`PROBE` lines in `logs/unit.*.log`). Runs are strictly sequential, never two at once on a node.
3. **T72 runs the harness in `--mode multi_core`.** In `single_core` mode the harness forces every
   thread knob to 1 (`flags.cpu_env`). In `multi_core` it sets `OMP_NUM_THREADS` to the process's
   affinity count, which is 72, the value the task asks for. `OMP_PLACES=cores` and
   `OMP_PROC_BIND=close` are set explicitly. Submissions that set their own thread count are flagged
   (`sets_thread_count`) and not patched.
4. **T72 times the library that default/T1 built and validated.** The harness rebuilds only when a
   source is newer than `lib<k>_<fw>.so`. Nothing touches the source between the two runs, so no
   T72-binding flags (e.g. gfortran's `-ftree-parallelize-loops=72`) ever enter a build. Each T72 row
   records the library's sha256, and the runner refuses to time a T72 cell whose library mtime or hash
   changed. **Pilot exception:** the pilot (job 4992464, `tsvc_2_s3110`) ran with an earlier runner
   version that wrote the pristine sources back and re-placed the submission before T72, which can
   trigger a rebuild under the 72-core binding. In all its T72 rows the library is byte-identical to the
   default/T1 one (sha256), so the timed binary is the T1 binary, and the pilot's rows are kept. The
   runner was fixed before the full sweep.
5. **Translated C is timed at T1 as well as T72** (once per kernel each) in the same unit, so that the
   GH200 speedup over translated C at T1 uses a same-node reference. The task asks for T72 only. The v2
   matrix's `c` cell is the cross-job alternative.
6. **Same-compiler sensitivity: `uenv prgenv-gnu/25.6:v2` is available and used.** Its default view
   provides Spack-built GCC 14.2.0 for gcc, g++ and gfortran, and the harness resolves all three
   to it inside `uenv run prgenv-gnu/25.6:v2 --view=default`. For C this is the same version as the
   default (SUSE gcc-14 14.2.0) but a different build. For Fortran it replaces gfortran 13.3.1.
   Every candidate is built and validated with it, and timed at T1.
7. **T72 times VALID candidates only** (valid = default/T1 validated). Every T72 run is validated
   again, so a race at 72 threads shows up as `incorrect`.
8. **Debug-partition chain.** The full sweep runs as chained 30-minute, 10-node debug jobs
   (`agents_chain.sbatch`, as v2's `chain_debug.sbatch`), because normal-partition starts were
   estimated hours away. A unit cut at the wall limit resumes in the next job. Candidates are written
   ATOMICALLY (all of a candidate's rows at once), so a cut candidate is redone from default/T1, and
   T72 always follows its own T1 build.
9. **Harness results-DB shard per unit** (`HPCAGENT_BENCH_DB_SHARD`), so concurrent nodes never share
   one SQLite file (the lock that cost LLR-40 follow-up 2b five cells).
10. **`tsvc_2_s2233` has no submissions** in either campaign, so it has no rows. That leaves 39 kernels
    and 518 candidates (313 C, 205 Fortran), all sha256-checked against `agent_picks.json` before use
    (no mismatch).
11. **`run_agents.sh` was edited in place while the pilot's bash was executing it** (to add the
    `parts/<k>.done` marker). bash reads a running script by byte offset, so after the pilot's sweep
    step finished, it resumed in the edited file and stopped with a syntax error. By then the sweep step
    had written all 41 rows ("wrote 41 rows" precedes the error). The only losses are the unit's final
    `end` line and its done marker. The chain reaches `tsvc_2_s3110`, skips its recorded candidates
    and marks it done. No script was changed while the chain was running.
