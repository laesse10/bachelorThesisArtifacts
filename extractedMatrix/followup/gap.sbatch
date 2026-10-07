#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --exclusive
#SBATCH --output=logs/%x.%j.out
#SBATCH --error=logs/%x.%j.out
# One unit per exclusive node (run_gap.sh); units from units.txt.
#   sbatch --job-name=gap --partition=debug --nodes=9 --time=00:30:00 --export=ALL,GAP_UNITS=u1:u2:...,GAP_STEP_MIN=25 gap.sbatch
set -uo pipefail
cd "${GAP_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts-extracted/extractedMatrix/followup}"
NODES=($(scontrol show hostnames "$SLURM_JOB_NODELIST"))
US=(${GAP_UNITS//:/ })
echo "job=$SLURM_JOB_ID nodes=${NODES[*]} units=${US[*]} start=$(date -Is)"
[ "${#NODES[@]}" -ge "${#US[@]}" ] || { echo "need ${#US[@]} nodes"; exit 1; }
for i in "${!US[@]}"; do
  bash run_gap.sh "${US[$i]}" "${NODES[$i]}" "$GAP_STEP_MIN" $((200 + i)) > "logs/unit.${US[$i]}.${SLURM_JOB_ID}.log" 2>&1 &
done
wait
grep -h "GEOMETRY_FAILED" logs/unit.*."${SLURM_JOB_ID}".log && echo "GEOMETRY CHECK FAILED on some node"
echo "job=$SLURM_JOB_ID end=$(date -Is)"
