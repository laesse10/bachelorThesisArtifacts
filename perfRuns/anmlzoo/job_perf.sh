#!/bin/bash
#SBATCH --job-name=perf_anmlzoo
#SBATCH --account=g34
#SBATCH --partition=debug
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=72
#SBATCH --time=00:30:00
#SBATCH --output=perf_anmlzoo_%j.out

# ANMLZoo on VASim (CPU, single-threaded): the four runs of nfa_work/profile.sh and
# profile2.sh, selected by argument (default: all four), e.g.  sbatch job_perf.sh snort10
# Sampled at 99 Hz, not profile.sh's 999 Hz: the runs take 8-30+ min each, and 999 Hz DWARF
# stacks make multi-GB perf.data whose reports take longer than the run itself.
set -u
cd /capstor/scratch/cscs/lhulsbergen/perf_runs/anmlzoo
source ../common.sh
V=/capstor/scratch/cscs/lhulsbergen/vasim/vasim
Z=/capstor/scratch/cscs/lhulsbergen/anmlzoo
export OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 MKL_NUM_THREADS=1 BLIS_NUM_THREADS=1

for run in ${@:-brill10 fermi1 entity10 snort10}; do
  case $run in
    brill10)  prof_cpu brill10  99 cycles:u -- $V -q -t $Z/Brill/anml/brill.1chip.anml           $Z/Brill/inputs/brill_10MB.input ;;
    fermi1)   prof_cpu fermi1   99 cycles:u -- $V -q -t $Z/Fermi/anml/fermi_2400.1chip.anml      $Z/Fermi/inputs/rp_input_1MB.input ;;
    entity10) prof_cpu entity10 99 cycles:u -- $V -q -t $Z/EntityResolution/anml/1000.1chip.anml $Z/EntityResolution/inputs/1000_10MB.input ;;
    snort10)  prof_cpu snort10  99 cycles:u -- $V -q -t $Z/Snort/anml/snort.1chip.anml           $Z/Snort/inputs/snort_10MB.input ;;
    *) echo "unknown run $run"; exit 1 ;;
  esac
done

keep_results anmlzoo 20261002_anmlzoo
echo "########## done $(date +%T)"
