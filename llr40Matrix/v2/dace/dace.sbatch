#!/bin/bash
# DaCe old / legacy / new CPU codegen: emit (+ compile-only reports) or timing, on daint.
#   sbatch --nodes=1  --time=00:30:00 --export=ALL,STAGE=emit  dace.sbatch   (emit, then vec)
#   sbatch --nodes=1  --time=00:30:00 --export=ALL,STAGE=vec   dace.sbatch   (compile-only reports only)
#   sbatch --nodes=N  --time=HH:MM:SS --export=ALL,STAGE=time,KERNELS=k1:k2:...  dace.sbatch
# Timing uses v2's geometry: one kernel per exclusive node, its cells one after the other, each cell a
# single task on a single core (--cpu-bind=cores --hint=nomultithread) bound to NUMA node 0, so the
# harness sees one CPU (OMP threads 1, -ftree-parallelize-loops=1), as in v2.
#SBATCH --job-name=dace-cg
#SBATCH --account=g34
#SBATCH --partition=normal
#SBATCH --exclusive
#SBATCH --output=logs/%x.%j.out
set -uo pipefail
cd "${SLURM_SUBMIT_DIR}"
ROOT=/capstor/scratch/cscs/lhulsbergen
export DACE_BENCH=${DACE_BENCH:-$ROOT/HPCAgent-Bench-dace}            # 5cfcb4f2
PY=${DACE_PYTHON:-$ROOT/venv_dace5cf/bin/python}                       # spcl/dace@dea0b39c
export PATH="$(dirname "$PY"):$PATH"    # the venv's ninja, so DaCe's command cache can replay builds
OUT=${DACE_OUT:-$ROOT/llr40_dace}
LLR40=$(cat ../roster40.txt)
BONSALL="tsvc_2_s313 tsvc_2_vdotr tsvc_2_s453 tsvc_2_s314"
mkdir -p logs "$OUT"
echo "stage=$STAGE job=$SLURM_JOB_ID nodes=$SLURM_JOB_NODELIST bench=$(git -C "$DACE_BENCH" rev-parse HEAD) dace=$($PY -c 'import dace,subprocess,os;print(subprocess.run(["git","-C",os.path.dirname(os.path.dirname(dace.__file__)),"rev-parse","HEAD"],capture_output=True,text=True).stdout.strip() or dace.__version__)')"

vec_reports() {
  srun --nodes=1 --ntasks=1 --cpus-per-task=288 "$PY" vec_reports.py --emit-root "$OUT/emit" --emit-log "$OUT/emit.jsonl" \
    --dace-include "$($PY -c 'import dace,os;print(os.path.join(os.path.dirname(dace.__file__),"runtime","include"))')" \
    --out "$OUT/vec" --jobs 96
}

case "$STAGE" in
  emit)   # every cell's sources (2 pipelines x 3 codegen settings), then the compile-only reports
    # Two phases: the first cell of a kernel writes its generated @dace.program module and the cached
    # parse into the benchmark tree, so the kernel's other five cells only start afterwards.
    emit_cells() {
      srun --nodes=1 --ntasks=1 --cpus-per-task=288 xargs -P 48 -L 1 sh -c \
        "$PY dace_cells.py emit \$0 \$1 \$2 --out $OUT/emit > $OUT/emit.\$0.\$1_\$2.log 2>&1; grep -h '^EMIT ' $OUT/emit.\$0.\$1_\$2.log || echo \"EMIT_FAILED \$0 \$1 \$2\""
    }
    for k in $LLR40 $BONSALL; do echo "$k canon new"; done | emit_cells
    for k in $LLR40 $BONSALL; do
      for pc in "canon old" "canon legacy" "none old" "none legacy" "none new"; do echo "$k $pc"; done
    done | emit_cells
    cat "$OUT"/emit.*.log | grep -h '^EMIT ' > "$OUT/emit.jsonl"
    vec_reports ;;
  vec)    # compile-only reports of every emitted cell
    vec_reports ;;
  time)   # $KERNELS split over the nodes: kernel i on node i mod N
    mapfile -t NODES < <(scontrol show hostnames "$SLURM_JOB_NODELIST")
    IFS=: read -ra KS <<< "$KERNELS"
    for n in "${!NODES[@]}"; do
      (
        for i in "${!KS[@]}"; do
          [ $(( i % ${#NODES[@]} )) -eq "$n" ] || continue
          k=${KS[$i]}
          CELLS=("c -" "cpp -" "canon new" "canon legacy" "canon old")
          [[ " $BONSALL " == *" $k "* ]] && CELLS+=("none new" "none legacy" "none old")
          for pc in "${CELLS[@]}"; do
            set -- $pc
            srun --nodes=1 --ntasks=1 -w "${NODES[$n]}" --exclusive --cpu-bind=cores --hint=nomultithread \
              env OMP_NUM_THREADS=1 OMP_PROC_BIND=close OMP_PLACES=cores numactl --cpunodebind=0 --membind=0 \
              "$PY" dace_cells.py time "$k" "$1" "$2" --out "$OUT/time" > "$OUT/time.$k.$1_$2.log" 2>&1
            grep -h '^TIME ' "$OUT/time.$k.$1_$2.log" || echo "TIME_FAILED $k $1 $2 (see $OUT/time.$k.$1_$2.log)"
          done
        done
      ) &
    done
    wait ;;
  retry)  # re-time listed cells, RETRY="kernel/pipeline/codegen ..."; a kernel's cells stay on one node
    mapfile -t NODES < <(scontrol show hostnames "$SLURM_JOB_NODELIST")
    mapfile -t KS < <(for c in $RETRY; do echo "${c%%/*}"; done | sort -u)
    for n in "${!NODES[@]}"; do
      (
        for i in "${!KS[@]}"; do
          [ $(( i % ${#NODES[@]} )) -eq "$n" ] || continue
          for c in $RETRY; do
            IFS=/ read -r k p g <<< "$c"
            [ "$k" = "${KS[$i]}" ] || continue
            srun --nodes=1 --ntasks=1 -w "${NODES[$n]}" --exclusive --cpu-bind=cores --hint=nomultithread \
              env OMP_NUM_THREADS=1 OMP_PROC_BIND=close OMP_PLACES=cores numactl --cpunodebind=0 --membind=0 \
              "$PY" dace_cells.py time "$k" "$p" "$g" --out "$OUT/time" > "$OUT/time.$k.${p}_$g.log" 2>&1
            grep -h '^TIME ' "$OUT/time.$k.${p}_$g.log" || echo "TIME_FAILED $k $p $g"
          done
        done
      ) &
    done
    wait ;;
esac
echo "done $(date -Is)"
