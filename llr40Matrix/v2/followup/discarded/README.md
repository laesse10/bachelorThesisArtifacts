# Discarded follow-up runs

`4982625/` (experiment 3) and `4982626/` (experiment 4) were cancelled about 90 s into the run, by hand.
They were submitted with `--export=ALL,...,FU_KERNELS=k1,k2,...`, and `sbatch --export` splits on commas,
so each job received only its FIRST kernel and left its other nodes idle. Both were cancelled at
11:22:30 and resubmitted with `:` as the kernel separator.

- `4982625/tsvc_2_s1232.round1.csv`: two cells of round 1 (`c`, `c_reference`), cut off mid-round.
  An incomplete round cannot be interleaved with the others, so it is not used.
- `4982626/versioned_distance_update.base.csv`: the unchanged `c` and `agent` cells (complete), but
  the K==1 variant step was killed before its first cell. The whole unit was re-run, so base and
  variant come from the same job and node.

Kept and USED from the same mistake: job 4982623's `tsvc_2_s316` unit (experiment 1) had finished
all four cells and its opt reports at 11:22:21, before the cancel at 11:22:30 (`logs/MINMAX.tsvc_2_s316.4982623.log`).
The other three experiment-1 kernels ran in a resubmitted job. Job 4982624 (experiment 2, one
kernel) was unaffected.

## `4990019/` (experiment 2b): harness database locked

In job 4990019 the ten nodes ran 40 kernels' Fortran cells against ONE bench worktree. The harness
records every run in an SQLite file in that worktree (`hpcagent_bench0.db`). For five cells, finishing
within the same 17 s (13:29:27-13:29:44) on five nodes, the write failed with
`sqlite3.OperationalError: database is locked`. The driver then reported `status: error` with no
timings, which the runner records as `build_error`. The 30 timed reps had run (the perf gate counted 30)
and the build was fine: this is a storage collision, not a result. Affected: `tsvc_2_s235`
(-fno-protect-parens arm), `tsvc_2_s252` and `tsvc_2_s4112` (default arm), `tsvc_2_vtvtv` and
`wf_triangular` (-fno-protect-parens arm). Both arms of these five kernels are kept here and are not
used. Both arms were re-measured together on one node, one kernel after another, so nothing
contended for the database (see DEVIATIONS.md).
