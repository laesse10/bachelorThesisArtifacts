#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --exclusive
#SBATCH --output=logs/%x.%j.out
#SBATCH --error=logs/%x.%j.out
# The units of units.txt (or $CLANG_UNITS, colon-separated), dealt round-robin over the job's nodes; each node
# runs its units one after the other (run_clang.sh).
#   sbatch --job-name=clang --partition=normal --nodes=10 --time=01:00:00 --export=ALL,CLANG_STEP_MIN=55 clang.sbatch
set -uo pipefail
cd "${CLANG_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts-clang/llr40Matrix/v2/clang}"
NODES=($(scontrol show hostnames "$SLURM_JOB_NODELIST"))
if [ -n "${CLANG_UNITS:-}" ]; then US=(${CLANG_UNITS//:/ }); else US=($(grep -v '^#' units.txt | awk -F'|' '{print $1}')); fi
echo "job=$SLURM_JOB_ID nodes=${NODES[*]} units=${#US[@]} start=$(date -Is)"
for i in "${!NODES[@]}"; do
  MINE=(); for j in "${!US[@]}"; do [ $((j % ${#NODES[@]})) -eq $i ] && MINE+=("${US[$j]}"); done
  [ ${#MINE[@]} -gt 0 ] && bash run_clang.sh "${NODES[$i]}" "$CLANG_STEP_MIN" $((300 + i)) "${MINE[@]}" \
    > "logs/node.${NODES[$i]}.${SLURM_JOB_ID}.log" 2>&1 &
done
wait
grep -h "GEOMETRY_FAILED" logs/node.*."${SLURM_JOB_ID}".log && echo "GEOMETRY CHECK FAILED on some node"
echo "job=$SLURM_JOB_ID end=$(date -Is)"
