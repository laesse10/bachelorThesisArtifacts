#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --partition=debug
#SBATCH --nodes=10
#SBATCH --exclusive
#SBATCH --time=00:30:00
#SBATCH --output=logs/%x.%j.out
#SBATCH --error=logs/%x.%j.out
# Part B as chained 30-minute debug jobs (as LLR-40 v2's chain_debug.sbatch). Each node runs ONE unit
# (run_agents.sh: every submission of one kernel) at a time; units are claimed atomically, never two
# kernels on a node at once. A kernel is done when its unit ends cleanly (parts/<k>.done). A unit cut
# at the wall limit resumes in the next job: candidates are written atomically, so the next job redoes
# only the candidate that was cut. The job resubmits itself (afterany) while kernels remain, and stops
# after 2 consecutive jobs without a newly finished kernel.
set -uo pipefail
cd "${AL_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts-extracted/llr40Matrix/v2/agent_language}"
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
LIMIT=$((29 * 60)); MINLEFT=$((3 * 60))
NODES=($(scontrol show hostnames "$SLURM_JOB_NODELIST"))
CLAIMS=claims/$SLURM_JOB_ID; mkdir -p "$CLAIMS"
# to-do: kernels with candidates, not done, most candidates first (a long unit starts early)
"$PY" - > "$CLAIMS/todo.txt" <<'PYEOF'
import json, os
p = json.load(open("../agent_picks.json"))
ks = sorted((k for k, v in p.items() if v and not os.path.exists(f"parts/{k}.done")), key=lambda k: -len(p[k]))
print("\n".join(ks))
PYEOF
D0=$(ls parts/*.done 2>/dev/null | wc -l)
echo "job=$SLURM_JOB_ID nodes=${NODES[*]} start=$(date -Is) done=$D0 todo=$(wc -l < "$CLAIMS/todo.txt")"
worker() {
  local node=$1 j=$2
  while :; do
    local left=$((LIMIT - SECONDS)); (( left < MINLEFT )) && break
    local k=""
    while read -r cand; do
      [ -e "parts/$cand.done" ] && continue
      if mkdir "$CLAIMS/$cand" 2>/dev/null; then k=$cand; break; fi
    done < "$CLAIMS/todo.txt"
    [ -z "$k" ] && break
    local shard=$(( 400 + $(grep -n -x "$k" "$CLAIMS/todo.txt" | cut -d: -f1) ))
    echo "$(date -Is) $node START $k (budget $((left / 60)) min)"
    bash run_agents.sh "$k" "$node" $((left / 60)) "$shard" >> "logs/unit.$k.$SLURM_JOB_ID.log" 2>&1
    echo "$(date -Is) $node END   $k rc=$?"
  done
}
for j in "${!NODES[@]}"; do worker "${NODES[$j]}" "$j" & done
wait
D1=$(ls parts/*.done 2>/dev/null | wc -l)
LEFT=$("$PY" -c "import json,os;p=json.load(open('../agent_picks.json'));print(sum(1 for k,v in p.items() if v and not os.path.exists(f'parts/{k}.done')))")
echo "end=$(date -Is) done=$D0->$D1 kernels_left=$LEFT"
STALL=claims/stall_count; s=$(cat "$STALL" 2>/dev/null || echo 0)
if (( D1 > D0 )); then s=0; else s=$((s + 1)); fi; echo "$s" > "$STALL"
if (( LEFT > 0 )); then
  if (( s >= 2 )); then echo "STALLED: 2 jobs without a finished kernel; not resubmitting" | tee claims/STALLED
  else
    ok=0
    for try in 1 2 3 4 5 6; do
      if sbatch --dependency=afterany:$SLURM_JOB_ID agents_chain.sbatch; then ok=1; break; fi
      sleep 20
    done
    (( ok )) || echo "RESUBMIT_FAILED $(date -Is): $LEFT kernels left" | tee claims/RESUBMIT_FAILED
  fi
else
  echo "CHAIN COMPLETE $(date -Is)" | tee claims/COMPLETE
fi
