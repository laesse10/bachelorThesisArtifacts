#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --partition=normal
#SBATCH --exclusive
#SBATCH --output=numba021/logs/%x.%j.out
#SBATCH --error=numba021/logs/%x.%j.out
# numba021: one exclusive node per unit, as extracted.sbatch. Units are KERNEL=REPRS pairs, ':'-separated,
# REPRS '+'-separated (sbatch --export splits on commas). Submit from extractedMatrix/:
#   sbatch --job-name=ext-numba021 --nodes=2 --time=01:30:00 \
#     --export=ALL,N021_UNITS=warpx_field_gather=numba+c:warpx_esirkepov_deposition=numba,EXT_STEP_MIN=60 numba021/numba021.sbatch
set -uo pipefail
cd "${EXT_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts-extracted/extractedMatrix}"
NODES=($(scontrol show hostnames "$SLURM_JOB_NODELIST"))
US=(${N021_UNITS//:/ })
echo "job=$SLURM_JOB_ID nodes=${NODES[*]} units=${US[*]} start=$(date -Is)"
[ "${#NODES[@]}" -ge "${#US[@]}" ] || { echo "need ${#US[@]} nodes"; exit 1; }
for i in "${!US[@]}"; do
  K=${US[$i]%%=*}; R=${US[$i]#*=}; R=${R//+/,}
  bash numba021/run_numba021.sh "$K" "${NODES[$i]}" "$EXT_STEP_MIN" $((120 + i)) "$R" \
    > "numba021/logs/unit.$K.${SLURM_JOB_ID}.log" 2>&1 &
done
wait
grep -h "GEOMETRY_FAILED" numba021/logs/unit.*."${SLURM_JOB_ID}".log && echo "GEOMETRY CHECK FAILED on some node"
echo "job=$SLURM_JOB_ID end=$(date -Is)"
