"""Builds the split variant of v4 from the single page, so both ablation arms carry the same text.

    python3 build/make_split.py        (from Solvers/skills/v4-no-tools/)

Reads single/solver-preconditioners.md, writes split/solver-preconditioners-{verdict,frozen,ladder,verify,samples}.md.
Edit the single page, then rerun; never edit split/ by hand.
"""

import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parent.parent
SINGLE = ROOT / "single" / "solver-preconditioners.md"
OUT = ROOT / "split"

text = SINGLE.read_text()
body = text.split("---\n", 2)[2]          # drop the single page's frontmatter


def section(start, end=None):
    """The text from the line starting with `start` up to (not including) the line starting with `end`."""
    i = body.index("\n" + start) + 1
    j = body.index("\n" + end, i) + 1 if end else len(body)
    return body[i:j].rstrip().rstrip("-").rstrip() + "\n"


intro = body[body.index("The literature judges"):body.index("## Workflow")].rstrip() + "\n"
step1 = section("## Step 1", "## Step 2")
step2 = section("## Step 2", "## Step 3")
frozen = section("### If the preconditioner is frozen", "### If the preconditioner is yours")
yours = section("### If the preconditioner is yours", "## Step 4")
step4 = section("## Step 4", "## Step 5")
step5 = section("## Step 5", "## Failure catalogue")
catalogue = section("## Failure catalogue", "## Hard rules")
rules = section("## Hard rules", "## Samples")
samples = section("## Samples")

PAGES = {
    "verdict": "the first page to read for any solver kernel: whether the preconditioner may change",
    "frozen": "the preconditioner is frozen: make it fast without changing what it computes",
    "ladder": "the preconditioner may change: which one to choose",
    "verify": "checking agreement and triaging a wrong answer",
    "samples": "NumPy code of the runs behind every table, with outcome and whether it was good or bad",
}


def others(me):
    lines = [f"- `solver-preconditioners-{k}`: {v}" for k, v in PAGES.items() if k != me]
    return "## Other pages of this skill\n\n" + "\n".join(lines) + "\n"


def page(name, description, when, title, parts, me):
    front = f'---\nname: {name}\ndescription: "{description}"\nwhen: "{when}"\n---\n\n'
    return front + f"# {title}\n\n" + "\n".join(parts) + "\n" + others(me)


RECAP = ("A graded solver kernel is judged elementwise against one reference computation, not by "
         "iteration count. A preconditioner is **frozen** when the output is a fixed iterate (a fixed "
         "sweep, cycle or step count), when any graded output is a count, flag or history, or when a "
         "tolerance loop's iteration cap binds anywhere in the input range. The page "
         "`solver-preconditioners-verdict` decides which case you are in.\n")


def sub_ws(s, old, new):
    """Replace `old` in `s` regardless of how the page wrapped it: any run of whitespace matches any other."""
    pat = r"\s+".join(re.escape(w) for w in old.split())
    return re.sub(pat, lambda m: new, s)


def relink(s):
    s = sub_ws(s, "Step 2, then the ladder in\nStep 3.",
                  "classify the operator\n(below), then the page `solver-preconditioners-ladder`.")
    s = sub_ws(s, "the runs behind the tables above,", "the runs behind the tables of this skill,")
    s = sub_ws(s, "make the given preconditioner fast without changing what it\ncomputes (Step 3, frozen case).",
                  "make the given preconditioner fast without changing what it\ncomputes: the page "
                  "`solver-preconditioners-frozen`.")
    s = sub_ws(s, "see the level structure below.", "see the page `solver-preconditioners-frozen`.")
    s = sub_ws(s, "The NumPy code of those runs is in the Samples section at the end.",
                  "The NumPy code of those runs is on the page `solver-preconditioners-samples`.")
    s = sub_ws(s, "a transform from Step 3 is still untried.",
                  "a transform from the page `solver-preconditioners-frozen` is still untried.")
    s = sub_ws(s, "On the Newton example in Step 1:",
                  "On the Newton example on the page `solver-preconditioners-verdict`:")
    s = sub_ws(s, "in Step 3. Contrast", "on the page `solver-preconditioners-frozen`. Contrast")
    s = sub_ws(s, "must pass Step 4 before it is trusted.",
                  "must pass the check on the page `solver-preconditioners-verify` before it is trusted.")
    return s


