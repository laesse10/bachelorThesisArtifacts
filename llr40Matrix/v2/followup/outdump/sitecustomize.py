"""Follow-up output dump: save the arrays the harness validates, from its own inputs and seed.

Inert unless FOLLOWUP_DUMP_DIR is set (sweep_followup.py --dump-outputs). The harness grades a run by
calling ``hpcagent_bench.frameworks.utilities.validate(np_out, frmwrk_out, ...)`` (frameworks/test.py,
``matches_oracle``), first on the fresh first call and again after the timed reps. This hook wraps
that function and, on the FIRST call of the process only, saves

    FOLLOWUP_DUMP_DIR/out<i>.npy          the build's i-th output, as the harness compared it
    FOLLOWUP_DUMP_ORACLE_DIR/out<i>.npy   the oracle's i-th output (written once per kernel; a later
                                          run compares its oracle bit for bit against the saved one)
    FOLLOWUP_DUMP_DIR/meta.json           shapes, dtypes, sha256 of every array, rtol/atol, verdict

and then calls the original validate unchanged, so the run's status is the harness's own.
"""
import hashlib
import importlib.abc
import importlib.util
import json
import os
import sys

_DIR = os.environ.get("FOLLOWUP_DUMP_DIR")
_ODIR = os.environ.get("FOLLOWUP_DUMP_ORACLE_DIR")
_TARGET = "hpcagent_bench.frameworks.utilities"
_done = [False]


def _sha(a):
    import numpy as np
    return hashlib.sha256(np.ascontiguousarray(a).view(np.uint8).reshape(-1)).hexdigest()


def _as_list(x):
    return list(x) if isinstance(x, (tuple, list)) else [x]


def _patch(mod):
    orig = mod.validate

    def validate(ref, val, framework="Unknown", rtol=1e-5, atol=1e-8):
        ok = orig(ref, val, framework, rtol=rtol, atol=atol)
        if _done[0]:
            return ok
        _done[0] = True
        import numpy as np
        os.makedirs(_DIR, exist_ok=True)
        os.makedirs(_ODIR, exist_ok=True)
        meta = {"framework": framework, "rtol": rtol, "atol": atol, "validated": bool(ok), "outputs": []}
        for i, (r, v) in enumerate(zip(_as_list(ref), _as_list(val))):
            r, v = np.asarray(r), np.asarray(v)
            np.save(os.path.join(_DIR, f"out{i}.npy"), v)
            opath = os.path.join(_ODIR, f"out{i}.npy")
            if os.path.exists(opath):
                prev = np.load(opath, mmap_mode="r")
                same = prev.shape == r.shape and prev.dtype == r.dtype and _sha(prev) == _sha(r)
            else:
                np.save(opath, r)
                same = None   # this run wrote the kernel's oracle
            meta["outputs"].append({"index": i, "shape": list(v.shape), "dtype": str(v.dtype),
                                    "sha256": _sha(v), "oracle_shape": list(r.shape),
                                    "oracle_dtype": str(r.dtype), "oracle_sha256": _sha(r),
                                    "oracle_identical_to_saved": same})
        with open(os.path.join(_DIR, "meta.json"), "w") as fh:
            json.dump(meta, fh, indent=1)
        return ok

    mod.validate = validate


class _Finder(importlib.abc.MetaPathFinder):
    def find_spec(self, name, path, target=None):
        if name != _TARGET:
            return None
        sys.meta_path.remove(self)
        try:
            spec = importlib.util.find_spec(name)
        finally:
            sys.meta_path.insert(0, self)
        if spec is None or spec.loader is None:
            return spec
        orig_exec = spec.loader.exec_module

        def exec_module(module):
            orig_exec(module)
            _patch(module)

        spec.loader.exec_module = exec_module
        return spec


if _DIR and _ODIR:
    sys.meta_path.insert(0, _Finder())
