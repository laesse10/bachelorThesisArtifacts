#!/bin/bash -l
#SBATCH --job-name=llr40v2-chain
#SBATCH --account=g34
#SBATCH --partition=debug
#SBATCH --nodes=10
#SBATCH --exclusive
#SBATCH --time=00:30:00
#SBATCH --output=sweep_logs/chain.%j.out
#SBATCH --error=sweep_logs/chain.%j.err
#
# Chained debug-partition run of the whole v2 workload (main sweep, variants, opt reports).
# Each of the 10 nodes runs ONE unit at a time (chain_unit.sh pins the step with -w); a unit is
# claimed atomically, so no two kernels ever share a node. Cells are checkpointed one by one,
# so a unit cut off at the wall limit resumes in the next job. The job resubmits itself
# (afterany) while work remains, and stops after 2 consecutive jobs with no new rows.
set -uo pipefail
cd "${LLR40_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts/llr40Matrix/v2}"
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
LIMIT=$((29 * 60)); MINLEFT=$((4 * 60))
NODES=($(scontrol show hostnames "$SLURM_JOB_NODELIST"))
CLAIMS=chain_claims/$SLURM_JOB_ID; mkdir -p "$CLAIMS"
P0=$("$PY" chain_status.py --progress)
"$PY" chain_status.py --todo > "$CLAIMS/todo.tsv"
echo "job=$SLURM_JOB_ID nodes=${NODES[*]} start=$(date -Is) progress=$P0 todo=$(wc -l < "$CLAIMS/todo.tsv")"

# --- geometry guard: refuse to measure unless the build child sees socket 0 (>1 core) and the
# harness resolves -ftree-parallelize-loops with n > 1, as in v1 (see discarded/4969249/).
for n in "${NODES[@]}"; do bash chain_unit.sh PROBE - "$n" 2 > "$CLAIMS/probe.$n" 2>&1 & done; wait
cat "$CLAIMS"/probe.* | grep -E "^PROBE" | tee "$CLAIMS/probe.summary"
if [ "$(grep -c '^PROBE' "$CLAIMS/probe.summary")" -ne "${#NODES[@]}" ] || \
   awk '$3 < 2 || $4 < 2 {bad=1} END {exit !bad}' "$CLAIMS/probe.summary"; then
  echo "GEOMETRY_FAILED: not measuring, not resubmitting" | tee chain_claims/GEOMETRY_FAILED
  cat "$CLAIMS"/probe.*; exit 1
fi

worker() {
  local node=$1
  while :; do
    local left=$((LIMIT - SECONDS))
    (( left < MINLEFT )) && break
    local unit=""
    while IFS=$'\t' read -r kind k; do
      if mkdir "$CLAIMS/${kind}.${k}" 2>/dev/null; then unit="$kind $k"; break; fi
    done < "$CLAIMS/todo.tsv"
    [ -z "$unit" ] && break
    echo "$(date -Is) $node START $unit (budget $((left / 60)) min)"
    bash chain_unit.sh $unit "$node" $((left / 60)) >> "sweep_logs/chain.$SLURM_JOB_ID.$node.log" 2>&1
    echo "$(date -Is) $node END   $unit rc=$?"
  done
}
for n in "${NODES[@]}"; do worker "$n" & done
wait

P1=$("$PY" chain_status.py --progress)
LEFT=$("$PY" chain_status.py --todo | wc -l)
echo "end=$(date -Is) progress=$P0->$P1 units_left=$LEFT"
STALL=chain_claims/stall_count; s=$(cat "$STALL" 2>/dev/null || echo 0)
if (( P1 > P0 )); then s=0; else s=$((s + 1)); fi; echo "$s" > "$STALL"
if (( LEFT > 0 )); then
  if (( s >= 2 )); then echo "STALLED: 2 jobs without progress; not resubmitting" | tee chain_claims/STALLED
  else sbatch --dependency=afterany:$SLURM_JOB_ID chain_debug.sbatch; fi
else
  echo "CHAIN COMPLETE $(date -Is)" | tee chain_claims/COMPLETE
fi
