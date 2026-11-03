"""The qwen38 HIP arms span two jobs each: the first run (710590, 711539), whose HIP grading wedge damaged some
kernels, and the rerun (714849, 714850) of exactly those kernels (lists in reruns/). Keep the first run's rows only
for kernels the rerun did not redo."""
import pathlib

import pandas as pd

HERE = pathlib.Path(__file__).parent
FIRST = {"scicomp40-qwen38-hip": "710590", "scicomp40-qwen38-hip-lang-skills": "711539"}
for name in ("llr40_observations.csv", "llr40_sources_index.csv"):
    path = HERE / "extract" / name
    table = pd.read_csv(path, low_memory=False, dtype={"job": str})
    drop = pd.Series(False, index=table.index)
    for setup, job in FIRST.items():
        rerun = set((HERE / "reruns" / f"{setup}.txt").read_text().split())
        drop |= (table.setup == setup) & (table.job == job) & table.kernel.isin(rerun)
    table[~drop].to_csv(path, index=False)
    print(name, "dropped", int(drop.sum()), "kept", int((~drop).sum()))
