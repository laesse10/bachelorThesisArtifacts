# Triage: when your change does not agree

An out-of-band result has several causes with very different remedies, and they all look like "the
numbers are wrong". Classify before you fix.

## Contents

- The ratio band identifies the cause
- The seven causes
- Remedies that are never allowed
- Defects in the reference itself

## The ratio band identifies the cause

Compute the budget ratio first. It is the strongest discriminator available.

```python
RTOL, ATOL = 1e-9, 1e-11                      # fp64
def budget(got, ref):
    return float(np.max(np.abs(got - ref) / (ATOL + RTOL * np.abs(ref))))
```

| ratio | almost certainly | what to do |
|---|---|---|
| 1 to ~10 | tolerance-boundary reassociation, or a marginal accumulation order | tighten the accumulation, not the tolerance |
| ~10 to 1e3 | a genuine numerical bug: an index, a boundary, a missed term | debug it as a bug |
| 1e3 to 1e6 | a different but convergent algorithm, or a changed step count | you changed the mathematics |
| above 1e6 | a different preconditioner, ordering, or recurrence | it was frozen |

## The seven causes

Only some are yours.

1. **Frozen-preconditioner violation.** You changed what the recurrence computes.
2. **Reordering mistaken for scheduling.** You permuted rows rather than grouping proved-independent
   ones.
3. **Cap-boundary flip.** Agreement holds at your test size and fails at another because a tolerance
   loop's iteration cap binds on one side. The tell is a small ratio at small sizes and an enormous
   one at large sizes.
4. **Discrete-output drift.** The floating-point output agrees and a count does not.
5. **Genuine implementation bug.** Index, boundary, sign, missed term, uninitialized scratch.
6. **Accumulation order.** Real but small. A tree reduction against a sequential reference drifts
   like sqrt(n). If this is the whole problem the ratio is O(1) to O(10), never more.
7. **Environment or toolchain.** Fast-math reassociation, an FMA contraction difference, a different
   BLAS bound at load time, threading nondeterminism in a reduction.

Investigate far enough to name which one it is before changing code.

## Remedies that are never allowed

- **Never loosen a tolerance to obtain a pass.** Tolerances come from the datatype and the operation
  structure. A change that needs a looser band is a change that is wrong.
- **Never reshape a correct kernel to hide a backend or toolchain failure.** Fix the toolchain, or
  report the limitation as independent of the kernel. Cause 7 is not a reason to change cause-free
  mathematics.

## Defects in the reference itself

Distinguish a defect you introduced from one that was already there. Signs to investigate rather
than route around: a near-zero pivot, a documented type the data contradicts, an operator claimed
definite that measures singular.

Do not silently reproduce it. Do not silently repair it. State which of the three you did:

1. the reference's behavior is well defined and you preserved it;
2. the behavior is demonstrably erroneous and you have evidence;
3. you deliberately corrected or excluded it, and here is why.
