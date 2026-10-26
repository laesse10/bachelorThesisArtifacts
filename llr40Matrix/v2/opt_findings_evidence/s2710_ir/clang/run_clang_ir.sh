#!/bin/sh
# clang's side of the s2710 comparison (2026-10-26): optimization remarks, LLVM IR and disassembly of the
# translated C and of the variant with x[0] read before the loop.
#
# daint was out of reach (expired CSCS certificate), so this cross-compiles on a Mac with Homebrew's
# clang 22.1.3 (the matrix used the official 22.1.0 release on daint). -mcpu=neoverse-v2 -mtune=generic stands
# in for -march=native on a GH200 node: with it, the instruction sequence of tsvc_2_s2710_fp64 is identical to
# the one daint's clang 22.1.0 built for the matrix (../../../clang/asm_so/tsvc_2_s2710/clang.s.txt), compared
# by norm.py (only objdump's spelling of the constant 1.0 differs).
#
# s2710.c / s2710_hoist.c: the kernel function of ../../../clang/emitted_sources/tsvc_2_s2710/tsvc_2_s2710_fp64.c
# and of ../../../followup/variants/src/tsvc_2_s2710_fp64.hoist.c, unchanged, with only <stdint.h> included
# (the function uses nothing else; the rest of the emitted file is helper code it does not call).
set -e
cd "$(dirname "$0")"
LLVM_BIN=${S2710_LLVM_BIN:-/opt/homebrew/opt/llvm@22/bin}
CL=$LLVM_BIN/clang
OBJDUMP=$LLVM_BIN/llvm-objdump
cc() {
  "$CL" --target=aarch64-unknown-linux-gnu -mcpu=neoverse-v2 -mtune=generic -O3 -std=c23 -fopenmp=libomp \
    -fstrict-aliasing -fPIC -fno-math-errno -fno-trapping-math -fno-signed-zeros -ffp-contract=fast \
    -fveclib=libmvec "$@"
}
"$CL" --version | head -1 > clang_version.txt
for f in s2710 s2710_hoist; do
  cc -c $f.c -o $f.o -Rpass='licm|loop-vectorize|simple-loop-unswitch' -Rpass-missed='licm|loop-vectorize' \
     -Rpass-analysis='loop-vectorize' 2> $f.remarks.txt
  cc -S -emit-llvm -fno-discard-value-names $f.c -o $f.ll
  "$OBJDUMP" -d $f.o > $f.s.txt
  rm -f $f.o
done
python3 norm.py s2710.s.txt > s2710.insns.txt
python3 norm.py ../../../clang/asm_so/tsvc_2_s2710/clang.s.txt > daint_clang.insns.txt
diff s2710.insns.txt daint_clang.insns.txt > insns.diff || true
