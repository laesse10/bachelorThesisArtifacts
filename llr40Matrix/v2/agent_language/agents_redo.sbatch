#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --partition=debug
#SBATCH --nodes=10
#SBATCH --exclusive
#SBATCH --time=00:30:00
#SBATCH --output=logs/%x.%j.out
#SBATCH --error=logs/%x.%j.out
# T72 redo pass over the kernels listed in $AL_REDO_KERNELS (':'-separated); kernel i runs on node i mod N,
# a node's kernels one after another. Each unit re-times only refused/missing default/T72 cells.
set -uo pipefail
cd "${AL_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts-extracted/llr40Matrix/v2/agent_language}"
NODES=($(scontrol show hostnames "$SLURM_JOB_NODELIST"))
KS=(${AL_REDO_KERNELS//:/ }); N=${#NODES[@]}
echo "job=$SLURM_JOB_ID nodes=${NODES[*]} kernels=${KS[*]} start=$(date -Is)"
for j in "${!NODES[@]}"; do
  ( for i in "${!KS[@]}"; do
      (( i % N == j )) || continue
      [ -e "parts/${KS[$i]}.redo_t72.done" ] && continue
      left=$(( 28 - SECONDS / 60 )); (( left < 4 )) && break
      bash run_agents_redo.sh "${KS[$i]}" "${NODES[$j]}" "$left" $(( 600 + i )) >> "logs/redo.${KS[$i]}.${SLURM_JOB_ID}.log" 2>&1
    done ) &
done
wait
echo "job=$SLURM_JOB_ID end=$(date -Is)"
