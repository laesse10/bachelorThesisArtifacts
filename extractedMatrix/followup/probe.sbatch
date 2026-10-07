#!/bin/bash -l
#SBATCH --account=g34
#SBATCH --exclusive
#SBATCH --output=logs/%x.%j.out
#SBATCH --error=logs/%x.%j.out
# nrt_probe.py alone (the esirkN2 unit's probe step failed on relative paths in job 4998121).
# NUMBA_CACHE_DIR is private: the probe imports the file under another module name than the harness, and
# loading the harness's cache entries for it fails (job 4998217, `numba` half). PROBE_CELLS picks the halves.
#   sbatch --job-name=nrtprobe --partition=debug --nodes=1 --time=00:15:00 probe.sbatch
set -uo pipefail
cd /capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts-extracted/extractedMatrix/followup
PY=/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python
BENCH=/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-ext2
K=warpx_esirkepov_deposition
KD=$(dirname $(find "$BENCH/hpcagent_bench/benchmarks" -name "$K.yaml" | head -1))
mkdir -p profiles/nrt_probe scratch/probe
NODE=$(hostname)
for V in ${PROBE_CELLS:-numba:$KD/${K}_numba_np.py nbnort:variants/${K}_nbnort.py}; do
  L=${V%%:*}; F=${V#*:}
  CMD=(srun --nodes=1 --ntasks=1 --exclusive --cpu-bind=cores --hint=nomultithread env LLR40_BENCH="$BENCH" NUMBA_CACHE_DIR="$PWD/scratch/probe/nbcache.$L" OMP_NUM_THREADS=1 NUMBA_NUM_THREADS=1
       numactl --cpunodebind=0 --membind=0 perf record -e cycles:u -c 100000 -o "scratch/probe/$L.data" -- "$PY" nrt_probe.py "$F" 1000 "profiles/nrt_probe/$L.addresses.txt")
  echo "$(date -Is) job=${SLURM_JOB_ID} partition=${SLURM_JOB_PARTITION} ${CMD[*]}" >> srun_lines.txt
  "${CMD[@]}"
  perf report -i "scratch/probe/$L.data" --stdio --sort dso,sym --percent-limit 0.5 > "profiles/nrt_probe/$L.report.txt" 2>&1
done
