#!/bin/bash
#SBATCH --job-name=perf_comet
#SBATCH --account=g34
#SBATCH --partition=debug
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --gpus=1
#SBATCH --time=00:30:00
#SBATCH --output=perf_comet_%j.out

# CoMet 2-way CCC on GH200 with the CUTLASS INT4 tensor-core GEMM (--tc 6 --num_kernel 10),
# 24000 vectors x 40000 fields on one GPU. --checksum no: the CPU checksum is a verification
# aid that otherwise takes ~15x the GEMM's wall time; this build reproduces Quick_Start's
# small-case checksum 0-245201878478-801640733671948288 on the same --tc 6 path.
[ -d /user-environment/env ] || exec uenv run prgenv-gnu/25.6:v2 --view=default -- bash "$0" "$@"
set -u
ulimit -c 0
cd /capstor/scratch/cscs/lhulsbergen/perf_runs/comet
source ../common.sh
EXE=/capstor/scratch/cscs/lhulsbergen/comet_work/installs/install_release_nompi_daint/bin/genomics_metric
ARGS="--num_field 40000 --num_vector 24000 --num_proc_vector 1 --metric_type ccc --num_way 2
      --compute_method GPU --all2all yes --tc 6 --num_kernel 10 --checksum no --verbosity 1"
export OMP_NUM_THREADS=72

prof_nsys ccc2_tc6_k10 -- $EXE $ARGS
prof_cpu  ccc2_tc6_k10 99 cycles -- $EXE $ARGS

keep_results comet
echo "########## done $(date +%T)"
