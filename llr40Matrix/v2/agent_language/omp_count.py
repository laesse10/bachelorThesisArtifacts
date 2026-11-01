"""Thread-level parallelism in the agents' valid submissions: OpenMP parallel regions (C, Fortran)
and do concurrent (Fortran). Reads agent_picks.json and the sha256 list of default/T1-valid ones."""
import collections, hashlib, json, re, sys

picks = json.load(open(sys.argv[1]))
valid = set(json.load(open(sys.argv[2])))
C_PAR = re.compile(r"^\s*#\s*pragma\s+omp\s+(parallel|target|taskloop)\b", re.M)
C_SIMD = re.compile(r"^\s*#\s*pragma\s+omp\s+simd\b", re.M)
F_PAR = re.compile(r"^\s*!\$omp\s+(parallel|target|taskloop)\b", re.M | re.I)
F_SIMD = re.compile(r"^\s*!\$omp\s+simd\b", re.M | re.I)
F_DC = re.compile(r"\bdo\s+concurrent\b", re.I)
n = collections.Counter(); par = collections.Counter(); simd = collections.Counter(); dc = collections.Counter()
missing = 0
for kernel, cands in picks.items():
    for c in cands:
        if c["sha256"] not in valid:
            continue
        try:
            src = open(c["path"], "rb").read()
        except OSError:
            missing += 1
            continue
        if hashlib.sha256(src).hexdigest() != c["sha256"]:
            missing += 1
            continue
        text = src.decode("utf-8", "replace")
        model = c["arm"].split("-")[1]
        for key in ((c["language"], model), (c["language"], "all")):
            n[key] += 1
            if c["language"] == "c":
                par[key] += bool(C_PAR.search(text)); simd[key] += bool(C_SIMD.search(text))
            else:
                par[key] += bool(F_PAR.search(text)); simd[key] += bool(F_SIMD.search(text))
                dc[key] += bool(F_DC.search(text))
print("missing or changed:", missing)
for key in sorted(n):
    print(f"{key[0]:8s} {key[1]:9s} n={n[key]:3d}  parallel={par[key]:3d} ({par[key] / n[key]:.0%})"
          f"  simd={simd[key]:3d}  do_concurrent={dc[key]:3d}")
