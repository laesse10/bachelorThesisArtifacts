#!/usr/bin/env python3
"""Write opt_findings.md (task 2) from opt_reports/, opt_findings_evidence/ and results.csv.

Every citation is looked up here, never typed: `report:N` is line N of that report file, and
asm citations give the instruction found in the kernel's disassembly. Times are min-of-k from
the v2 matrix. The interpretation is written as prose and depends only on the facts printed
next to it.
"""
import csv, pathlib, re

REPO = pathlib.Path(__file__).resolve().parent
OR, EV = REPO / "opt_reports", REPO / "opt_findings_evidence"
T = {(r["kernel"], r["representation"]): r for r in csv.DictReader(open(REPO / "results.csv"))
     if r["preset"] == "M" and not r["flags_variant"]}
LAB = {r["kernel"]: r["optimization_class"] for r in csv.DictReader(open(REPO / "labels.csv"))}


def ms(k, r):
    v = T.get((k, r), {}).get("time_ns_min")
    return f"{int(v)/1e6:.1f} ms" if v else "--"


def rep(k, r):
    for p in sorted((OR / k / r).glob("*.optreport.txt")):
        return p
    return None


def asm(k, r):
    for p in sorted((OR / k / r).glob("*.s.txt")):
        return p
    return None


def cite(path, pattern, first=True):
    """'path:N: text' for the first (or every) line matching pattern."""
    if path is None or not path.is_file():
        return []
    out = []
    for i, l in enumerate(path.read_text().splitlines(), 1):
        if re.search(pattern, l):
            out.append(f"`{path.relative_to(REPO)}:{i}`: `{l.strip()[:150]}`")
            if first:
                break
    return out


def count(path, pattern):
    return sum(1 for l in path.read_text().splitlines() if re.search(pattern, l)) if path and path.is_file() else 0


L = []
w = L.append
w("# Optimisation-report findings (v2, task 2)\n")
w("Reports: `opt_reports/<kernel>/<column>/<source>.optreport.txt` (the first line is the exact compile "
  "argv: the sweep's harness compile line plus `-fopt-info-vec-optimized -fopt-info-vec-missed "
  "-fopt-info-loop-optimized`), and the kernel's disassembly in `<source>.s.txt`. Citations are "
  "`file:line`. Explanatory compile-only experiments, each changing one thing against that compile "
  "line, are in `opt_findings_evidence/` (`opt_evidence.sh`). Times are v2 min-of-k at preset M.\n")
w("Toolchain note that matters for every C-vs-C++/Fortran comparison below: the C columns are built "
  "by **gcc 14.2.0** (harness floor for `-std=c23`), and C++ and Fortran by **13.3.1**.\n")

# ---------------------------------------------------------------- Q1
w("## 1. `loop_interchange` and `loop_distribution`: did GCC interchange or distribute anywhere?\n")
LI = sorted(k for k, c in LAB.items() if c in ("loop_interchange", "loop_distribution"))
cols = ["c", "c_reference", "cpp", "fortran", "agent"]
w("(`vec` = number of 'loop vectorized' remarks in that report.)\n")
w("**No.** In none of the 6 kernels, in none of the compiled columns, does any report contain an "
  "interchange or distribution remark. With `-fopt-info-loop-optimized`, GCC reports a successful "
  "interchange ('loops interchanged in loop nest') or distribution ('Loop N distributed'):\n")
w("| kernel | class | " + " | ".join(f"`{c}`" for c in cols) + " | agent vs fastest non-agent column |")
w("|---|---|" + "---|" * len(cols) + "---|")
for k in LI:
    cells = []
    for c in cols:
        p = rep(k, c)
        if p is None:
            cells.append("n/a"); continue
        ni, nd = count(p, r"(?i)interchang"), count(p, r"(?i)distribut")
        nv = count(p, r"optimized: loop vectorized")
        cells.append(f"interch {ni}, distrib {nd}, vec {nv}")
    comp = [T.get((k, c), {}).get("time_ns_min") for c in ("c", "c_reference", "cpp", "fortran", "numba")]
    comp = [int(x) for x in comp if x]
    ag = T.get((k, "agent"), {}).get("time_ns_min")
    ratio = f"{min(comp)/int(ag):.1f}x faster ({ms(k,'agent')} vs {min(comp)/1e6:.1f} ms)" if ag and comp else "no agent cell"
    w(f"| `{k}` | {LAB[k]} | " + " | ".join(cells) + f" | {ratio} |")
