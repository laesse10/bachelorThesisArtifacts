#!/usr/bin/env python3
"""Phase 4: one optimization-class label per roster kernel -> labels.csv.

`source_of_label` distinguishes a DECLARED label (read from the manifest's
`loop_level_reasoning.category`) from a DERIVED one (read off the kernel source because the
manifest declares none). A derived label is an interpretation and is marked as such.
"""
import csv, pathlib, re, yaml

REPO = pathlib.Path(__file__).resolve().parent
B = REPO / "bench/hpcagent_bench/benchmarks/loop_level_reasoning"

# Manifest category (free text) -> normalized class. The corpus vocabulary is its own; the
# task's expected list is a poor fit and is NOT forced onto it.
NORMALIZE = {
    "reductions": "reduction",
    "loop interchange": "loop_interchange",
    "indirect addressing": "indirect_addressing",
    "control flow": "control_flow",
    "scalar and array expansion": "scalar_expansion",
    "loop-carried dependence": "dependence_distance",
    "linear dependence": "linear_dependence",
    "packing": "packing",
    "node splitting": "node_splitting",
    "interprocedural data flow": "interprocedural_dataflow",
    "loop distribution": "loop_distribution",
    "recurrences": "recurrence",
}

# The 9 kernels whose manifest declares no category. Each label is read off the loop body in
# <kernel>_numpy.py; the rationale is recorded so the derivation is auditable, not asserted.
DERIVED = {
    "argmax_with_index":  ("reduction", "running max carrying both value and index (TSVC s315)"),
    "quasi_affine_reduce_odd": ("reduction", "sum over a strided (quasi-affine) index range"),
    "ext_break_capture":  ("control_flow", "find-first with a data-dependent early exit (TSVC s332)"),
    "ext_war_unit":       ("dependence_distance", "a[i]=a[i+1]+b[i]: anti-dependence at distance 1 (TSVC s121)"),
    "fuse_diamond":       ("loop_fusion", "one producer feeding two consumers across four loops"),
    "fuse_move_ifs":      ("loop_fusion", "two nests whose guards block fusion; needs guard hoisting"),
    "fuse_stencil_through_transient": ("loop_fusion", "non-pointwise vertical fusion through a transient"),
    "wf_diff_skew":       ("wavefront", "difference-diagonal wavefront; parallel front is a diagonal"),
    "wf_triangular":      ("wavefront", "triangular wavefront; parallel front is the i+j anti-diagonal"),
}

TSVC_RE = re.compile(r"TSVC ``(s\d+|v[a-z]+)``|^tsvc_2_(.+)$")


def tsvc_id(kernel, text):
    if kernel.startswith("tsvc_2_"):
        return kernel[len("tsvc_2_"):]
    m = re.search(r"TSVC ``(s\d+[a-z]*|v[a-z]+)``", text)
    return m.group(1) if m else ""


def main():
    rows = []
    for k in [l.strip() for l in open(REPO / "roster40.txt") if l.strip()]:
        y = yaml.safe_load(open(B / k / f"{k}.yaml"))
        llr = y.get("loop_level_reasoning", {}) or {}
        cat, opt = llr.get("category"), llr.get("optimization", "")
        src = (B / k / f"{k}_numpy.py").read_text()
        if cat:
            cls, source, desc = NORMALIZE.get(cat, cat.replace(" ", "_")), "declared", opt
        else:
            cls, why = DERIVED[k]
            source, desc = "derived", why
        rows.append({
            "kernel": k, "tsvc_id": tsvc_id(k, src), "optimization_class": cls,
            "description": desc, "source_of_label": source,
            "manifest_category": cat or "", "manifest_optimization": opt or "",
        })
    with open(REPO / "labels.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)

    import collections
    dist = collections.Counter(r["optimization_class"] for r in rows)
    decl = collections.Counter(r["source_of_label"] for r in rows)
    print(f"wrote labels.csv: {len(rows)} kernels, {len(dist)} classes")
    print(f"label provenance: {dict(decl)}\n")
    print(f"{'class':<26}{'n':>3}  share")
    for c, n in dist.most_common():
        print(f"{c:<26}{n:>3}  {100*n/len(rows):5.1f}%  {'#'*n}")
    print(f"\nclasses with <=2 members: {sum(1 for n in dist.values() if n <= 2)} of {len(dist)}")
    print(f"kernels in a <=2-member class: {sum(n for n in dist.values() if n <= 2)} of {len(rows)}")


if __name__ == "__main__":
    main()
