#!/usr/bin/env bash
# scicomp40 stacked efficacy and per-kernel speedups, usable comparisons only (qwen38 HIP is the wedged pair).
set -uo pipefail
X=/capstor/scratch/cscs/ybudanaz/x86_64; O=$X/reports/scicomp40-data-20261102; H=$X/hb-v050-w2/W2-1; PY=$H/.venv/bin/python
export PYTHONDONTWRITEBYTECODE=1 MPLBACKEND=Agg
cd $O
$PY - <<'P'
import pandas as pd
o = pd.read_csv("extract/llr40_observations.csv", low_memory=False)
o[~o.setup.str.startswith("scicomp40-qwen38-hip")].to_csv("observations-usable.csv", index=False)
P
$PY $H/statistics/paired_setups.py --observations observations-usable.csv --family lang-skills-scicomp40 --include-incomplete   --pair scicomp40-qwen38-c-lang-skills,scicomp40-qwen38-c --pair scicomp40-oss120b-c-lang-skills,scicomp40-oss120b-c   --pair scicomp40-oss120b-hip-lang-skills,scicomp40-oss120b-hip --out figures/pairs.csv --setups-out figures/setups.csv 2>&1 | tail -3
for mode in absolute relative; do
 $PY -m hpcagent_bench.cli plot efficacy observations-usable.csv --mode $mode --intervention lang-skills --control-label "No Skills"    --include-incomplete --success-row --pairs-csv figures/pairs.csv --out figures/efficacy-stacked-$mode.pdf --table figures/efficacy-stacked-$mode.csv 2>&1 | tail -3
done
for lang in c hip; do
 $PY -m hpcagent_bench.cli plot kernel-speedups observations-usable.csv --experiment scicomp40 --tag-file /capstor/scratch/cscs/ybudanaz/x86_64/hb-v050-w2/W2-1/hpcagent_bench/tags/scicomp40.txt --experiment scicomp40 --language $lang --tag-file /capstor/scratch/cscs/ybudanaz/x86_64/hb-v050-w2/W2-1/hpcagent_bench/tags/scicomp40.txt --mark-pending --title "SciComp40 $lang" --out figures/kernel-speedups-$lang.pdf 2>&1 | tail -3
done