def demote(s):
    return re.sub(r"^### ", "## ", s, flags=re.M)


def unstep(s):
    """Split pages carry only some steps, so drop the step numbers from their headings."""
    return re.sub(r"^## Step \d: ", "## ", s, flags=re.M)


workflow = ("## Workflow\n\n```\n"
            "- Freedom verdict    -- may the preconditioner change?          (this page)\n"
            "- Classify operator  -- symmetry, singularity, spectrum, spread  (this page)\n"
            "- Choose the transform  -- frozen: solver-preconditioners-frozen\n"
            "                        -- yours:  solver-preconditioners-ladder\n"
            "- Verify agreement   -- budget ratio across the input range      (solver-preconditioners-verify)\n"
            "- Account for cost   -- setup included, two problem sizes        (this page)\n"
            "```\n")

pages = {
    "verdict": page(
        "solver-preconditioners-verdict",
        "Decides whether a solver kernel's preconditioner may be changed or is frozen by what is graded, "
        "classifies the operator, and lists the moves that look like optimizations and are not.",
        "ALWAYS read this page first, before you parallelize or restructure a solver kernel: a Krylov, "
        "Newton, multigrid, Gauss-Seidel/SOR, triangular-solve, factorization, Lanczos or "
        "time-integration loop",
        "Preconditioning a graded solver kernel: the verdict",
        [relink(intro), workflow] + [unstep(relink(x)) for x in (step1, step2, step5)] + [catalogue, rules],
        "verdict"),
    "frozen": page(
        "solver-preconditioners-frozen",
        "Makes a frozen preconditioner fast without changing what it computes: level scheduling versus "
        "reordering, measured level structures, synchronization-free triangular solves, always-legal transforms.",
        "ALWAYS read before you parallelize a solver kernel whose preconditioner, sweep order or iteration "
        "count must stay as in the reference (most solver kernels): it lists what made such kernels fast",
        "Making a frozen preconditioner fast",
        [RECAP, relink(demote(frozen))], "frozen"),
    "ladder": page(
        "solver-preconditioners-ladder",
        "Chooses a preconditioner when the kernel allows changing it: defaults per operator class, the full "
        "ladder with what each family buys and costs, measured iteration counts, and library rules.",
        "the verdict says the preconditioner may change and you are choosing or tuning one",
        "Choosing a preconditioner",
        [RECAP, relink(demote(yours))], "ladder"),
    "verify": page(
        "solver-preconditioners-verify",
        "Verifies an optimized solver kernel against its reference with the budget ratio, and triages a "
        "disagreement to a named cause by the size of the ratio.",
        "ALWAYS read when a solver-kernel rewrite fails the grader's correctness check or disagrees with "
        "the reference, and before you submit one",
        "Verifying a solver kernel and triaging a wrong answer",
        [RECAP, unstep(relink(step4))], "verify"),
    "samples": page(
        "solver-preconditioners-samples",
        "NumPy formulations of runs already done on solver kernels -- preconditioner swaps in a fixed-sweep PCG, "
        "level-scheduled triangular solves, locating a binding iteration cap, a singularity check, batched "
        "reorthogonalization -- each with its outcome and whether the change was good or bad.",
        "you want a worked example of how a solver-kernel change was checked and how it turned out",
        "Samples: runs already done",
        [RECAP, relink(samples.split("\n", 2)[2])], "samples"),
}

OUT.mkdir(exist_ok=True)
for k, v in pages.items():
    (OUT / f"solver-preconditioners-{k}.md").write_text(v)
    print(k, len(v.splitlines()), "lines")
