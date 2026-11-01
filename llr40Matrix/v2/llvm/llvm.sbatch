#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --exclusive
#SBATCH --output=logs/%x.%j.out
#SBATCH --error=logs/%x.%j.out
# The units of units.txt (or $LLVM_UNITS, colon-separated), dealt round-robin over the job's nodes; each node
# runs its units one after the other (run_llvm.sh).
#   sbatch --job-name=llvm --partition=normal --nodes=10 --time=01:00:00 --export=ALL,LLVM_STEP_MIN=55 llvm.sbatch
set -uo pipefail
cd "${LLVM_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts-clang/llr40Matrix/v2/llvm}"
NODES=($(scontrol show hostnames "$SLURM_JOB_NODELIST"))
if [ -n "${LLVM_UNITS:-}" ]; then US=(${LLVM_UNITS//:/ }); else US=($(grep -v '^#' units.txt | awk -F'|' '{print $1}')); fi
echo "job=$SLURM_JOB_ID nodes=${NODES[*]} units=${#US[@]} start=$(date -Is)"
for i in "${!NODES[@]}"; do
  MINE=(); for j in "${!US[@]}"; do [ $((j % ${#NODES[@]})) -eq $i ] && MINE+=("${US[$j]}"); done
  [ ${#MINE[@]} -gt 0 ] && bash run_llvm.sh "${NODES[$i]}" "$LLVM_STEP_MIN" $((500 + i)) "${MINE[@]}" \
    > "logs/node.${NODES[$i]}.${SLURM_JOB_ID}.log" 2>&1 &
done
wait
grep -h "GEOMETRY_FAILED" logs/node.*."${SLURM_JOB_ID}".log && echo "GEOMETRY CHECK FAILED on some node"
echo "job=$SLURM_JOB_ID end=$(date -Is)"
