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
