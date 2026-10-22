#!/bin/bash
# DaCe against translated C++ with GCC 14.2: the timing stage of ../dace.sbatch, unchanged except that
# bin_gxx14/g++ (-> /usr/bin/g++-14) comes first on PATH. The harness resolves the C++ driver on PATH
# (languages.resolve_compiler) for the translated C++ column and pins it as DaCe's compiler.cpu.executable
# (frameworks/dace_framework.pin_host_compiler), so both are built by g++ 14.2 here, where the original run used
# g++ 13.3. Translated C (gcc 14.2) is timed as well. After every cell, the .comment section of the library it
# built is saved to $OUT/comment/<cell>.txt, as a record of the compiler.
#   sbatch --nodes=11 --time=01:00:00 --export=ALL,KERNELS=k1:k2:... dace_gcc14.sbatch   (from llr40Matrix/v2/dace/gcc14/)
#SBATCH --job-name=dace-gcc14
#SBATCH --account=g34
#SBATCH --partition=normal
#SBATCH --exclusive
#SBATCH --output=logs/%x.%j.out
set -uo pipefail
cd "${SLURM_SUBMIT_DIR}"
ROOT=/capstor/scratch/cscs/lhulsbergen
export DACE_BENCH=${DACE_BENCH:-$ROOT/HPCAgent-Bench-dace}            # 5cfcb4f2
PY=${DACE_PYTHON:-$ROOT/venv_dace5cf/bin/python}                       # spcl/dace@dea0b39c
export PATH="$(pwd)/bin_gxx14:$(dirname "$PY"):$PATH"
OUT=${DACE_OUT:-$ROOT/llr40_dace_gcc14}
BONSALL="tsvc_2_s313 tsvc_2_vdotr tsvc_2_s453 tsvc_2_s314"
mkdir -p logs "$OUT/comment"
echo "job=$SLURM_JOB_ID nodes=$SLURM_JOB_NODELIST bench=$(git -C "$DACE_BENCH" rev-parse HEAD) g++=$(command -v g++) $(g++ --version | head -1)"
comment() {   # the .comment section of the library a cell built
  local k=$1 p=$2 g=$3 lib
  if [ "$p" = c ] || [ "$p" = cpp ]; then
    lib=$(find "$DACE_BENCH/hpcagent_bench/benchmarks" -path "*/$k/cpp_backend/build/lib${k}_$([ "$p" = c ] && echo cc || echo cpp).so" | head -1)
  else
    lib=$(find "$OUT/time/build/$k.${p}_$g" -name "*.so" | head -1)
  fi
  { echo "lib=$lib"; [ -n "$lib" ] && readelf -p .comment "$lib"; } > "$OUT/comment/$k.${p}_$g.txt" 2>&1
}
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
          "$PY" ../dace_cells.py time "$k" "$1" "$2" --out "$OUT/time" > "$OUT/time.$k.$1_$2.log" 2>&1
        grep -h '^TIME ' "$OUT/time.$k.$1_$2.log" || echo "TIME_FAILED $k $1 $2 (see $OUT/time.$k.$1_$2.log)"
        comment "$k" "$1" "$2"
      done
    done
  ) &
done
wait
echo "done $(date -Is)"
