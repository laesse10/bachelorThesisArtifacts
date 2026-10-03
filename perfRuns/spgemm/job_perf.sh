#!/bin/bash
#SBATCH --job-name=perf_spgemm
#SBATCH --account=g34
#SBATCH --partition=debug
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --gpus=1
#SBATCH --time=00:30:00
#SBATCH --output=perf_spgemm_%j.out

# SpBench cuBool boolean SpGEMM (nsparse row-binned hash accumulator), C = A * A on five
# SuiteSparse graphs, 10 iterations each. cuBool rebuilt 2026-10-02 from upstream 81573de
# plus the CUDA 12.9 / sm_90 build fixes and the static pwarp shuffle mask.
[ -d /user-environment/env ] || exec uenv run prgenv-gnu/25.6:v2 --view=default -- bash "$0" "$@"
set -u
cd /capstor/scratch/cscs/lhulsbergen/perf_runs/spgemm
source ../common.sh
EXE=/capstor/scratch/cscs/lhulsbergen/spbench/build/cubool_mult

prof_nsys cubool_mult5 -- $EXE spgemm5_cfg.txt
prof_cpu  cubool_mult5 99 cycles -- $EXE spgemm5_cfg.txt
cp Summary-Cubool-Multiply.txt summary_cubool_mult5.txt 2>/dev/null

keep_results spgemm
echo "########## done $(date +%T)"
