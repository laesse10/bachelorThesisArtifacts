#!/bin/bash
# One follow-up unit on ONE named node (one timed kernel per node, never two), v2's chain_unit.sh
# geometry: srun --nodes=1 --ntasks=1 -w NODE --exclusive --cpu-bind=cores --hint=nomultithread,
# i.e. one task holding one core; the timed child is bound to NUMA node 0 by sweep_followup.py.
# Compile-only opt-report steps run AFTER the timed steps on the same node, never beside them.
# usage: run_unit.sh EXP KERNEL NODE MINUTES      (run from followup/)
set -uo pipefail
EXP=$1; K=$2; NODE=$3; MIN=$4
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
WT=/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2-fu          # 26a4f0cf, no patch
WTF=/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2-fu-finite  # 26a4f0cf + variants/finite_math_only.patch
LLR=hpcagent_bench/benchmarks/loop_level_reasoning
mkdir -p parts logs/attempts
step() {  # $@ = command run on the node
  local CMD=(srun --nodes=1 --ntasks=1 -w "$NODE" --exclusive
             --cpu-bind=cores --hint=nomultithread --time="$MIN" "$@")
  echo "SRUN[$EXP $K]: ${CMD[*]}"
  echo "$(date -Is) job=${SLURM_JOB_ID} partition=${SLURM_JOB_PARTITION} node=$NODE ${CMD[*]}" >> srun_lines.txt
  "${CMD[@]}"
}
probe() {  # what the build child sees: must be 1 core and -ftree-parallelize-loops=1, as in v2
  local out
  out=$(step env LLR40_BENCH="$1" numactl --cpunodebind=0 --membind=0 "$PY" -c \
    "import os,re,sys;sys.path[:0]=[os.environ['LLR40_BENCH'],os.environ['LLR40_BENCH']+'/hpcagent_bench/numpy_translators/src'];from hpcagent_bench import languages as L;f=L.baseline_flags('fortran');m=re.search(r'parallelize-loops=(\d+)',f);print('PROBE',os.uname().nodename,len(os.sched_getaffinity(0)),m.group(1) if m else 0)" | grep '^PROBE')
  echo "$out"
  [ "$(echo "$out" | awk '{print $3, $4}')" = "1 1" ] || { echo "GEOMETRY_FAILED [$EXP $K]: $out"; exit 3; }
}
sweep() {  # $1 worktree, $2 out-stem, rest: extra sweep_followup.py args
  local wt=$1 stem=$2; shift 2
  mkdir -p "parts/$(dirname "$stem")"
  step env LLR40_BENCH="$wt" "$PY" sweep_followup.py --kernels "$K" --preset M --warmup 5 --reps 30 \
    --out "parts/${stem}.csv" --scratch "scratch/${stem}" --attempts-out "parts/${stem}.attempts.csv" \
    --attempt-logs logs/attempts "$@"
}
optrep() {  # $1 worktree, $2 out subdir, $3 variant label, rest: KERNEL:REPR:LANG:SOURCE cells
  local wt=$1 sub=$2 var=$3; shift 3
  step env LLR40_BENCH="$wt" numactl --cpunodebind=0 --membind=0 "$PY" opt_reports_followup.py \
    --out-dir "opt_reports/$sub" --index "parts/$sub/optrep.${K}.${var:-base}.csv" --variant "$var" "$@"
}
agent_sha() {  # v2's chosen agent candidate for $K (results.csv notes, git 632952d)
  grep -m1 "^$K," v2_agent_cells.csv | cut -d, -f2
}
regmem_optrep() {  # unchanged and variant cells in SEPARATE subdirs (they share <k>/<repr>/ names)
  local V=variants/src
  if [ "$K" = versioned_distance_update ]; then
    local A
    A=$("$PY" -c "import json,sys;p=json.load(open('agent_picks.json'));print(next(c['path'] for c in p['$K'] if c['sha256'].startswith('$(agent_sha)')))")
    optrep "$WT" regmem/unchanged "" "$K:c:c:emitted_sources/regmem/$K/${K}_fp64.c" "$K:agent:fortran:$A"
    optrep "$WT" regmem/vdu_k1_scalar vdu_k1_scalar "$K:c:c:$V/${K}_fp64.k1scalar.c"
  else
    optrep "$WT" regmem/unchanged "" "$K:numba:python:emitted_sources/regmem/$K/${K}_numba_np.py" \
      "$K:c:c:emitted_sources/regmem/$K/${K}_fp64.c"
    optrep "$WT" regmem/wf_west_scalar wf_west_scalar "$K:numba:python:$V/${K}_numba_np.westscalar.py"
  fi
}
echo "unit $EXP $K node=$NODE start=$(date -Is) g++=$(command -v g++) gfortran=$(command -v gfortran)"
case "$EXP" in
  MINMAX)   # experiment 1: harness compile line + -ffinite-math-only
    probe "$WTF"
    sweep "$WTF" "minmax/$K" --reprs c,cpp,fortran,c_reference \
      --flags-variant=-ffinite-math-only --variant ffinite-math-only --emitted-dir emitted_sources/minmax
    E=emitted_sources/minmax/$K
    optrep "$WTF" minmax ffinite-math-only "$K:c:c:$E/${K}_fp64.c" "$K:cpp:cpp:$E/${K}_fp64.cpp" \
      "$K:fortran:fortran:$E/${K}_fp64.f90" "$K:c_reference:c:$WTF/$LLR/$K/${K}_reference.c" ;;
  S2710)    # experiment 2: x[0] hoisted (A: c; B: cpp and c_reference)
    probe "$WT"
    V=variants/src
    sweep "$WT" "s2710/$K" --reprs c,cpp,c_reference --variant hoist_x0 --emitted-dir emitted_sources/s2710 \
      --override c=$V/${K}_fp64.hoist.c --override cpp=$V/${K}_fp64.hoist.cpp \
      --override c_reference=$V/${K}_reference.hoist.c
    optrep "$WT" s2710 hoist_x0 "$K:c:c:$V/${K}_fp64.hoist.c" "$K:cpp:cpp:$V/${K}_fp64.hoist.cpp" \
      "$K:c_reference:c:$V/${K}_reference.hoist.c" ;;
  UNSTABLE) # experiment 3: 6 interleaved rounds, column order rotated by one each round
    probe "$WT"
    COLS=(c c_reference cpp fortran numba agent)
    for r in 1 2 3 4 5 6; do
      ORDER=$(for i in 0 1 2 3 4 5; do echo -n "${COLS[$(( (i + r - 1) % 6 ))]},"; done); ORDER=${ORDER%,}
      echo "round $r order $ORDER $(date -Is)"
      sweep "$WT" "unstable/$K.round$r" --reprs "$ORDER" --ordered --round "$r" --variant interleaved_rerun \
        --agent-sha "$(agent_sha)" --perf --vmstat --emitted-dir emitted_sources/unstable
    done ;;
  REGMEM)   # experiment 4: unchanged cells and the one variant, perf over the timed reps
    probe "$WT"
    V=variants/src
    if [ "$K" = versioned_distance_update ]; then
      sweep "$WT" "regmem/$K.base" --reprs c,agent --agent-sha "$(agent_sha)" --perf --emitted-dir emitted_sources/regmem
      sweep "$WT" "regmem/$K.k1scalar" --reprs c --variant vdu_k1_scalar --perf --emitted-dir emitted_sources/regmem \
        --override c=$V/${K}_fp64.k1scalar.c
      regmem_optrep
    else
      sweep "$WT" "regmem/$K.base" --reprs numba,c --perf --emitted-dir emitted_sources/regmem
      sweep "$WT" "regmem/$K.westscalar" --reprs numba --variant wf_west_scalar --perf --emitted-dir emitted_sources/regmem \
        --override numba=$V/${K}_numba_np.westscalar.py
      regmem_optrep
    fi ;;
  REGMEM_OPTREP)  # experiment 4 opt reports only (compile-only; re-run after the base/variant overwrite)
    probe "$WT"
    regmem_optrep ;;
  *) echo "unknown EXP $EXP"; exit 2 ;;
esac
echo "unit $EXP $K node=$NODE end=$(date -Is)"
