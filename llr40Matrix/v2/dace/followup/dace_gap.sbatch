#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --exclusive
#SBATCH --output=logs/%x.%j.out
#SBATCH --error=logs/%x.%j.out
#   sbatch --job-name=dgap --partition=debug --nodes=N --time=00:30:00 --export=ALL,UNITS=u1:u2:...,STEP_MIN=12 dace_gap.sbatch
set -uo pipefail
cd /capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts-extracted/llr40Matrix/v2/dace/followup
NODES=($(scontrol show hostnames "$SLURM_JOB_NODELIST")); US=(${UNITS//:/ })
echo "job=$SLURM_JOB_ID nodes=${NODES[*]} units=${US[*]} start=$(date -Is)"
for i in "${!US[@]}"; do bash run_dace_gap.sh "${US[$i]}" "${NODES[$i]}" "$STEP_MIN" > "logs/unit.${US[$i]}.$SLURM_JOB_ID.log" 2>&1 & done
wait; echo "job=$SLURM_JOB_ID end=$(date -Is)"
