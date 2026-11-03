#!/usr/bin/env bash
# scicomp40 stacked efficacy and per-kernel speedups, all 8 arms (qwen38 HIP = reruns 714849/714850).
set -uo pipefail
X=/capstor/scratch/cscs/ybudanaz/x86_64; O=$X/reports/scicomp40-data-20261103; H=$X/hb-bf; PY=$X/hb-v050/.venv/bin/python; export PYTHONPATH=$H:$H/agent
export PYTHONDONTWRITEBYTECODE=1 MPLBACKEND=Agg
cd $O
$PY - <<'P'
import pandas as pd
o = pd.read_csv("extract/llr40_observations.csv", low_memory=False)
o.to_csv("observations-usable.csv", index=False)
P
$PY $H/statistics/paired_setups.py --observations observations-usable.csv --family lang-skills-scicomp40 --include-incomplete   --pair scicomp40-qwen38-c-lang-skills,scicomp40-qwen38-c --pair scicomp40-oss120b-c-lang-skills,scicomp40-oss120b-c   --pair scicomp40-oss120b-hip-lang-skills,scicomp40-oss120b-hip --pair scicomp40-qwen38-hip-lang-skills,scicomp40-qwen38-hip --out figures/pairs.csv --setups-out figures/setups.csv 2>&1 | tail -3
for mode in absolute relative; do
 $PY -m hpcagent_bench.cli plot efficacy observations-usable.csv --mode $mode --intervention lang-skills --control-label "No Skills"    --include-incomplete --success-row --pairs-csv figures/pairs.csv --out figures/efficacy-stacked-$mode.pdf --table figures/efficacy-stacked-$mode.csv 2>&1 | tail -3
done
for lang in c hip; do
 $PY -m hpcagent_bench.cli plot kernel-speedups observations-usable.csv --experiment scicomp40 --tag-file $H/hpcagent_bench/tags/scicomp40.txt --experiment scicomp40 --language $lang --tag-file $H/hpcagent_bench/tags/scicomp40.txt --mark-pending --title "SciComp40 $lang" --out figures/kernel-speedups-$lang.pdf 2>&1 | tail -3
done
