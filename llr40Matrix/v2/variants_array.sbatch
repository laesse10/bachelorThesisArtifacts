#!/bin/bash -l
#SBATCH --job-name=llr40v2-variants
#SBATCH --account=g34
#SBATCH --partition=normal
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=03:00:00
#SBATCH --array=1-14
#SBATCH --output=sweep_logs/variants.%A_%a.out
#SBATCH --error=sweep_logs/variants.%A_%a.err
#
# Tasks 3.2 (preset F), 3.3 (s115, -ffp-contract=off) and 4 (gfortran -ftree-parallelize-loops A/B).
# Same srun line, warmup/reps and time limit as sweep_array.sbatch. One kernel per exclusive node;
# the two A/B arms run one AFTER the other on the same node, never concurrently. Each bench
# worktree is the pinned commit plus exactly one patch in variants/ (abwith: no patch).
set -euo pipefail
cd "${LLR40_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts/llr40Matrix/v2}"
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
WT=/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2
IFS=$'\t' read -r KIND KERNEL < <(sed -n "${SLURM_ARRAY_TASK_ID}p" variants/jobs.tsv)
mkdir -p variant_parts variant_attempts_parts
echo "task=$SLURM_ARRAY_TASK_ID kind=$KIND kernel=$KERNEL node=$(hostname) $(date -Is)"
run() {  # $1 bench worktree  $2 preset  $3 preset label  $4 flags_variant  $5 reprs  $6 tag
  local CMD=(srun --exclusive --cpu-bind=cores --hint=nomultithread
    env LLR40_BENCH="$1" "$PY" sweep_variant.py
      --kernels "$KERNEL" --preset "$2" --preset-label="$3" --flags-variant="$4" --reprs "$5"
      --warmup 5 --reps 30
      --out "variant_parts/${6}.${KERNEL}.csv"
      --scratch "scratch/variants/${6}.${SLURM_ARRAY_TASK_ID}"
      --attempts-out "variant_attempts_parts/${6}.${KERNEL}.csv"
      --attempt-logs agent_attempt_logs_variants
      --emitted-dir "emitted_sources_variants/${6}")
  echo "SRUN: ${CMD[*]}"
  echo "$(date -Is) job=${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID} node=$(hostname) ${CMD[*]}" >> srun_lines.txt
  "${CMD[@]}"
}
case "$KIND" in
  F)     run "$WT-F"     S F "" c,cpp,fortran,numba,c_reference,agent presetF ;;
  FPOFF) run "$WT-fpoff" M M "-ffp-contract=off" c,cpp,fortran,c_reference fpoff ;;
  AB)    if (( SLURM_ARRAY_TASK_ID % 2 )); then
           run "$WT-abwith" M M "with-ftree-parallelize-loops"    fortran abwith
           run "$WT-nopar"  M M "without-ftree-parallelize-loops" fortran abwithout
         else
           run "$WT-nopar"  M M "without-ftree-parallelize-loops" fortran abwithout
           run "$WT-abwith" M M "with-ftree-parallelize-loops"    fortran abwith
         fi ;;
esac
echo "DONE $KIND $KERNEL $(date -Is)"
