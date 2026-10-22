#!/bin/bash
# The commands that drew every plot. Run from the package root with HPCAgent-Bench installed.
set -euo pipefail
COLS=dace_cpu_canon,dace_gpu_canon,pluto,polly,ppcg_hip,numba_fastmath
for t in llr40 scicomp40; do
  exp=$([ $t = llr40 ] && echo llr40v021 || echo scicomp40)
  for lang in c hip; do
    hpcagent-bench plot kernel-speedups data/observations/observations.db --canon-db data/canon.db \
      --canon-columns $COLS --experiment $exp --language $lang --setups "$exp-(oss120b|qwen38)-$lang" \
      --tag-file data/tag-$t.txt --offset 0.7 \
      --title "$t $lang: oss120b and qwen38 baselines with compiler columns (speedup over best-of(numba,c))" \
      --out plots/$t-$lang-baselines.pdf
  done
  hpcagent-bench plot kernel-speedups --canon-db data/canon.db --canon-columns $COLS \
    --tag-file data/tag-$t.txt --baseline numba --offset 0.7 \
    --title "$t: compilers over numba (canon sweep)" --out plots/$t-compilers.pdf
done