w("")
w("What GCC reports instead, autogen C and Fortran:\n")
for k in LI:
    w(f"- `{k}`:")
    for c in ("c", "fortran"):
        for s in cite(rep(k, c), r"not vectorized:|not suitable", first=True):
            w(f"  - {c}: {s}")
w("")
f = next(iter(sorted(EV.glob("s231/*linterchange"))), None)
summ = (EV / "s231" / "summary.txt").read_text().strip().replace("\n", "; ") if (EV / "s231" / "summary.txt").is_file() else ""
w(f"`-fopt-info` reports only SUCCESSFUL interchanges, so for the cleanest perfect nest (`tsvc_2_s231`, "
  f"dependence vector (0,1), interchange legal) the interchange pass was dumped "
  f"(`opt_findings_evidence/s231/`, `-fdump-tree-linterchange-details`): {summ}. The pass never "
  "took up the nest as a candidate: it printed no analysis and no rejection. The dump does not "
  "say why.\n")
w("**What the agents did differently.** Every agent submission does the restructuring by hand, and "
  "GCC then vectorises the unit-stride inner loop it was given:\n")
AG = {
    "tsvc_2_s231": "interchanged: `j` outer, `i` inner, so the inner loop is unit-stride and carries no dependence (`aa[j][i] = aa[j-1][i] + bb[j][i]`)",
    "tsvc_2_s233": "distributed the two `j`-loops into separate nests and interchanged each to an `i`-inner, unit-stride loop",
    "tsvc_2_s235": "(Fortran) split the `a(i)` update into its own `!$omp simd` loop (distribution) and interchanged the 2-D update to `j` outer, `!$omp do simd` over `i`",
    "tsvc_2_s1232": "(Fortran) linearised the triangular update into a contiguous 1-D run `aa(k) = bb(k) + cc(k)` per column (column-major), i.e. unit stride",
    "tsvc_2_s2275": "flattened the 2-D update into one 1-D loop over all `LEN_2D^2` elements and moved the `a[i]` statement to its own loop (distribution); both loops vectorise",
}
for k in LI:
    if k in AG:
        cs = cite(rep(k, "agent"), r"optimized: loop vectorized", first=True)
        w(f"- `{k}`: {AG[k]}. {cs[0] if cs else ''}")
w(f"- `tsvc_2_s2233` has no agent cell. GCC vectorises its second inner loop in every compiled "
  f"column ({(cite(rep('tsvc_2_s2233', 'c'), r'optimized: loop vectorized') or [''])[0]}), which is why "
  "this kernel looked like an outlier in v1's normalisation.\n")
ags = []
for k in LI:
    comp = [int(T[(k, c)]["time_ns_min"]) for c in ("c", "c_reference", "cpp", "fortran", "numba") if T.get((k, c), {}).get("time_ns_min")]
    if T.get((k, "agent"), {}).get("time_ns_min"):
        ags.append(min(comp) / int(T[(k, "agent")]["time_ns_min"]))
w(f"So the {min(ags):.1f}-{max(ags):.1f}x agent advantage on these classes is restructuring that GCC 13/14 at `-O3` does not do "
  "on these sources: its interchange pass does not consider the nests, and its vectoriser rejects "
  "them as outer-loop, multi-inner-loop or strided forms.\n")

# ---------------------------------------------------------------- Q2
k = "tsvc_2_s3111"
w(f"## 2. `tsvc_2_s3111`: why is `fortran` {int(T[(k,'fortran')]['time_ns_min'])/int(T[(k,'c')]['time_ns_min']):.1f}x slower?\n")
w(f"min-of-k: c {ms(k,'c')}, c_reference {ms(k,'c_reference')}, cpp {ms(k,'cpp')}, **fortran {ms(k,'fortran')}**, "
  f"numba {ms(k,'numba')}, agent {ms(k,'agent')}. (The task's '4.4x' is the deviation from the reduction "
  "class geomean in `outliers.csv`. The raw ratio to `c` is the one in the heading.)\n")
