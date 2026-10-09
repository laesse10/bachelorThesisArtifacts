#!/bin/bash
# One follow-up unit on ONE named node, with the extracted matrix's step geometry (run_extracted.sh):
# srun --nodes=1 --ntasks=1 -w NODE --exclusive --cpu-bind=cores --hint=nomultithread; sweep_gap.py binds
# every timed child to NUMA node 0. Compile-only opt reports of the new builds run AFTER the timed step.
# usage: run_gap.sh UNIT NODE MINUTES DB_SHARD        (run from extractedMatrix/followup/)
set -uo pipefail
U=$1; NODE=$2; MIN=$3; SHARD=$4
LINE=$(grep -E "^$U +\|" units.txt) || { echo "no unit $U"; exit 2; }
IFS='|' read -r _ K WT RR CELLS EXTRA <<< "$LINE"   # EXTRA (optional 6th field): more sweep_gap.py options
K=$(echo $K); WT=$(echo $WT); RR=$(echo $RR)
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
BENCH="/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-$WT"
mkdir -p parts logs
step() {
  local CMD=(srun --nodes=1 --ntasks=1 -w "$NODE" --exclusive --cpu-bind=cores --hint=nomultithread --time="$MIN" "$@")
  echo "SRUN[$U]: ${CMD[*]}"
  echo "$(date -Is) job=${SLURM_JOB_ID} partition=${SLURM_JOB_PARTITION} node=$NODE ${CMD[*]}" >> srun_lines.txt
  "${CMD[@]}"
}
echo "unit $U kernel=$K bench=$BENCH node=$NODE start=$(date -Is) gfortran=$(command -v gfortran) bin14=$(readlink bin14/gfortran)"
P=$(step env LLR40_BENCH="$BENCH" numactl --cpunodebind=0 --membind=0 "$PY" -c \
  "import os,re,sys;sys.path[:0]=[os.environ['LLR40_BENCH'],os.environ['LLR40_BENCH']+'/hpcagent_bench/numpy_translators/src'];from hpcagent_bench import languages as L;f=L.baseline_flags('fortran');m=re.search(r'parallelize-loops=(\d+)',f);print('PROBE',os.uname().nodename,len(os.sched_getaffinity(0)),m.group(1) if m else 0)" | grep '^PROBE')
echo "$P"
[ "$(echo "$P" | awk '{print $3, $4}')" = "1 1" ] || { echo "GEOMETRY_FAILED [$U]: $P"; exit 3; }
step env LLR40_BENCH="$BENCH" "$PY" sweep_gap.py --kernel "$K" --cells $CELLS --rounds 3 --perf --record-reps "$RR" \
  --out "parts/$U.csv" --scratch "scratch/$U" --db-shard "$SHARD" $EXTRA
# opt reports / numba asm of the cells that are new here (the default cells have theirs in ../opt_reports
# or in llr40Matrix/v2/opt_reports)
E=emitted_sources/$K
OPT=(env LLR40_BENCH="$BENCH" numactl --cpunodebind=0 --membind=0 "$PY" ../opt_reports_extracted.py --index "parts/optrep.$U.csv")
for C in $CELLS; do
  case "$C" in
    fortran14=*) step env PATH="$PWD/bin14:$PATH" "${OPT[@]}" --out-dir opt_reports/gfortran14 --variant gfortran14 "$K:fortran:fortran:$E/${K}_fp64.f90"
                 step "${OPT[@]}" --out-dir opt_reports/gfortran13 --variant gfortran13 "$K:fortran:fortran:$E/${K}_fp64.f90"
                 step "${OPT[@]}" --out-dir opt_reports/gcc14 --variant gcc14 "$K:c:c:$E/${K}_fp64.c" ;;
    cshift=*)    S=$(echo "$C" | sed -E 's/.*:src=([^:]*).*/\1/')
                 step "${OPT[@]}" --out-dir opt_reports/cshift --variant cshift "$K:c:c:$S" ;;
    nb*=numba:src=*) S=$(echo "$C" | sed -E 's/.*:src=([^:]*).*/\1/')
                 step "${OPT[@]}" --out-dir "opt_reports/${C%%=*}" "$K:numba_hand:python:$S" ;;
  esac
done
# (the esirkN2 address probe is probe.sbatch: run inside this unit in job 4998121 it failed on relative paths)
echo "unit $U node=$NODE end=$(date -Is)"
