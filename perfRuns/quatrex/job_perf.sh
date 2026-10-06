#!/bin/bash
#SBATCH --job-name=perf_quatrex
#SBATCH --account=g34
#SBATCH --partition=debug
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --gpus=1
#SBATCH --time=00:30:00
#SBATCH --output=perf_quatrex_%j.out

# QuaTrEx carbon-nanotube GW example (examples/w90/carbon-nanotube/gw), 4 SCBA iterations.
#   gpu: default CuPy backend -- nsys trace plus perf of the host side
#   cpu: QTX_ARRAY_MODULE=numpy, OPENBLAS_NUM_THREADS=1 -- the CPU-side profile; without
#        the thread pin OpenBLAS's idle worker pool spins and dominates perf
[ -d /user-environment/env ] || exec uenv run prgenv-gnu/25.6:v2 --view=default -- bash "$0" "$@"
set -u
cd /capstor/scratch/cscs/lhulsbergen/perf_runs/quatrex
source ../common.sh
QTX=/capstor/scratch/cscs/lhulsbergen/quatrex_venv/bin/quatrex
export PYTHONPATH=/capstor/scratch/cscs/lhulsbergen/quatrex_work/stubs   # gmsh stub
export OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=1 MKL_NUM_THREADS=1 NUMBA_NUM_THREADS=1

QTX_ARRAY_MODULE=cupy  prof_nsys cnt_gw_gpu -- $QTX run cnt_gw_gpu
QTX_ARRAY_MODULE=cupy  prof_cpu  cnt_gw_gpu 999 cycles -- $QTX run cnt_gw_gpu
QTX_ARRAY_MODULE=numpy prof_cpu  cnt_gw_cpu 999 cycles -- $QTX run cnt_gw_cpu
grep -h "Leaving SCBA" run_*.log

keep_results quatrex
echo "########## done $(date +%T)"
