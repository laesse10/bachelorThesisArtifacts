#!/usr/bin/env python3
"""Per-loop share of the perf samples in a `perf annotate --stdio` file: every backward branch
(target < its own address) closes a loop [target, branch]; a sample is charged to the innermost loop
that contains it. Prints the loops in address order with their share and their instruction count.
usage: loop_shares.py ANNOTATE_FILE [SYMBOL]"""
import re, sys

lines = open(sys.argv[1]).read().split("##########")
sym = sys.argv[2] if len(sys.argv) > 2 else None
blk = next(b for b in lines if b.strip() and (sym is None or b.strip().startswith(sym)))
ins = []   # (addr, pct, text)
for l in blk.splitlines():
    m = re.match(r"^\s+([\d.]+)\s+:\s+([0-9a-f]+):\s+(\S+)\s*(.*)$", l)
    if m:
        ins.append((int(m.group(2), 16), float(m.group(1)), m.group(3), m.group(4)))
loops = []
for a, p, mn, ops in ins:
    if mn.startswith("b") and mn not in ("bl", "blr", "br"):
        t = re.search(r"\b([0-9a-f]+) <", ops)
        if t and int(t.group(1), 16) < a:
            loops.append((int(t.group(1), 16), a))
loops = sorted(set(loops))
share = {lp: 0.0 for lp in loops}
outside = 0.0
for a, p, mn, ops in ins:
    inner = [lp for lp in loops if lp[0] <= a <= lp[1]]
    if inner:
        share[min(inner, key=lambda lp: lp[1] - lp[0])] += p
    else:
        outside += p
tot = sum(p for _, p, _, _ in ins)
for lp in loops:
    n = sum(1 for a, *_ in ins if lp[0] <= a <= lp[1])
    if share[lp] >= 0.3:
        print(f"{lp[0]:6x}-{lp[1]:6x}  {n:4d} insns  {share[lp]:6.2f}%")
print(f"outside loops {outside:.2f}%  total {tot:.2f}%")
