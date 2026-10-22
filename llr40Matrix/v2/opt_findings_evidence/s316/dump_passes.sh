#!/bin/bash
# Which GCC pass turns s316's conditional update into a select (fcsel) in GCC 14 but not in GCC 13?
# Compiles the same s316.c with both versions and the harness's C flags, with every tree and RTL dump,
# then lists, per version, the first dump file in which a conditional-move form of the update appears.
# Run on a daint login node (compile only):  bash dump_passes.sh
set -euo pipefail
cd "$(dirname "$0")"
FLAGS="-O3 -march=native -fopenmp -fno-math-errno -fno-trapping-math -fno-signed-zeros -ffp-contract=fast -fstrict-aliasing -fPIC -std=c2x"
for cc in gcc-13 gcc-14; do
  out=dumps/$cc; rm -rf "$out"; mkdir -p "$out"
  ( cd "$out" && /usr/bin/$cc $FLAGS -fdump-tree-all-details -fdump-rtl-all -fopt-info-all=optinfo.txt -c ../../s316.c -o s316.o )
  objdump -d "$out/s316.o" > "$out/s316.s.txt"
  echo "== $cc: fcsel=$(grep -c fcsel "$out/s316.s.txt" || true) b.gt=$(grep -c 'b.gt' "$out/s316.s.txt" || true)"
  # tree level: MIN_EXPR / COND_EXPR / phiopt; RTL level: if_then_else / cond-exec (ce1, ce2, ce3)
  for f in $(ls "$out" | grep -E '\.(t|r)\.' | sort -t. -k3,3n); do
    if grep -qE "MIN_EXPR|COND_EXPR|if_then_else" "$out/$f"; then echo "   first conditional form in: $f"; break; fi
  done
done
