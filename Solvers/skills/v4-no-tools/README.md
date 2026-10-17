# v4: no tools, two packagings

Version 4 of the solver-preconditioning skill, written for a harness that hands the agent skill
**pages**: plain files on disk that the task prompt lists with a trigger (`when:`) and that the agent
opens with Read. The agent's only feedback is the harness's own grading and profiling calls, so
nothing on these pages tells the agent to run a script.

It exists in two packagings with the same text, for an ablation (one large page against several
small ones).

| variant | files to ship | pages | lines |
|---|---|---|---|
| single | `single/solver-preconditioners.md` | 1 | 565 |
| split | `split/solver-preconditioners-{verdict,frozen,ladder,verify,samples}.md` | 5 | 65-207 each |

Each page carries `name`, `description` and `when` in its frontmatter, the format of the harness's
own skill pages. In the split variant every page has its own trigger and stands on its own: each
opens with a two-sentence recap of the freedom verdict and ends with the list of the other pages,
named by their `name`, so an agent that reads only one page is not left with dead links.

## What changed from v3

- **No tool calls.** v3 told the agent to run `measurements/*.py`. Those scripts carried absolute
  cluster paths and a seeded matrix cache, and an agent running them learns nothing graded. They are
  gone from the pages. Their results stay, framed as runs already done: what was tried, the outcome,
  and whether it was good or bad. A Samples section (single) or page (split) shows the NumPy code of
  six of those runs as patterns to read and translate, not to call.
- **Verification through the grader.** The budget ratio is defined as a formula; the grader's
  correctness check, run on every candidate, is the verification. A local comparison only bisects.
  Size-dependent properties (a binding cap, a level structure) must be argued over the whole declared
  input range, because previews run a few fixed inputs and graded inputs are drawn from the whole
  range.
- **Timing through the grader.** Timings come from the harness on the graded machine; local timings
  on another node do not transfer.
- **One page instead of SKILL.md plus five reference files** (single variant). v3 split the text to
  stay under the 500-line limit for a SKILL.md body that Claude Code's skill loader applies. A
  harness that delivers one page per trigger never ships the reference files, and every Read is
  re-billed on each later turn as cached input, so the split bought nothing there.
- **Library tables cut to rules.** The CPU/CUDA/HIP library table and the linking mechanics are
  gone: what a build links is the harness's decision. Kept: link a routine that computes the same
  mathematics if the build provides it; read frameworks for the algorithm, do not submit their
  answer.
- **`evaluations.md` is not shipped.** It is for evaluating the skill, not for the agent; the v3
  copy stays the reference for the three scenarios.
- **Transforms that make frozen solver kernels fast** (added for performance): a table by loop
  structure (temporal blocking, asynchronous wavefronts, one persistent thread team, blocked BLAS-3,
  SIMD across independent instances, streaming layouts, multigrid bookkeeping, lean serial
  factorization), with why each pays, plus a rule on not losing the best checked version.
- **Imperative triggers** ("ALWAYS read ... before you parallelize"), in the style of the harness's
  own solver page, which glm53 and qwen38 read in 11 of 14 and 15 of 21 episodes. oss120b read that
  page in 2 of 16 despite the same wording, so a trigger alone does not guarantee a read.
- **No benchmark data.** Every number on the pages was re-measured on operators and problems the
  benchmark does not contain (a 2-D 9-point grid operator with random edge weights, its graph
  Laplacian, `-Lap(u) + u^3 = f`, a random dense matrix), by `build/examples.py`. The sample code is
  the code that produced the numbers. The transform table lists techniques by loop structure, without
  speedups from benchmark runs. v3's numbers came from the benchmark's own operators and are gone.
  Two v3 claims did not survive re-measurement and were dropped: contiguous block 64 is not worse
  than block 8 here, and two-stage SGS needs about 10 inner sweeps, not 3, to match SGS.

## Building the split variant

The numbers come from `build/examples.py` (numpy and scipy, about a minute):

```
python3 build/examples.py            # or name sections: levels subst iters spectrum cap lanczos qr
```

The split pages are generated from the single page so the two arms cannot drift apart:

```
cd Solvers/skills/v4-no-tools && python3 build/make_split.py
```

Edit `single/solver-preconditioners.md`, then rerun. Never edit `split/` by hand. `build/` is not
shipped.
