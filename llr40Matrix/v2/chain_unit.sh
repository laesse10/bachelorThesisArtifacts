#!/bin/bash
# One unit of the debug chain on ONE named node (one timed kernel per node, never two).
# Same srun flags as sweep_array.sbatch; -w pins the step to its node, --time caps it to what is
# left of the 30-min debug allocation (a cell killed there left no row and is redone next job).
# usage: chain_unit.sh KIND KERNEL NODE MINUTES
set -uo pipefail
KIND=$1; KERNEL=$2; NODE=$3; MIN=$4
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
WT=/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2
mkdir -p results_parts agent_attempts_parts variant_parts variant_attempts_parts
step() {  # $@ = command run on the node
  # v1 geometry: ONE task with ONE cpu (v1's `srun --exclusive --cpu-bind=cores --hint=nomultithread`
  # in a 1-node job). The step cgroup holds one core, so the timed child runs on it and the harness
  # resolves -ftree-parallelize-loops=1, exactly as v1 did and recorded. (A whole-node step gives
  # n=72 and parloops ON -- supplementary_wholenode/.) -w keeps one step per node.
  local CMD=(srun --nodes=1 --ntasks=1 -w "$NODE" --exclusive
             --cpu-bind=cores --hint=nomultithread --time="$MIN" "$@")
  echo "SRUN[$KIND $KERNEL]: ${CMD[*]}"
  echo "$(date -Is) job=${SLURM_JOB_ID} partition=${SLURM_JOB_PARTITION} node=$NODE ${CMD[*]}" >> srun_lines.txt
  "${CMD[@]}"
}
variant() {  # $1 worktree $2 preset $3 label $4 flags_variant $5 reprs $6 tag
  step env LLR40_BENCH="$1" "$PY" sweep_variant.py --kernels "$KERNEL" --preset "$2" --preset-label="$3" \
    --flags-variant="$4" --reprs "$5" --warmup 5 --reps 30 \
    --out "variant_parts/${6}.${KERNEL}.csv" --scratch "scratch/variants/${6}.${KERNEL}" \
    --attempts-out "variant_attempts_parts/${6}.${KERNEL}.csv" \
    --attempt-logs agent_attempt_logs_variants --emitted-dir "emitted_sources_variants/${6}"
}
case "$KIND" in
  MAIN)  step env LLR40_BENCH="$WT" "$PY" sweep.py --kernels "$KERNEL" --preset M --warmup 5 --reps 30 \
           --out "results_parts/${KERNEL}.csv" --scratch "scratch/${KERNEL}" \
           --attempts-out "agent_attempts_parts/${KERNEL}.csv" --attempt-logs agent_attempt_logs ;;
  F)     variant "$WT-F" S F "" c,cpp,fortran,numba,c_reference,agent presetF ;;
  FPOFF) variant "$WT-fpoff" M M "-ffp-contract=off" c,cpp,fortran,c_reference fpoff ;;
  AB)    i=$(grep -n -P "^AB\t${KERNEL}$" variants/jobs.tsv | cut -d: -f1)   # order alternates by line
         if (( i % 2 )); then
           variant "$WT-abwith" M M "with-ftree-parallelize-loops" fortran abwith
           variant "$WT-nopar"  M M "without-ftree-parallelize-loops" fortran abwithout
         else
           variant "$WT-nopar"  M M "without-ftree-parallelize-loops" fortran abwithout
           variant "$WT-abwith" M M "with-ftree-parallelize-loops" fortran abwith
         fi ;;
  PROBE) # geometry check run by chain_debug.sbatch before any unit: what the build child sees
         step env LLR40_BENCH="$WT" numactl --cpunodebind=0 --membind=0 "$PY" -c \
           "import os,re,sys;sys.path[:0]=[os.environ['LLR40_BENCH'],os.environ['LLR40_BENCH']+'/hpcagent_bench/numpy_translators/src'];from hpcagent_bench import languages as L;f=L.baseline_flags('fortran');m=re.search(r'parallelize-loops=(\d+)',f);print('PROBE',os.uname().nodename,len(os.sched_getaffinity(0)),m.group(1) if m else 0)" ;;
  OPTREP) "$PY" merge_results.py
          step env LLR40_BENCH="$WT-aux" numactl --cpunodebind=0 --membind=0 "$PY" opt_reports.py ;;
esac
