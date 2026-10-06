#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --partition=normal
#SBATCH --exclusive
#SBATCH --output=logs/%x.%j.out
#SBATCH --error=logs/%x.%j.out
# Part A sweep: one exclusive node per kernel; each node runs its one unit (run_extracted.sh).
#   sbatch --job-name=ext-sweep --nodes=8 --time=03:00:00 --export=ALL,EXT_KERNELS=k1:k2:...,EXT_STEP_MIN=150 extracted.sbatch
set -uo pipefail
cd "${EXT_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts-extracted/extractedMatrix}"
NODES=($(scontrol show hostnames "$SLURM_JOB_NODELIST"))
KS=(${EXT_KERNELS//:/ })   # ':'-separated: sbatch --export splits on commas
echo "job=$SLURM_JOB_ID nodes=${NODES[*]} kernels=${KS[*]} start=$(date -Is)"
[ "${#NODES[@]}" -ge "${#KS[@]}" ] || { echo "need ${#KS[@]} nodes"; exit 1; }
for i in "${!KS[@]}"; do
  bash run_extracted.sh "${KS[$i]}" "${NODES[$i]}" "$EXT_STEP_MIN" $((100 + i)) > "logs/unit.${KS[$i]}.${SLURM_JOB_ID}.log" 2>&1 &
done
wait
grep -h "GEOMETRY_FAILED" logs/unit.*."${SLURM_JOB_ID}".log && echo "GEOMETRY CHECK FAILED on some node"
echo "job=$SLURM_JOB_ID end=$(date -Is)"
