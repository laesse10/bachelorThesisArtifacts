#!/usr/bin/env python3
"""Task 1: v2 (26a4f0cf) vs v1 (e2bceb68) matrix, cell by cell -> diff_vs_v1.csv.

A cell is FLAGGED when its min-of-k moved by more than 5% or its status changed. For every cell
(flagged or not) the timed source is compared across the two commits:

  c / cpp / fortran / numba : the emitted <k>_fp64.{c,cpp,f90} / <k>_numba_np.py
                              (v1: emitted_sources_v1/, regenerated at e2bceb68 and byte-identical
                              to the v1 bench tree; v2: emitted_sources/, snapshotted by the sweep)
  c_reference               : <k>_reference.c, git blob at each commit
  agent                     : the chosen candidate's sha256 (a different pick = a different source)

``source_change`` is identical / comments_only / preprocessed_identical / code / n.a. Comment lines
are // ! # and /* */ blocks plus the autogen marker; preprocessed_identical = the text differs but
``gcc -E -P`` of both is identical (e.g. a macro that expands to nothing on the host); only ``code``
means the compiler saw different input.
"""
import csv, hashlib, pathlib, re, statistics, subprocess

REPO = pathlib.Path(__file__).resolve().parent
V1 = REPO.parent
BENCH = pathlib.Path("/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2")
C1, C2 = "e2bceb68db2b32dc2203a6c6d4bc169dff291a49", "26a4f0cfc1540cead109dfc8d736f1c45263e390"
REL = "hpcagent_bench/benchmarks/loop_level_reasoning"
FILES = {"c": "_fp64.c", "cpp": "_fp64.cpp", "fortran": "_fp64.f90", "numba": "_numba_np.py"}


def load(p):
    return {(r["kernel"], r["representation"]): r for r in csv.DictReader(open(p))
            if r.get("preset", "M") == "M" and not r.get("flags_variant")}


def rsd(r):
    s = [int(x) for x in (r.get("time_ns_all") or "").split()]
    return round(100 * statistics.stdev(s) / statistics.fmean(s), 2) if len(s) > 1 else ""


def strip_comments(text, lang):
    if lang in ("c", "cpp", "c_reference"):
        text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
        text = re.sub(r"//[^\n]*", "", text)
    elif lang == "fortran":
        text = re.sub(r"![^\n]*", "", text)
    else:
        text = re.sub(r"#[^\n]*", "", text)
        text = re.sub(r'"""(.*?)"""', "", text, flags=re.S)
    return "\n".join(l.rstrip() for l in text.splitlines() if l.strip())


def compare(a, b, lang):
    if a is None or b is None:
        return "missing", "", "", ""
    ha, hb = hashlib.sha256(a.encode()).hexdigest()[:16], hashlib.sha256(b.encode()).hexdigest()[:16]
    if a == b:
        return "identical", ha, hb, 0
    ca, cb = strip_comments(a, lang), strip_comments(b, lang)
    import difflib
    nd = sum(1 for l in difflib.unified_diff(ca.splitlines(), cb.splitlines(), lineterm="", n=0)
             if l[:1] in "+-" and not l.startswith(("+++", "---")))
    if ca == cb:
        return "comments_only", ha, hb, nd
    if lang in ("c", "cpp", "c_reference"):
        pa, pb = preprocess(a, lang), preprocess(b, lang)
        if pa is not None and pa == pb:
            return "preprocessed_identical", ha, hb, nd
    return "code", ha, hb, nd


def preprocess(text, lang):
    """The translation unit as the compiler sees it (gcc -E -P, host: no __HIPCC__/__CUDACC__)."""
    import tempfile
    drv, ext = ("g++", ".cpp") if lang == "cpp" else ("gcc", ".c")
    with tempfile.NamedTemporaryFile("w", suffix=ext, delete=False) as fh:
        fh.write(text)
    std = "-std=c++23" if lang == "cpp" else "-std=c2x"
    p = subprocess.run([drv, "-E", "-P", std, fh.name], capture_output=True, text=True)
    pathlib.Path(fh.name).unlink()
    if p.returncode != 0:
        return None
    return "\n".join(l.rstrip() for l in p.stdout.splitlines() if l.strip())


