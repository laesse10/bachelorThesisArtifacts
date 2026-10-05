#!/usr/bin/env python3
"""Write the follow-up source variants and their patches (variants/src/, variants/*.patch).

Each variant is the timed v2 source (v2/emitted_sources/, git 632952d, byte-identical to a fresh
autogen at 26a4f0cf) or the corpus' <k>_reference.c, with exactly the one change the experiment
names. Every edit is an exact-match replacement that must hit exactly once.

The emitted files carry the autogen marker on line 1. The harness regenerates a marked file and
leaves an unmarked one alone ("delete this line to keep local edits as a hand override"), so that
line is REPLACED by a comment, never deleted: the code and its line numbers are otherwise unchanged
up to the edit, and the opt-report line numbers stay comparable with v2's.
"""
import difflib, os, pathlib, sys

HERE = pathlib.Path(__file__).resolve().parent
V2SRC = pathlib.Path(os.environ.get("LLR40_V2_EMITTED",
                                    "/capstor/scratch/cscs/lhulsbergen/llr40_ref_632952d/llr40Matrix/v2/emitted_sources"))
BENCH = pathlib.Path(os.environ.get("LLR40_BENCH", "/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2-fu"))
LLR = BENCH / "hpcagent_bench/benchmarks/loop_level_reasoning"
OUT = HERE / "variants" / "src"
MARK = "// hpcagent_bench-autogen"
PYMARK = "# hpcagent_bench-autogen"


def unmark(text, name, lead):
    first, rest = text.split("\n", 1)
    assert first.startswith(lead), (name, first[:60])
    c = "//" if lead.startswith("//") else "#"
    return f"{c} follow-up variant {name}: autogen marker line replaced so the harness keeps this file as a hand override\n" + rest


def sub1(text, old, new, what):
    n = text.count(old)
    assert n == 1, f"{what}: expected exactly one match, got {n}"
    return text.replace(old, new)


def write(name, orig_path, text, orig_text):
    OUT.mkdir(parents=True, exist_ok=True)
    dst = OUT / name
    dst.write_text(text)
    diff = difflib.unified_diff(orig_text.splitlines(keepends=True), text.splitlines(keepends=True),
                                fromfile=f"a/{orig_path}", tofile=f"b/{orig_path}")
    patch = HERE / "variants" / f"{name}.patch"
    patch.write_text("".join(diff))
    print(f"wrote {dst.relative_to(HERE)}  {patch.relative_to(HERE)}")


def s2710():
    k = "tsvc_2_s2710"
    loop = "        for (int64_t i = 0; i < LEN_1D; ++i) {\n"
    use = "            if ((x[0] > 0.0)) {\n"
    for ext in (".c", ".cpp"):
        src = V2SRC / k / f"{k}_fp64{ext}"
        o = src.read_text()
        t = unmark(o, f"s2710_hoist ({ext[1:]})", MARK)
        t = sub1(t, loop, "        const double x0 = x[0];\n" + loop, f"s2710{ext} loop")
        t = sub1(t, use, "            if ((x0 > 0.0)) {\n", f"s2710{ext} x[0]")
        assert t.count("x[0]") == 1   # only the hoisted read is left
        write(f"{k}_fp64.hoist{ext}", f"cpp_backend/{k}_fp64{ext}", t, o)
    ref = LLR / k / f"{k}_reference.c"
    o = ref.read_text()
    t = sub1(o, "\n  for (int64_t i = 0; i < LEN_1D; ++i) {\n",
             "\n  const double x0 = x[0];\n  for (int64_t i = 0; i < LEN_1D; ++i) {\n", "s2710 ref loop")
    t = sub1(t, "      if (x[0] > 0.0) {\n", "      if (x0 > 0.0) {\n", "s2710 ref x[0]")
    assert t.count("x[0]") == 1
    write(f"{k}_reference.hoist.c", f"{k}_reference.c", t, o)


def versioned_k1():
    k = "versioned_distance_update"
    src = V2SRC / k / f"{k}_fp64.c"
    o = src.read_text()
    t = unmark(o, "vdu_k1_scalar (c)", MARK)
    orig_loop = ("        for (int64_t i = K; i < LEN_1D; ++i) {\n"
                 "          a[i] = ((0.75 * a[(i - K)]) + (b[i] * c[i]));\n"
                 "        }\n")
    k1 = ("        if (K == 1) {\n"
          "          double prev = a[0];\n"
          "          for (int64_t i = 1; i < LEN_1D; ++i) {\n"
          "            prev = ((0.75 * prev) + (b[i] * c[i]));\n"
          "            a[i] = prev;\n"
          "          }\n"
          "        } else {\n"
          + orig_loop.replace("\n        ", "\n          ").replace("        for", "          for", 1) +
          "        }\n")
    t = sub1(t, orig_loop, k1, "vdu loop")
    write(f"{k}_fp64.k1scalar.c", f"cpp_backend/{k}_fp64.c", t, o)


def wf_west():
    k = "wf_triangular"
    src = V2SRC / k / f"{k}_numba_np.py"
    o = src.read_text()
    t = unmark(o, "wf_west_scalar (numba)", PYMARK)
    old = ("    for i in range(1, LEN_2D):\n"
           "        for j in range(i, LEN_2D):\n"
           "            a[i, j] = a[i, j] + a[i - 1, j] + a[i, j - 1]\n")
    new = ("    for i in range(1, LEN_2D):\n"
           "        west = a[i, i - 1]\n"
           "        for j in range(i, LEN_2D):\n"
           "            west = a[i, j] + a[i - 1, j] + west\n"
           "            a[i, j] = west\n")
    t = sub1(t, old, new, "wf loop")
    write(f"{k}_numba_np.westscalar.py", f"{k}_numba_np.py", t, o)


if __name__ == "__main__":
    for f in (s2710, versioned_k1, wf_west):
        f()
    sys.exit(0)
