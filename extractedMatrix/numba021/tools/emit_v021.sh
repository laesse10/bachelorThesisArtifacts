#!/bin/bash
# Emit <k>_numba.py with the v0.2.1 Numba translator (release-v0.2.1 305559cc9, which carries PR #113)
# from the 26a4f0cf NumPy reference and the bench-info the 26a4f0cf harness builds for it.
# The v0.2.1 BenchInfoDocument rejects fields it does not know (extra_forbidden); only those are
# dropped, one per try, and listed in <k>.dropped.txt. For all seven kernels that was `precisions`.
# Run on macOS with the HPCAgent-Bench .venv (it has relayout 2718b2fe, which the release pins and
# venv_llr40v2 on daint lacks); the release tree cannot build bench-info itself on macOS (seal.py).
# usage: emit_v021.sh RELEASE_TREE PYTHON WORKDIR KERNEL
#   WORKDIR/KERNEL/ holds KERNEL_numpy.py and bench_info.26a4f0cf.json (both copied from daint)
set -uo pipefail
B=$1; PY=$2; D=$3/$4; k=$4
cp "$D/bench_info.26a4f0cf.json" "$D/bench_info.v021in.json"; : > "$D/$k.dropped.txt"
for _ in $(seq 1 20); do
  out=$(cd "$B" && PYTHONPATH="$B" "$PY" -m hpcagent_bench.translators.numpyto_numba.cli emit \
        --kernel "$D/${k}_numpy.py" --out "$D" --bench-info "$D/bench_info.v021in.json" 2>&1)
  if ! grep -q extra_forbidden <<<"$out"; then
    echo "$k: $(tail -1 <<<"$out") | dropped: $(paste -sd, "$D/$k.dropped.txt")"; exit 0
  fi
  loc=$(grep -B1 "Extra inputs are not permitted" <<<"$out" | head -1 | tr -d ' ')
  echo "$loc" >> "$D/$k.dropped.txt"
  "$PY" -I - "$D/bench_info.v021in.json" "$loc" <<'P'
import json, sys
p, loc = sys.argv[1], sys.argv[2].split(".")
d = json.load(open(p)); o = d
for part in loc[:-1]: o = o[int(part)] if isinstance(o, list) else o[part]
del o[loc[-1]]; json.dump(d, open(p, "w"), indent=1)
P
done
echo "$k: gave up"; tail -3 <<<"$out"; exit 1