def git_blob(commit, path):
    p = subprocess.run(["git", "-C", str(BENCH), "show", f"{commit}:{path}"], capture_output=True, text=True)
    return p.stdout if p.returncode == 0 else None


def agent_sha(r):
    m = re.search(r"sha256=([0-9a-f]+)", (r or {}).get("notes", ""))
    return m.group(1) if m else ""


def agent_rank(r):
    m = re.search(r"rank=(\d+)", (r or {}).get("notes", ""))
    return m.group(1) if m else ""


def main():
    v1, v2 = load(V1 / "results.csv"), load(REPO / "results.csv")
    rows = []
    for key in sorted(set(v1) | set(v2)):
        k, rep = key
        a, b = v1.get(key), v2.get(key)
        t1 = int(a["time_ns_min"]) if a and a.get("time_ns_min") else None
        t2 = int(b["time_ns_min"]) if b and b.get("time_ns_min") else None
        ratio = round(t2 / t1, 4) if (t1 and t2) else ""
        st1, st2 = (a or {}).get("status", "missing"), (b or {}).get("status", "missing")
        reasons = []
        if st1 != st2:
            reasons.append("status")
        if ratio != "" and abs(ratio - 1) > 0.05:
            reasons.append("time>5%")
        if rep in FILES:
            f = f"{k}{FILES[rep]}"
            s1 = (REPO / "emitted_sources_v1" / k / f)
            s2 = (REPO / "emitted_sources" / k / f)
            kind, h1, h2, nd = compare(s1.read_text() if s1.is_file() else None,
                                       s2.read_text() if s2.is_file() else None, rep)
            src = f
        elif rep == "c_reference":
            path = f"{REL}/{k}/{k}_reference.c"
            kind, h1, h2, nd = compare(git_blob(C1, path), git_blob(C2, path), "c_reference")
            if kind == "missing":
                kind = "n.a. (no _reference.c at either commit)"
            src = f"{k}_reference.c"
        else:  # agent
            s1, s2 = agent_sha(a), agent_sha(b)
            if not s1 and not s2:
                kind, h1, h2, nd = "n.a. (no agent submission)", "", "", ""
            else:
                kind = "identical" if s1 == s2 else "code (different candidate chosen)"
                h1, h2, nd = s1, s2, ""
            src = f"agent rank v1={agent_rank(a) or '?'} v2={agent_rank(b) or '?'}"
        explanation = ""
        if reasons:
            if kind.startswith("code"):
                explanation = "timed source changed between commits"
            elif kind in ("identical", "comments_only", "preprocessed_identical"):
                explanation = ("source identical for the compiler; change is harness/environment/noise"
                               + ("; status reclassified by v2 rule" if "status" in reasons and
                                  "v1 rule would record" in (b or {}).get("notes", "") else ""))
            else:
                explanation = kind
        rows.append({
            "kernel": k, "representation": rep, "flagged": "yes" if reasons else "no",
            "reasons": "+".join(reasons), "v1_status": st1, "v2_status": st2,
            "v1_min_ns": t1 or "", "v2_min_ns": t2 or "", "ratio_v2_over_v1": ratio,
            "v1_rsd_pct": rsd(a) if a else "", "v2_rsd_pct": rsd(b) if b else "",
            "source": src, "source_change": kind, "v1_source_sha": h1, "v2_source_sha": h2,
            "changed_code_lines": nd, "v1_compiler": (a or {}).get("compiler_version", "")[:40],
            "v2_compiler": (b or {}).get("compiler_version", "")[:40], "explanation": explanation,
            "v2_notes": (b or {}).get("notes", "")[:200],
        })
    with open(REPO / "diff_vs_v1.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)
    fl = [r for r in rows if r["flagged"] == "yes"]
    import collections
    print(f"{len(rows)} cells, {len(fl)} flagged; by source_change:",
          dict(collections.Counter(r["source_change"] for r in fl)))
    print("source changes over ALL cells:", dict(collections.Counter(r["source_change"] for r in rows)))


if __name__ == "__main__":
    main()
