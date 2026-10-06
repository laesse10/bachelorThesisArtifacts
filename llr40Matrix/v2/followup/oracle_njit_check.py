#!/usr/bin/env python3
"""Experiment 2b: is the harness oracle NumPy's arithmetic? At 26a4f0cf the oracle is the NumPy reference
compiled by numba (frameworks/test.py:njit_reference, no fastmath, no parallel). This runs the same
reference both interpreted and through njit_reference on the harness's preset-S inputs and checks the
outputs bit for bit. (Preset M interpreted would take minutes per kernel; S exercises the same code.)
usage: LLR40_BENCH=<bench> python oracle_njit_check.py      (cwd = bench, PYTHONPATH = bench)"""
import importlib, inspect
import numpy as np
from hpcagent_bench.frameworks import Benchmark
from hpcagent_bench.frameworks.test import njit_reference

for k in ["tsvc_2_s3111", "tsvc_2_s311", "tsvc_2_s319", "quasi_affine_reduce_odd", "segment_reduce_ragged",
          "scan_affine_decay", "versioned_distance_update", "tsvc_2_s323"]:
    b = Benchmark(k)
    d = b.get_data(preset="S", datatype="float64")
    f = getattr(importlib.import_module(f"hpcagent_bench.benchmarks.loop_level_reasoning.{k}.{k}_numpy"), b.info["func_name"])
    args = list(inspect.signature(f).parameters)
    d1 = {a: (np.copy(v) if isinstance(v, np.ndarray) else v) for a, v in d.items()}
    d2 = {a: (np.copy(v) if isinstance(v, np.ndarray) else v) for a, v in d.items()}
    f(*[d1[a] for a in args])
    g = njit_reference(f, b, d)
    g(*[d2[a] for a in args])
    outs = b.info.get("output_args") or b.spec.init.output_args
    same = all(np.array_equal(np.asarray(d1[o]).view(np.uint8), np.asarray(d2[o]).view(np.uint8)) for o in outs)
    print(f"{k:28s} njit oracle vs interpreted reference, preset S, outputs {outs}: "
          f"bitwise identical={same}; njit used={g is not f}")
