#!/usr/bin/env python3
"""Table of the DaCe follow-up (rows/): per kernel and cell, the fastest timed call over all rounds, each
round's fastest call, and the speedup over the unit's translated C++ cell (``cpp``) and over DaCe's own
cell (``canon``). The harness reports milliseconds (``python``: host-side timer around the call).

usage: python3 analyze_dace_gap.py [OUT_DIR] > summary_table.md
"""
import collections, json, pathlib, sys

OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else pathlib.Path(__file__).resolve().parent / "out")
cells = collections.defaultdict(dict)    # kernel -> label -> {round: (min_ms, status, validated)}
for f in sorted((OUT / "rows").glob("*.json")):
    r = json.loads(f.read_text())
    t, ok = [], r["status"] == "ok"
    for impl in (r.get("impls") or {}).values():
        t += impl.get("python") or []
        ok = ok and impl.get("validated") is not False
    cells[r["kernel"]].setdefault(r["label"], {})[r["round"]] = (min(t) if t else None, r["status"] if ok else f"{r['status']}/invalid")
print("| kernel | cell | min of k (ms) | over C++ | over DaCe | per round (ms) | status |")
print("|---|---|---:|---:|---:|---|---|")
for k, labs in cells.items():
    best = {l: min((v[0] for v in rr.values() if v[0] is not None), default=None) for l, rr in labs.items()}
    for l, rr in labs.items():
        b = best[l]
        sp = lambda ref: f"{best[ref] / b:.2f}" if b and best.get(ref) else ""
        per = ", ".join(f"r{x} {v[0]:.3f}" if v[0] is not None else f"r{x} -" for x, v in sorted(rr.items()))
        st = "/".join(sorted({v[1] for v in rr.values()}))
        print(f"| `{k}` | `{l}` | {b:.3f} | {sp('cpp')} | {sp('canon')} | {per} | {st} |" if b else f"| `{k}` | `{l}` | | | | {per} | {st} |")
