"""Run the NumPy reference and an emitted Numba module on one preset; compare every array argument."""
import importlib.util, inspect, sys, pathlib, time
import numpy as np, yaml
kdir, emitdir, preset = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2]), sys.argv[3]
k = kdir.name
def load(path, name):
    s = importlib.util.spec_from_file_location(name, path); m = importlib.util.module_from_spec(s); s.loader.exec_module(m); return m
y = yaml.safe_load((kdir / f"{k}.yaml").read_text())
init = load(kdir / f"{k}.py", k + "_init")
ref = load(kdir / f"{k}_numpy.py", k + "_np")
nb = load(emitdir / f"{k}_numba.py", k + "_nb")
import json
scal = dict(json.load(open(emitdir / "bench_info.26a4f0cf.json"))["benchmark"]["parameters"][preset])
sig = inspect.signature(init.initialize)
args = init.initialize(**{p: scal[p] for p in sig.parameters if p in scal}, rng=np.random.default_rng(0))
import ast
fn = next(n for n in ast.parse((kdir / f"{k}.py").read_text()).body if isinstance(n, ast.FunctionDef) and n.name == "initialize")
ret = max((n.value for n in ast.walk(fn) if isinstance(n, ast.Return) and isinstance(n.value, ast.Tuple)), key=lambda t: len(t.elts))
names = [ast.unparse(e.args[0]) if isinstance(e, ast.Call) else ast.unparse(e) for e in ret.elts]
assert len(names) == len(args), (names, len(args))
arrs = dict(zip(names, args))
def call(mod):
    a = {n: np.copy(v) for n, v in arrs.items()}
    f = getattr(mod, y["func_name"]); fs = inspect.signature(f)
    kw = {p: (a[p] if p in a else scal[p]) for p in fs.parameters if p in a or p in scal}
    t = time.perf_counter(); f(**kw); return a, time.perf_counter() - t
ra, _ = call(ref)
na, t1 = call(nb)
_, t2 = call(nb)
worst = 0.0
for n in names:
    if not isinstance(ra[n], np.ndarray): continue
    r, o = np.asarray(ra[n]), np.asarray(na[n])
    if r.dtype.kind == "f":
        d = float(np.max(np.abs(r - o) / (np.abs(r) + 1e-300))) if r.size else 0.0
        same = np.allclose(o, r, rtol=1e-9, atol=1e-12)
    else:
        d = float(np.sum(r != o)); same = d == 0
    worst = max(worst, d) if r.dtype.kind == "f" else worst
    if not same: print(f"  MISMATCH {n}: {d}")
print(f"{k} {preset}: numba first call {t1:.1f} s (incl. compile), second {t2*1e3:.2f} ms; max rel diff {worst:.2e}")
