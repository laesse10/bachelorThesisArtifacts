#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --partition=normal
#SBATCH --exclusive
#SBATCH --output=logs/%x.%j.out
#SBATCH --error=logs/%x.%j.out
#
# One follow-up experiment: one exclusive node per unit (run_unit.sh); a node runs its units one
# after another, so no two timed kernels ever share a node at the same time. Submit from followup/ with
#   sbatch --job-name=fu-<exp> --nodes=<#kernels> --time=<hh:mm:ss> \
#          --export=ALL,FU_EXP=<EXP>,FU_KERNELS=<k1:k2:...>,FU_STEP_MIN=<minutes> followup.sbatch
set -uo pipefail
cd "${LLR40_FOLLOWUP:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts-followup/llr40Matrix/v2/followup}"
NODES=($(scontrol show hostnames "$SLURM_JOB_NODELIST"))
KS=(${FU_KERNELS//:/ })   # ":"-separated: sbatch --export splits on commas
echo "job=$SLURM_JOB_ID exp=$FU_EXP nodes=${NODES[*]} kernels=${KS[*]} start=$(date -Is)"
# kernel i runs on node i mod N; a node with several kernels runs them ONE AFTER ANOTHER
N=${#NODES[@]}
for j in "${!NODES[@]}"; do
  ( for i in "${!KS[@]}"; do
      (( i % N == j )) || continue
      bash run_unit.sh "$FU_EXP" "${KS[$i]}" "${NODES[$j]}" "$FU_STEP_MIN" \
        > "logs/${FU_EXP}.${KS[$i]}.${SLURM_JOB_ID}.log" 2>&1
    done ) &
done
wait
grep -h "GEOMETRY_FAILED" logs/*."${SLURM_JOB_ID}".log && echo "GEOMETRY CHECK FAILED on some node"
echo "job=$SLURM_JOB_ID end=$(date -Is)"
