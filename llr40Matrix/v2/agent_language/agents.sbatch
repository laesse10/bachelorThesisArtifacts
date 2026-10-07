#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --partition=normal
#SBATCH --exclusive
#SBATCH --output=logs/%x.%j.out
#SBATCH --error=logs/%x.%j.out
# Part B: kernel i runs on node i mod N; a node with several kernels runs them ONE AFTER ANOTHER.
#   sbatch --job-name=al-sweep --nodes=N --time=hh:mm:ss --export=ALL,AL_KERNELS=k1:k2:...,AL_STEP_MIN=m,AL_SHARD0=200 agents.sbatch
set -uo pipefail
cd "${AL_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts-extracted/llr40Matrix/v2/agent_language}"
NODES=($(scontrol show hostnames "$SLURM_JOB_NODELIST"))
KS=(${AL_KERNELS//:/ }); N=${#NODES[@]}
echo "job=$SLURM_JOB_ID nodes=${NODES[*]} kernels=${KS[*]} start=$(date -Is)"
for j in "${!NODES[@]}"; do
  ( for i in "${!KS[@]}"; do
      (( i % N == j )) || continue
      bash run_agents.sh "${KS[$i]}" "${NODES[$j]}" "$AL_STEP_MIN" $(( ${AL_SHARD0:-200} + i )) \
        > "logs/unit.${KS[$i]}.${SLURM_JOB_ID}.log" 2>&1
    done ) &
done
wait
grep -h "GEOMETRY_FAILED" logs/unit.*."${SLURM_JOB_ID}".log && echo "GEOMETRY CHECK FAILED on some node"
echo "job=$SLURM_JOB_ID end=$(date -Is)"
