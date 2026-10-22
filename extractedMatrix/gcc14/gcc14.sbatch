#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --exclusive
#SBATCH --uenv=prgenv-gnu/25.6:v2
#SBATCH --output=logs/%x.%j.out
#SBATCH --error=logs/%x.%j.out
# The extracted kernels with GCC 14.2 for C, C++ and Fortran: one kernel per exclusive node (run_gcc14.sh, copied
# from ../../llr40Matrix/v2/gcc14/ with the matrix's bench tree). The uenv is mounted WITHOUT a view, so the
# compilers stay /usr/bin/gcc-14, ~/bin/g++ and ~/bin/gfortran (GCC 13.3) and the bin*/ shims; only CoMet's
# cells put its OpenBLAS on PKG_CONFIG_PATH (units.txt), as in ../DEVIATIONS.md item 9.
#   sbatch --job-name=ext-gcc14 --partition=normal --nodes=8 --time=02:30:00 --export=ALL,GCC14_STEP_MIN=140 gcc14.sbatch
set -uo pipefail
cd "${GCC14_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts-extracted/extractedMatrix/gcc14}"
NODES=($(scontrol show hostnames "$SLURM_JOB_NODELIST"))
if [ -n "${GCC14_UNITS:-}" ]; then US=(${GCC14_UNITS//:/ }); else US=($(grep -v '^#' units.txt | awk -F'|' '{print $1}')); fi
echo "job=$SLURM_JOB_ID nodes=${NODES[*]} units=${#US[@]} start=$(date -Is)"
for i in "${!NODES[@]}"; do
  MINE=(); for j in "${!US[@]}"; do [ $((j % ${#NODES[@]})) -eq $i ] && MINE+=("${US[$j]}"); done
  [ ${#MINE[@]} -gt 0 ] && bash run_gcc14.sh "${NODES[$i]}" "$GCC14_STEP_MIN" $((500 + i)) "${MINE[@]}" \
    > "logs/node.${NODES[$i]}.${SLURM_JOB_ID}.log" 2>&1 &
done
wait
grep -h "GEOMETRY_FAILED" logs/node.*."${SLURM_JOB_ID}".log && echo "GEOMETRY CHECK FAILED on some node"
echo "job=$SLURM_JOB_ID end=$(date -Is)"
