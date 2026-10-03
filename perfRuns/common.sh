# Shared helpers for the perf re-runs (sourced by each app's job script).
# Same perf/flamegraph recipe as the WarpX tutorial01_perf job; nsys added for GPU apps.
#
#   prof_cpu TAG FREQ EVENT -- cmd args...   perf record + stdio reports + flamegraph
#   prof_nsys TAG -- cmd args...             nsys trace + kernel/API summaries
#   keep_results APP [DIR]                   copy the results to the g34 project store

STORE=/capstor/store/cscs/userlab/g34/lhulsbergen/perf_runs
FLAME=$HOME/FlameGraph

prof_cpu() {
  local tag=$1 freq=$2 event=$3; shift 4
  echo "########## perf $tag  $(date +%T)  node=$(hostname)  job=${SLURM_JOB_ID:-none}"
  echo "cmd: $*"
  perf record -F "$freq" --call-graph dwarf,4096 -e "$event" -o "perf_${tag}.data" -- "$@" \
    2>&1 | tee "run_${tag}.log"
  perf report --stdio --no-children -g none --no-inline --percent-limit 0.5 --sort=dso,symbol \
    -i "perf_${tag}.data" 2>/dev/null > "report_${tag}.self.txt"
  perf report --stdio --children --no-inline --percent-limit 0.5 \
    -i "perf_${tag}.data" 2>/dev/null > "report_${tag}.tree.txt"
  perf script --no-inline -i "perf_${tag}.data" 2>/dev/null \
    | "$FLAME/stackcollapse-perf.pl" > "stacks_${tag}.folded"
  "$FLAME/flamegraph.pl" --width 1600 --title "$tag (job ${SLURM_JOB_ID:-none})" \
    "stacks_${tag}.folded" > "flame_${tag}.svg"
  echo "----- $tag: self time top 25 -----"
  grep -m1 '^# Samples' "report_${tag}.self.txt"
  grep -E '^ +[0-9]' "report_${tag}.self.txt" | head -25
}

prof_nsys() {
  local tag=$1; shift 2
  echo "########## nsys $tag  $(date +%T)  node=$(hostname)  job=${SLURM_JOB_ID:-none}"
  echo "cmd: $*"
  nsys profile --force-overwrite=true --trace=cuda,nvtx,osrt --sample=none \
    -o "nsys_${tag}" "$@" 2>&1 | tee "run_nsys_${tag}.log"
  nsys stats --force-export=true --report cuda_gpu_kern_sum,cuda_gpu_mem_time_sum,cuda_api_sum \
    --format csv --output "nsys_${tag}" "nsys_${tag}.nsys-rep" > /dev/null 2>&1
  nsys stats --force-export=true --report cuda_gpu_kern_sum "nsys_${tag}.nsys-rep" 2>/dev/null \
    | tee "nsys_${tag}.kern_sum.txt" | head -30
}

keep_results() {
  local app=$1 dest
  dest="$STORE/$app/${2:-$(date +%Y%m%d)_job${SLURM_JOB_ID:-none}}"
  mkdir -p "$dest"
  cp -a ./*.txt ./*.svg ./*.folded ./*.log ./*.csv ./*.nsys-rep ./*.sh ../common.sh "$dest"/ 2>/dev/null
  # The folded stacks carry everything the flamegraph needs; raw perf.data (only readable
  # against the binaries it was recorded with) is kept when it is under 2 GB
  find . -maxdepth 1 -name 'perf_*.data' -size -2G -exec cp -a {} "$dest"/ \;
  echo "Results copied to $dest"
}
