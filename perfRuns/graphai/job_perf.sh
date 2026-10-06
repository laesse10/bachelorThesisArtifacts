#!/bin/bash
#SBATCH --job-name=perf_graphai
#SBATCH --account=g34
#SBATCH --partition=normal
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --gpus=1
#SBATCH --time=01:00:00
#SBATCH --output=perf_graphai_%j.out

# GraphAIBench triangle counting (tc_gpu_base, warp-centric edge-parallel) on com-Orkut.
# TC_REPS repeats the count so the profile is the steady state, not graph loading
# (a single run is ~4% GPU-busy). Expected total_num_triangles = 627584181.
[ -d /user-environment/env ] || exec uenv run prgenv-gnu/25.6:v2 --view=default -- bash "$0" "$@"
set -u
cd /capstor/scratch/cscs/lhulsbergen/perf_runs/graphai
source ../common.sh
G=/capstor/scratch/cscs/lhulsbergen/graphaibench
export OMP_NUM_THREADS=72
export TC_REPS=100

prof_nsys tc_orkut -- $G/bin/tc_gpu_base $G/inputs/orkut/graph
prof_cpu  tc_orkut 99 cycles -- $G/bin/tc_gpu_base $G/inputs/orkut/graph
grep -h total_num_triangles run_*.log

keep_results graphai
echo "########## done $(date +%T)"