w("The kernel is a conditional sum, `if a(i) > 0: sum += a(i)`. Its input is uniform on [-1000, 1000), "
  "so exactly half the elements are positive, and `LEN_1D` = 2e8.\n")
for c in ("c", "cpp"):
    for s in cite(rep(k, c), r"optimized: loop vectorized"):
        w(f"- {c}: {s}")
w(f"  The loop is a predicated SVE compare feeding an ordered, predicated add, with no branch: "
  f"{(cite(asm(k,'c'), r'fcmgt') or [''])[0]}, {(cite(asm(k,'c'), r'fadda') or [''])[0]}.")
for s in cite(rep(k, "fortran"), r"missed: not vectorized"):
    w(f"- fortran: {s}")
w(f"  The loop stays scalar with one data-dependent branch per element: "
  f"{(cite(asm(k,'fortran'), r'fcmpe') or [''])[0]}, {(cite(asm(k,'fortran'), r'b\.gt') or [''])[0]}.")
w("")
w("**Cause: the translator's parentheses.** The emitted Fortran is `sum_val = (sum_val + a((i_l0) + 1))`. "
  "In Fortran, parentheses are semantically binding: gfortran must keep them, as a `PAREN_EXPR` in "
  "GIMPLE, and the vectoriser does not recognise a reduction through it. From the vectoriser dump "
  "(`opt_findings_evidence/s3111/orig.vect.excerpt.txt`):\n")
w("```")
w((EV / "s3111" / "orig.vect.excerpt.txt").read_text().strip())
w("```")
w(f"Removing only those parentheses (`noparen.diff`) makes gfortran vectorise the loop: "
  f"{(EV / 's3111' / 'summary.txt').read_text().strip().splitlines()[1]}. A 50%-taken branch over "
  f"2e8 elements mispredicts about half the time; {int(T[(k,'fortran')]['time_ns_min'])/2e8*3.26:.1f} cycles/element at 3.26 GHz is "
  "consistent with that (a ~20-cycle penalty at a 50% miss rate), though branch misses were not "
  "counted here. In C the same parentheses "
  "carry no meaning and create no `PAREN_EXPR`.\n")

# ---------------------------------------------------------------- Q3
k = "tsvc_2_s316"
w(f"## 3. `tsvc_2_s316`: why are `c` and `c_reference` about {int(T[(k,'c')]['time_ns_min'])/int(T[(k,'cpp')]['time_ns_min']):.1f}x slower than `cpp` and `fortran`?\n")
w(f"min-of-k: c {ms(k,'c')}, c_reference {ms(k,'c_reference')}, cpp {ms(k,'cpp')}, fortran {ms(k,'fortran')}. "
  "The kernel is a running minimum, `if a[i] < x: x = a[i]`, with `LEN_1D` = 2e8. No column vectorises "
  "it, and the C and C++ sources are identical.\n")
w(f"- c (gcc 14.2.0) if-converts the update into a branch-free select: "
  f"{(cite(asm(k,'c'), r'fcsel') or [''])[0]}. Both the compare and the select sit on the loop-carried "
  "chain through `x`, so each element pays their latency: about 4.5 cycles/element at 3.26 GHz.")
w(f"- cpp and fortran (13.3.1) keep a branch to an out-of-line move: "
  f"{(cite(asm(k,'cpp'), r'b\.gt') or [''])[0]}. A running minimum over random data almost never "
  "updates, so the branch predicts well and nothing serialises the loop. It runs at about 20 GB/s, "
  "the single-core bandwidth fast state of protocol.md section 4.")
w(f"- **Cause: compiler version, not language** (`opt_findings_evidence/s316/summary.txt`, the same "
  f"sources rebuilt with the other version): {'; '.join((EV / 's316' / 'summary.txt').read_text().split(chr(10))).strip('; ')}. "
  "This was already true in v1, whose C columns were also gcc 14.2.0.\n")

