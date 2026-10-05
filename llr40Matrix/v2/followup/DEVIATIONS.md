# Follow-up experiments: deviations and incidents

Every way these runs depart from the follow-up task text or from the v2 protocol, and every
incident, with where the evidence is. Nothing in `llr40Matrix/` outside `v2/followup/` was changed.

## Inputs

1. **The v2 tree is no longer on `main`.** Commit `e6d1162` ("Updates from Overleaf", merged into
   `main` as `4206fa4` on 2026-10-05) deleted 1583 files, among them 1493 of `llr40Matrix/v2/`.
   They include `summary.md`, `opt_findings.md`, `results.csv`, `opt_reports_index.csv`,
   `srun_lines.txt`, `sweep_variant.py`, `opt_reports.py`, `variants/` and `emitted_sources/`
   apart from a few. `perfRuns/` and most of `plotting/` went with them. This follow-up did NOT restore
   them, because that would modify `llr40Matrix/` beyond `v2/followup/`. Instead:
   - the context files were read at `632952d`, the last commit before the deletion;
   - `analyze_followup.py` reads v2's `results.csv`, `opt_reports_index.csv` and opt reports with
     `git show 632952d:...`, so every "v2" number here is traceable to that commit;
   - `sweep_followup.py` is a copy of `632952d:llr40Matrix/v2/sweep_variant.py`, and
     `opt_reports_followup.py` reuses `632952d:llr40Matrix/v2/opt_reports.py`'s functions
     verbatim. `agent_picks.json` is copied from `632952d` because the runner reads it from its
     own directory.

## Measurement

2. **Partition `normal`, not v2's chained `debug` jobs.** The step geometry is v2's, unchanged:
   one task with one core on one named exclusive node per kernel (`srun --nodes=1 --ntasks=1
   -w <node> --exclusive --cpu-bind=cores --hint=nomultithread`), with the timed child bound to NUMA
   node 0. Every unit first probes that the build child sees exactly 1 core and
   `-ftree-parallelize-loops=1` and refuses to measure otherwise (`PROBE <node> 1 1` in each
   `logs/*.log`). No node ever ran two timed kernels, and compile-only opt-report steps ran on a
   node only after its timed steps.
3. **Submission mistake, two jobs cancelled and resubmitted.** The first submission passed the
   kernel list as `--export=...,FU_KERNELS=k1,k2,...`. `sbatch --export` splits on commas, so each
   multi-kernel job got only its first kernel. Jobs 4982625 (exp. 3) and 4982626 (exp. 4) were
   cancelled about 90 s in and resubmitted with `:` as the separator (4982638, 4982639). Their partial
   output is in `discarded/` (README there) and is not used. Job 4982623 (exp. 1) had already
   finished its only unit, `tsvc_2_s316`, before the cancel, and that unit IS used. Experiment 1
   therefore comes from two jobs, 4982623 (`s316`) and 4982637 (the other three), each kernel on
   its own node as always.
4. **A leftover variant source was removed automatically.** Cancelling 4982626 killed its runner
   before the `finally` that restores the pristine source, which left the K==1 variant in
   `versioned_distance_update/cpp_backend/`. The rerun's step 0 (inherited from v2's runner) found
   the unmarked file, deleted it and regenerated the lowering
   (`logs/REGMEM.versioned_distance_update.4982639.log`: "removing leftover non-autogen ..."). The
   unchanged `c` cell that was timed has sha256 `4ce0822d...`, identical to v2's emitted source.

## Sources and tooling

5. **Variant sources replace the autogen marker line by a comment.** The harness regenerates any
   emitted file whose first line carries `hpcagent_bench-autogen`, and leaves an unmarked one alone
   (its own instruction: "delete this line to keep local edits as a hand override"). So each
   emitted-source variant has that line replaced by a comment naming the variant, besides its one
   code change. The line is replaced, not deleted, to keep line numbers comparable with v2's
   reports. The comment does not change the code. `make_variants.py` applies the edits as exact
   single-match replacements, and `variants/*.patch` shows every change.
6. **Runner additions** (`sweep_followup.py` against v2's `sweep_variant.py`). The additions are a
   `variant`/`round`/`position`/`source_sha256`/`node` column; source overrides, re-checked by
   sha256 after the run so a regenerated file cannot be timed silently; `--ordered` (run columns in
   the given order); `--agent-sha` (time v2's chosen agent candidate only); and `--perf`/`--vmstat`.
   The timed path is unchanged: `hpcagent_bench.cli run` -> `Framework.measure`, 5 discarded
   warm-ups plus 30 timed reps.
7. **perf gating** (`perfgate/sitecustomize.py`, experiments 3 and 4). The driver runs under
   `perf stat -e cycles,instructions --delay=-1 --control fifo:...`, with counters starting
   disabled. A hook wraps `Framework.measure` and sends `enable` before each kept rep's
   `start_timer` and `disable` after its `stop_timer`. The fifo round-trips are outside the timed
   bracket. Warm-ups, input staging, the build, the oracle and validation are never counted. Every
   row records the number of gated reps (`perf_gated_reps`; all 30). Two side effects are recorded:
   - `perf_event_paranoid=2` on these nodes, so the counts are user-space only (`cycles:u`,
     `instructions:u`).
   - perf prepends `/usr/bin` to its child's PATH, where `g++` is GCC 7.5, so the driver is started
     through `env PATH=<the sweep's PATH>`. Before submitting, a login-node check confirmed that the
     libraries built under perf carry `GCC 13.3.1` (`cpp`, `fortran`) and `14.2.0` (`c`) in
     `.comment`, as every recorded row says.
8. **`run_unit.sh` was edited in place while experiment 3 was running (11:27), and restored in
   place within a minute (11:27:55).** bash reads a running script by byte offset, so an in-place
   edit can make it resume at the wrong place once the current compound command ends. All
   experiment-3 units were inside the already-parsed `case` block at the time, and the restored
   bytes are identical to the originals. The new version was then installed as a new file (rename),
   which running shells do not see. The experiment-1 units had ended at 11:25-11:26, before the
   edit. No unit logged a shell error.
9. **Experiment 4's first opt reports were overwritten.** The unchanged and variant cells of a
   kernel share `<kernel>/<column>/` names, and the first layout wrote both into one directory, so
   the variant's files replaced the unchanged cell's. These reports were deleted and regenerated
   compile-only in job 4982663, with separate subdirectories (`opt_reports/regmem/unchanged/`,
   `opt_reports/regmem/<variant>/`). Timing is unaffected.
