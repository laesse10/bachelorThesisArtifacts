---
name: solver-preconditioners-verify
description: "Verifies an optimized solver kernel against its reference with the budget ratio, and triages a disagreement to a named cause by the size of the ratio."
when: "ALWAYS read when a solver-kernel rewrite fails the grader's correctness check or disagrees with the reference, and before you submit one"
---

# Verifying a solver kernel and triaging a wrong answer

A graded solver kernel is judged elementwise against one reference computation, not by iteration count. A preconditioner is **frozen** when the output is a fixed iterate (a fixed sweep, cycle or step count), when any graded output is a count, flag or history, or when a tolerance loop's iteration cap binds anywhere in the input range. The page `solver-preconditioners-verdict` decides which case you are in.

## Verify agreement

**The budget ratio** of one graded output is

```
max over elements of  |got - ref| / (atol + rtol * |ref|)        <= 1.0 passes
```

with the task's own `rtol` and `atol`. Report error as this multiple, never as "close".

1. **The grader's correctness check is the verification.** If the harness offers a preview of the
   grade, run it on every candidate: it is the only check that runs your code the way it is graded.
   A local comparison against the reference can help bisect a failure; it does not replace the
   grader.
2. **Cover the range.** A preview runs a few fixed inputs; graded inputs are drawn from the whole
   declared range. Anything that depends on size (a binding cap, a level structure, a step count)
   must be argued for the whole range, not read off the preview sizes.
3. **If a result is out of band, classify it by the ratio before changing anything:**

| ratio | almost certainly | what to do |
|---|---|---|
| 1 to ~10 | reassociation at the tolerance boundary, marginal accumulation order | tighten the accumulation, not the tolerance |
| ~10 to 1e3 | a genuine bug: an index, a boundary, a missed term | debug it as a bug |
| 1e3 to 1e6 | a different but convergent algorithm, or a changed step count | you changed the mathematics |
| above 1e6 | a different preconditioner, ordering or recurrence | it was frozen |

The causes, only some of them yours: a frozen-preconditioner violation; reordering mistaken for
scheduling; a **cap-boundary flip** (small ratio at small sizes, enormous at large ones); a graded
count that drifted while the floats agree; a genuine bug (index, boundary, sign, missed term,
uninitialized scratch); accumulation order (O(1) to O(10), never more); the toolchain (fast-math
reassociation, FMA contraction, a different library bound at load time, a non-deterministic threaded
reduction). Name the cause before fixing it.

> **MUST NOT** loosen a tolerance, lower an iteration cap or shrink a problem to obtain a pass.
> A change that needs a looser band is a change that is wrong.

> **MUST NOT** reshape correct mathematics to hide a toolchain failure. Fix the toolchain.

**Do not lose a good version.** Three common ways to lose a good result: submitting a version the
grader never checked, which then fails on held-out inputs; ending after a diagnostic experiment has
overwritten the best file, so a slower version is graded; and stopping while untried transforms
remain. Check every candidate with the grader, keep the best checked version in the file the grader
reads (experiments go in other files), submit only a version the grader has already passed, and keep
going while a transform from the page `solver-preconditioners-frozen` is still untried.

**Defects in the reference.** A near-zero pivot, a documented type the data contradicts, an operator
claimed definite that measures singular. Do not silently reproduce it and do not silently repair it:
preserve well-defined behavior, and say so if you deliberately deviate.

## Other pages of this skill

- `solver-preconditioners-verdict`: the first page to read for any solver kernel: whether the preconditioner may change
- `solver-preconditioners-frozen`: the preconditioner is frozen: make it fast without changing what it computes
- `solver-preconditioners-ladder`: the preconditioner may change: which one to choose
- `solver-preconditioners-samples`: NumPy code of the runs behind every table, with outcome and whether it was good or bad
