#!/usr/bin/env python3
"""Share of the perf samples inside Numba's NRT_incref / NRT_decref, from `perf report --sort dso,sym`.
nrt_probe.py found Numba's runtime module at a 64 KiB-aligned JIT address with nrt_atomic_add at +0x000,
nrt_atomic_sub at +0x010, NRT_incref at +0x050 and NRT_decref at +0x060 (profiles/nrt_probe/*.addresses.txt).
perf cannot name JIT code, so a [JIT] sample counts here when its address is at +0x050..+0x07f of a 64 KiB
boundary. (Kernel code at such offsets would also count; the kernels' own JIT code starts at other
offsets in every profile here, the hot ones at +0x0f44..+0x1764.)
usage: nrt_share.py REPORT [REPORT ...]"""
import re, sys

for f in sys.argv[1:]:
    t = open(f).read()
    nrt = jit = 0.0
    for p, a in re.findall(r"^\s+([\d.]+)%\s+\[JIT\][^\[]*\[\.\]\s+0x([0-9a-f]+)", t, re.M):
        jit += float(p)
        if 0x50 <= int(a, 16) & 0xFFFF < 0x80:
            nrt += float(p)
    print(f"{f}: NRT_incref/NRT_decref {nrt:.1f}% of samples (all JIT code {jit:.1f}%)")
