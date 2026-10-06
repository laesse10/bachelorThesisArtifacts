# Follow-up experiments: deviations and incidents

Every way these runs depart from the follow-up task text or from the v2 protocol, and every
incident, with where the evidence is. Nothing in `llr40Matrix/` outside `v2/followup/` was changed.

## Inputs

1. **The v2 tree was missing from `main` while experiments 1, 2a, 3 and 4 ran.** Commit `e6d1162` ("Updates from Overleaf", merged into
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

   On 2026-10-06, `16f2b66` ("Restore files deleted by the Overleaf sync", not part of this
   follow-up) restored those files on `main`. The follow-up still cites `632952d`, the commit it
   actually read.

1a. **Two versions of the task.** Experiments 1, 2a, 3 and 4 were run and pushed under the first
   version of the task text. Experiment 2a is that version's experiment 2 (files `s2710_hoist.*`).
   The second version added experiment 2b. It also dropped two sentences: experiment 1's "if it
   vectorises with something other than the flag, e.g. a GCC version difference, say so" (answered
   anyway in `minmax_finite_math.md`) and experiment 3's optional perf recording (recorded anyway).
   Nothing was re-run for the second version.

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
   edit. No unit logged a shell error, and all five experiment-3 units printed their final
   `unit ... end` line, which comes after the `case` block, so the restored script resumed correctly.
9. **Experiment 4's first opt reports were overwritten.** The unchanged and variant cells of a
   kernel share `<kernel>/<column>/` names, and the first layout wrote both into one directory, so
   the variant's files replaced the unchanged cell's. These reports were deleted and regenerated
   compile-only in job 4982663, with separate subdirectories (`opt_reports/regmem/unchanged/`,
   `opt_reports/regmem/<variant>/`). Timing is unaffected.

## Repository

10. **The experiment-1 push ran without the preceding rebase.** `git pull --rebase origin main`
    refused to start because a running job had just appended to the tracked `srun_lines.txt`
    ("cannot pull with rebase: You have unstaged changes"). The command chain did not stop on that,
    because the pipe into `tail` hid the exit code, and `git push origin main` went ahead. It was
    a plain fast-forward (`4206fa4..36eb4da`), so `origin/main` had nothing new to rebase onto and
    the pushed history is what the prescribed sequence would have produced. Nothing was forced.
    Later pushes run `git pull --rebase --autostash origin main` and push only if it exits 0.

## Experiment 3

11. **The separation criterion was fixed after part of the data was seen.** It is Kendall's W of
    the GCC columns' per-round ranks with a permutation p < 0.05, and worst/best GCC minima above
    the median within-column round-to-round spread. It was written while three to five of the six
    rounds per kernel were in, and it was not changed once the final rounds arrived. It flags
    `tsvc_2_s2275` (p = 0.03), which the write-up reports as such rather than reading it away.
12. **perf and vmstat were recorded for every series**, as the task allows. They added no wall
    time beyond the fifo round-trips between reps. The vmstat deltas are node-wide and cover the
    whole series process, not only the timed reps (`unstable_interleaved.md`).

## Experiment 2b

13. **Same-node A/B, plus a C cell.** The task asks for the Fortran column of all 40 kernels with
    `-fno-protect-parens`, and for kernels whose Fortran min-of-k "moves by more than 5% with the
    flag". So that the move compares like with like, the unchanged Fortran cell was re-measured on
    the same node in the same unit, one arm after the other, with the order alternating along
    `fparens_roster.txt`. For the eight kernels, the unchanged arm also timed the C cell, because
    the predictions are stated against C and must use perf cycles, not an assumed clock. v2's values
    are listed beside them in `fortran_parens.csv`.
14. **Harness database lock: five cells re-measured.** In job 4990019, ten nodes ran against one
    bench worktree. The harness records every run in an SQLite file there (`hpcagent_bench0.db`),
    and for five cells finishing within 17 s on five nodes the write failed with `database is
    locked`. The driver then returned no timings, which the runner records as `build_error`, after
    the 30 timed reps had run. Both arms of those five kernels (`tsvc_2_s235`, `tsvc_2_s252`,
    `tsvc_2_s4112`, `tsvc_2_vtvtv`, `wf_triangular`) were moved to `discarded/4990019/` and
    re-measured together on one node, one kernel after another (job 4990145), so nothing contended
    for the database. No other cell was affected: all 88 timed rows used are `ok`, apart from
    `tsvc_2_s115`, which is `incorrect` in both arms as in v2.
15. **Opt-report indexes regenerated.** In 4990019 every `opt_reports_followup.py` call wrote its
    reports but failed to write its index, because the index directory (`parts/fparens/<arm>/`) did
    not exist. The script now creates it. The opt reports of the other 35 kernels were regenerated
    compile-only in job 4990146 (`FPARENS_OPTREP`). The five re-measured kernels got theirs in
    4990145.
16. **The output arrays are not in git.** The four builds' outputs plus the oracle come to 24 GB,
    for example `s311`'s `sum_out` is 1.76 GB per copy. They are in
    `/capstor/store/cscs/userlab/g34/lhulsbergen/llr40_followup/fparens_outputs/` (the g34 project
    store, which is not purged). `fortran_parens_outputs.csv` carries the sha256 of every array.
    The output runs are separate single-rep runs (0 warm-up, 1 rep, `variant=output_dump`,
    `parts/fparens_dump/`). They capture the harness's first validation call, are not timings, and
    are not used as such.
17. **"NumPy" is the harness oracle, which is numba-compiled.** At `26a4f0cf` the oracle is the
    NumPy reference compiled by `njit_reference` (no fastmath, no parallel). The outputs are
    compared against exactly that, the arrays the harness validates. At preset S it is bitwise
    identical to the interpreted reference for all eight kernels (`oracle_njit_check.txt`). It was
    not checked at preset M, where the interpreted loops would take minutes per kernel.
18. **Another session wrote to this clone during experiment 2b.** A second Claude Code session
    (`lhulsbergen-22`, apparently a copy of this one) worked in the same working tree from 13:34
    on 2026-10-06. It added `MISSED2B` and the `asm_lines_changed_vs_default` column to
    `analyze_followup.py`, and wrote `check_oracle_njit.py` and an `oracle_njit_check.txt`. On
    request it stopped writing. Its `analyze_followup.py` edits are kept. `check_oracle_njit.py`
    duplicates `oracle_njit_check.py` (same result) and is not committed, and
    `oracle_njit_check.txt` was regenerated with the committed script. The jobs, the cancellations
    and the moves to `discarded/` were each done once (checked with `sacct` and in the files).
