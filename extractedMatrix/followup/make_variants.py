#!/usr/bin/env python3
"""Writes the follow-up source variants into variants/ from the sources timed in job 4990441
(../emitted_sources). Every edit is asserted to apply exactly once."""
import pathlib, re

HERE = pathlib.Path(__file__).resolve().parent
EM = HERE.parent / "emitted_sources"


def once(s, old, new):
    assert s.count(old) == 1, (old[:80], s.count(old))
    return s.replace(old, new)


# ---- triangle counting: floor division by 2 and 32 as arithmetic shifts (C) ----
src = (EM / "triangle_count/triangle_count_fp64.c").read_text()
first, rest = src.split("\n", 1)
assert "hpcagent_bench-autogen" in first
head = ("// extractedMatrix/followup variant `cshift`: the emitted triangle_count_fp64.c (job 4990441) with every\n"
        "// int_floor(x, 2) written as (x) >> 1 and int_floor(x, 32) as (x) >> 5. For a signed int64, GCC's >> is an\n"
        "// arithmetic shift, which rounds towards minus infinity exactly as Python's // does. Nothing else changed.")
pre, kern = rest.split("void triangle_count_fp64", 1)
n = len(re.findall(r"int_floor\(\(([^()]*)\), (2|32)\)", kern))
kern = re.sub(r"int_floor\(\(([^()]*)\), 2\)", r"((\1) >> 1)", kern)
kern = re.sub(r"int_floor\(\(([^()]*)\), 32\)", r"((\1) >> 5)", kern)
assert n == 5 and "int_floor" not in kern
(HERE / "variants/triangle_count_cshift.c").write_text(head + "\n" + pre + "void triangle_count_fp64" + kern)
# the harness runs triangle_count_fp32 (integer-only arguments bind the fp32 symbol); its emitted source is
# the fp64 one with fp64 -> fp32 in the names (checked on daint: no other difference), so the same edit
v32 = (head + "\n" + pre + "void triangle_count_fp64" + kern).replace("triangle_count_fp64", "triangle_count_fp32")
(HERE / "variants/triangle_count_cshift_fp32.c").write_text(v32)

# ---- Esirkepov deposition, hand-written Numba: three variants ----
K = "warpx_esirkepov_deposition"
orig = (EM / K / f"{K}_numba_np.py").read_text()
doc_end = orig.index('"""', orig.index('"""') + 3) + 3

SERIAL = '''@nb.njit(cache=True)
def deposit_serial(
    jx, jy, jz, ion_lev, mask, uxp, uyp, uzp, wp, xp, yp, zp, dinv, xyzmin, lo, dt, rt, q, o, n_modes, geom,
    do_ion, red, npart,
):
    """FOLLOW-UP VARIANT: every particle in order, straight into Jx/Jy/Jz (no private copies, no parallel region)."""
{alloc}    for ip in range(npart):
        deposit_particle(
            ip, jx, jy, jz, ion_lev, mask, uxp, uyp, uzp, wp, xp, yp, zp, dinv, xyzmin, lo, dt, rt, q, o,
            n_modes, geom, do_ion, red, {bufs},
        )


'''
ALLOC_VIEWS = "    sb = np.zeros((7, o + 3))\n"
ALLOC_SEP = "".join(f"    {b} = np.zeros(o + 3)\n" for b in ("sxn", "sxo", "syn", "syo", "szn", "szo", "cum"))


def make(name, views, fastmath, note):
    s = orig[:doc_end] + f"\n\n# extractedMatrix/followup variant `{name}` of the hand-written file timed in job 4990441.\n{note}" + orig[doc_end:]
    # the parallel driver and the private-copy reduction go; deposit_serial takes their place
    a = s.index("@nb.njit(parallel=True, cache=True)\ndef deposit_chunks(")
    b = s.index("def warpx_esirkepov_deposition(")
    s = s[:a] + SERIAL.format(alloc=ALLOC_VIEWS if views else ALLOC_SEP,
                              bufs="sb" if views else "sxn, sxo, syn, syo, szn, szo, cum") + s[b:]
    c = s.index("    grid_bytes = 8 *")
    d = s.index("        ion_lev,\n", c)
    e = s.index("    for grid, p in zip(")
    s = s[:c] + "    deposit_serial(\n        Jx,\n        Jy,\n        Jz,\n" + s[d:e].rstrip() + "\n"
    if not views:
        s = once(s, "    red,\n    sb,\n):\n", "    red,\n    sxn,\n    sxo,\n    syn,\n    syo,\n    szn,\n    szo,\n    cum,\n):\n")
        s = once(s, "    sxn, sxo, syn, syo, szn, szo, cum = sb[0], sb[1], sb[2], sb[3], sb[4], sb[5], sb[6]\n", "")
    if fastmath:
        assert s.count("@nb.njit(cache=True)\ndef") == 6, s.count("@nb.njit(cache=True)\ndef")
        s = s.replace("@nb.njit(cache=True)\ndef", "@nb.njit(cache=True, fastmath={'nsz', 'contract'})\ndef")
    body = s[s.index("\nimport math"):]
    bad = [l for l in body.splitlines() if "prange" in l or "parallel=True" in l]
    assert not bad, bad
    (HERE / f"variants/{K}_{name}.py").write_text(s)


make("nbserial", True, False,
     "# Change: deposit_serial deposits every particle in order straight into Jx/Jy/Jz with one njit loop. The\n"
     "# private grid copies, their zeroing, the reduce_private pass and both parallel=True regions are gone.\n"
     "# deposit_particle and the shape-factor code are unchanged.\n")
make("nbnoviews", False, False,
     "# Changes: as `nbserial`, and deposit_particle takes its seven per-particle buffers as seven separate\n"
     "# 1-D arrays, allocated once, instead of the rows sb[0]..sb[6] of one 2-D array taken as views for every\n"
     "# particle. The arithmetic is unchanged.\n")
make("nbfastmath", False, True,
     "# Changes: as `nbnoviews`, and every @nb.njit(cache=True) gets fastmath={'nsz', 'contract'}, the LLVM\n"
     "# counterparts of GCC's -fno-signed-zeros -ffp-contract=fast.\n")
print("ok")


# ---- nbnort: the hand-written file unchanged, except that the per-particle functions compile without NRT ----
# (no reference counting of the arrays and views they touch). deposit_chunks, reduce_private and the driver keep
# NRT because they allocate.
s = orig[:doc_end] + ("\n\n# extractedMatrix/followup variant `nbnort` of the hand-written file timed in job 4990441.\n"
                      "# Change: shape_factor, shifted_shape_factor, safe_div, axis_factors and deposit_particle are compiled\n"
                      "# with _nrt=False, so Numba emits no reference counting (NRT_incref/NRT_decref) inside them. Nothing\n"
                      "# else changed: same parallel driver, private grids and arithmetic.\n") + orig[doc_end:]
assert s.count("@nb.njit(cache=True)\ndef") == 5
s = s.replace("@nb.njit(cache=True)\ndef", "@nb.njit(cache=True, _nrt=False)\ndef")
(HERE / f"variants/{K}_nbnort.py").write_text(s)
print("ok nbnort")