# ---------------------------------------------------------------- Q4
w("## 4. Reductions: does gfortran vectorise reductions that gcc-C does not (reassociation)?\n")
RED = sorted(k for k, c in LAB.items() if c == "reduction")
w("**No, the opposite.** gfortran vectorises none of the 10. gcc-C/C++ vectorise exactly two, and only "
  "as in-order (non-reassociating) SVE `fadda` reductions. The fortran/c ratio column shows the result:\n")
w("| kernel | c: vec / fadda | cpp: vec / fadda | fortran: vec / fadda / reassoc. vector fadd | fortran/c min-of-k |")
w("|---|---|---|---|---:|")
for k in RED:
    def vf(c):
        p, a = rep(k, c), asm(k, c)
        return (count(p, r"optimized: loop vectorized"), count(a, r"\sfadda\s"),
                count(a, r"\sfadd\s+(z\d+\.d|v\d+\.2d)"))
    c_, p_, f_ = vf("c"), vf("cpp"), vf("fortran")
    fc = T.get((k, "fortran"), {}).get("time_ns_min"); cc = T.get((k, "c"), {}).get("time_ns_min")
    w(f"| `{k}` | {c_[0]} / {c_[1]} | {p_[0]} / {p_[1]} | {f_[0]} / {f_[1]} / {f_[2]} | "
      f"{int(fc)/int(cc):.2f} |" if fc and cc else f"| `{k}` | | | | -- |")
w("")
w("- gfortran *would* reassociate. With the parentheses removed it vectorises `s3111` into vector "
  "partial sums plus a horizontal `faddp` (`opt_findings_evidence/s3111/summary.txt`), which is the "
  "reassociation licence protocol.md section 5.1 warns about. But the translator writes every "
  "accumulation in parentheses, and parentheses forbid it. The protocol's '3.1x Fortran advantage on "
  "reductions' therefore does not occur in this matrix. Where C and Fortran differ on a reduction, "
  "Fortran is the slower one.")
for s in cite(rep("tsvc_2_s311", "fortran"), r"unsupported use in stmt"):
    w(f"- The same failure on the plain sum `tsvc_2_s311`: {s}. Here gcc's in-order `fadda` "
      f"({(cite(asm('tsvc_2_s311','c'), r'fadda') or [''])[0]}) buys nothing: c {ms('tsvc_2_s311','c')} vs "
      f"scalar fortran {ms('tsvc_2_s311','fortran')}, because an in-order reduction is still latency-bound. "
      f"Only the agent, whose OpenMP `reduction` clause licenses reassociation, vectorises it with "
      f"partial sums: {count(asm('tsvc_2_s311','agent'), r'\sfadd\s+(z\d+\.d|v\d+\.2d)')} vector `fadd`, "
      f"{ms('tsvc_2_s311','agent')}.")
w(f"- The parentheses also block FMA contraction in scalar Fortran. `scan_affine_decay` "
  f"(`((c*y) + x)`) compiles to `fmul`+`fadd` in Fortran and `fmadd` in C "
  f"(`opt_findings_evidence/scan_affine_decay/summary.txt`). On a loop-carried recurrence that is "
  f"3+2 vs 4 cycles: fortran {ms('scan_affine_decay','fortran')} vs c {ms('scan_affine_decay','c')}.")
w(f"- In vectorised Fortran code the parentheses do NOT stop contraction: `tsvc_2_s115`'s Fortran "
  f"body uses a fused vector `fmls` (`opt_findings_evidence/s115/summary.txt`: "
  f"{', '.join(' '.join(l.split()[:2]) for l in (EV / 's115' / 'summary.txt').read_text().splitlines() if l.strip())}"
  " -- count, instruction; the scalar peel/tail uses separate `fmul`/`fsub`). "
  "That is why its Fortran column is `incorrect` with `-ffp-contract=fast`, like the C columns, and "
  "`ok` with `-ffp-contract=off`.\n")
(REPO / "opt_findings.md").write_text("\n".join(L) + "\n")
print(f"wrote opt_findings.md ({len(L)} lines)")
