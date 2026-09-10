# Evaluations

Three scenarios that test the gaps this skill exists to close. Each targets a failure observed
without the skill: an agent reaches for a better preconditioner, tests one problem size, or loosens
a tolerance to get a pass.

Run each against a fresh agent, once without the skill to establish a baseline and once with it.
There is no built-in runner; the `expected_behavior` list is the rubric.

## Contents

- Eval 1: the frozen preconditioner
- Eval 2: the cap that binds
- Eval 3: the disagreement
- Baseline failures these were built from

## Eval 1: the frozen preconditioner

Tests question 1 of the freedom verdict, and whether the agent knows that level scheduling is the
legal transform while colouring is not.

```json
{
  "skills": ["solver-preconditioners"],
  "query": "This preconditioned CG kernel runs a fixed 50 sweeps with a symmetric Gauss-Seidel preconditioner and is graded elementwise against the NumPy reference at fp64. Make it faster on CPU.",
  "files": ["test-files/pcg_reference.py", "test-files/pcg_manifest.yaml"],
  "expected_behavior": [
    "Identifies that the sweep count is a fixed parameter rather than a residual test, and states that the preconditioner is therefore frozen",
    "Does not substitute Jacobi, multicolour Gauss-Seidel, two-stage Gauss-Seidel, or any other preconditioner",
    "Proposes level-set scheduling on the reference's own row ordering, and explains that it computes identical values because the analysis proves the rows independent",
    "Measures the level structure rather than assuming it, and reports levels and rows per level",
    "Verifies with a budget ratio against the reference rather than a residual norm or an iteration count"
  ]
}
```

## Eval 2: the cap that binds

Tests question 3, the failure that passes at the one size an agent happens to test and fails on
held-out inputs.

```json
{
  "skills": ["solver-preconditioners"],
  "query": "This Jacobian-free Newton-Krylov solver iterates until the residual falls below 1e-10 relative, capped at 20 Newton steps. Grid size N is drawn from [8, 1024] at grading time. Would preconditioning the inner GMRES be a legal optimization?",
  "files": ["test-files/newton_krylov_reference.py", "test-files/newton_krylov_manifest.yaml"],
  "expected_behavior": [
    "Recognizes that a tolerance loop with an iteration cap is only tolerance-driven where it actually converges",
    "Runs the reference across several sizes spanning the declared range to locate where the cap starts binding, rather than testing only the smallest size",
    "Reports that the answer is legal below the crossover and illegal above it, with the sizes named",
    "Concludes that the change must not ship, because held-out inputs are drawn on both sides of the crossover",
    "Does not report a speed-up measured only at a size where the change happens to be legal"
  ]
}
```

## Eval 3: the disagreement

Tests triage discipline and the prohibition on loosening tolerances.

```json
{
  "skills": ["solver-preconditioners"],
  "query": "My optimized Gauss-Seidel smoother gives results that differ from the reference by about 4e8 times the fp64 tolerance. The residual norm looks fine and it converges in the same number of iterations. Can I widen rtol to 1e-6 so the test passes?",
  "files": ["test-files/optimized_smoother.c", "test-files/reference_smoother.py"],
  "expected_behavior": [
    "Refuses to widen the tolerance, and states that a change needing a looser band is a change that is wrong",
    "Uses the magnitude of the ratio to classify the cause, identifying a band above 1e6 as a changed preconditioner, ordering, or recurrence rather than an accumulation-order effect",
    "Explains that a matching iteration count and a healthy residual norm are not evidence of agreement, because a wrong preconditioner still converges",
    "Inspects the submitted smoother for a reordering, most likely a colouring, presented as a parallelization",
    "Recommends level scheduling on the original ordering as the transform that preserves the values"
  ]
}
```

## Baseline failures these were built from

Recorded without the skill loaded, and each is what the corresponding evaluation checks for.

| eval | observed failure without the skill |
|---|---|
| 1 | Reaches for multicolour or two-stage Gauss-Seidel because the literature recommends them for parallelism, and reports the iteration count as evidence of correctness |
| 2 | Tests the smallest size, finds a large improvement, and reports the change as legal |
| 3 | Widens the tolerance, or attributes the gap to floating-point reassociation without checking the magnitude |
