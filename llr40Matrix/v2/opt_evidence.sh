#!/bin/bash
# Compile-only experiments behind opt_findings.md -> opt_findings_evidence/. Each changes ONE
# thing against the opt-report compile line (same flags, same -march=native host type) and records
# what the compiler does. They explain the reports; they produce no timing.
#
#   s3111_vect_dump      gfortran -fdump-tree-vect-details: why the conditional sum is not vectorised
#   s3111_noparen        the same Fortran with the outer parentheses of the update removed
#   s316_compiler_swap   the C source with gcc-13, the C++ source with g++-14
#   s231_linterchange    gcc-14 -fdump-tree-linterchange-details on the autogen C
#   s115_fortran_fma     which FP instructions gfortran emits for s115 (fused or not)
#   scan_affine_fma      fmadd vs fmul+fadd in scan_affine_decay, C vs Fortran
set -euo pipefail
cd "$(dirname "$0")"
OR=opt_reports; EV=opt_findings_evidence; rm -rf "$EV"; mkdir -p "$EV"
V=/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2-aux/hpcagent_bench/envs/vecmath.h
BASE="-O3 -march=native -fopenmp -fno-math-errno -fno-trapping-math -fno-signed-zeros -ffp-contract=fast -fstrict-aliasing -fPIC"
CF="$BASE -include $V -Wall -Wextra -D_POSIX_C_SOURCE=199309L"
FF="$BASE -ftree-parallelize-loops=1 -Wall -Wextra -std=f2018 -ffree-form -ffree-line-length-none"
dis() { objdump -d --no-show-raw-insn --disassemble="$2" "$1" | grep -E "^\s+[0-9a-f]+:" | cut -f2-; }

# --- s3111 -----------------------------------------------------------------------------------
d=$EV/s3111; mkdir -p $d; cp $OR/tsvc_2_s3111/fortran/tsvc_2_s3111_fp64.f90 $d/orig.f90
sed 's/sum_val = (sum_val + a((i_l0) + 1))/sum_val = sum_val + a((i_l0) + 1)/' $d/orig.f90 > $d/noparen.f90
( cd $d && /usr/bin/gfortran-13 $FF -c orig.f90 -o orig.o -fdump-tree-vect-details -fopt-info-vec-all 2> orig.optinfo.txt \
  && /usr/bin/gfortran-13 $FF -c noparen.f90 -o noparen.o -fopt-info-vec-optimized -fopt-info-vec-missed 2> noparen.optinfo.txt )
grep -n -B2 -A1 "unsupported use in stmt" $d/orig.f90.*vect > $d/orig.vect.excerpt.txt || true
diff $d/orig.f90 $d/noparen.f90 > $d/noparen.diff || true
dis $d/orig.o tsvc_2_s3111_fp64 > $d/orig.s.txt; dis $d/noparen.o tsvc_2_s3111_fp64 > $d/noparen.s.txt

# --- s316 ------------------------------------------------------------------------------------
d=$EV/s316; mkdir -p $d
cp $OR/tsvc_2_s316/c/tsvc_2_s316_fp64.c $d/s316.c; cp $OR/tsvc_2_s316/cpp/tsvc_2_s316_fp64.cpp $d/s316.cpp
( cd $d && /usr/bin/gcc-14 $CF -std=c23 -c s316.c -o c_gcc14.o && /usr/bin/gcc-13 $CF -std=c2x -c s316.c -o c_gcc13.o \
  && /usr/bin/g++-13 $CF -std=c++23 -c s316.cpp -o cpp_gxx13.o && /usr/bin/g++-14 $CF -std=c++23 -c s316.cpp -o cpp_gxx14.o ) 2>/dev/null
for o in c_gcc14 c_gcc13 cpp_gxx13 cpp_gxx14; do dis $d/$o.o tsvc_2_s316_fp64 > $d/$o.s.txt
  echo "$o fcsel=$(grep -cE '^fcsel' $d/$o.s.txt) branch_b.gt=$(grep -cE '^b\.gt' $d/$o.s.txt)"; done > $d/summary.txt

# --- s231 interchange pass -----------------------------------------------------------------------
d=$EV/s231; mkdir -p $d; cp $OR/tsvc_2_s231/c/tsvc_2_s231_fp64.c $d/s231.c
( cd $d && /usr/bin/gcc-14 $CF -std=c23 -c s231.c -o s231.o -fdump-tree-linterchange-details 2>/dev/null )
f=$(ls $d/*linterchange); { echo "dump: $(basename $f), $(wc -l < $f) lines";
  echo "lines mentioning interchange/consider/loop nest: $(grep -ciE 'interchang|consider|loop nest|loop_pair' $f)"; } > $d/summary.txt

# --- s115 fortran FMA ------------------------------------------------------------------------------
d=$EV/s115; mkdir -p $d; cp $OR/tsvc_2_s115/fortran/tsvc_2_s115_fp64.f90 $d/s115.f90
( cd $d && /usr/bin/gfortran-13 $FF -c s115.f90 -o s115.o 2>/dev/null )
dis $d/s115.o tsvc_2_s115_fp64 > $d/s115.s.txt
grep -oE "^(fmls|fmla|fmsub|fnmsub|fmadd|fmul|fsub)\s+\S+" $d/s115.s.txt | sort | uniq -c > $d/summary.txt

# --- scan_affine_decay FMA -------------------------------------------------------------------------
d=$EV/scan_affine_decay; mkdir -p $d
for r in c fortran; do grep -E "^(fmadd|fmul|fadd)\s" $OR/scan_affine_decay/$r/scan_affine_decay_fp64.*.s.txt 2>/dev/null \
  | awk '{print $1}' | sort | uniq -c | sed "s/^/$r: /"; done > $d/summary.txt || true
cut -f3- $OR/scan_affine_decay/c/scan_affine_decay_fp64.c.s.txt > $d/c.s.txt
cut -f3- $OR/scan_affine_decay/fortran/scan_affine_decay_fp64.f90.s.txt > $d/fortran.s.txt
grep -E "^(fmadd|fmul|fadd)\s" $d/c.s.txt $d/fortran.s.txt | sed 's/\s\+/ /g' >> $d/summary.txt || true
echo "evidence -> $EV"; for s in $EV/*/summary.txt; do echo "== $s"; cat $s; done
