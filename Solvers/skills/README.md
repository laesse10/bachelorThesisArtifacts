# Versions of the solver-preconditioners skill

Each directory is a complete, installable skill. Every `SKILL.md` carries `version:` and `status:`
in its frontmatter, so a file that gets copied somewhere else still says which version it is.

Version 3 is the one to read. Versions 1 and 2 are kept because the differences between them are
the point: each was a response to a specific problem with the one before.

| version | directory | shape | SKILL.md | other markdown | status |
|---|---|---|---|---|---|
| 1 | `v1-kernel-focused/` | single file, organised around the 14 named kernels | 587 lines | none | superseded |
| 2 | `v2-generalised/` | single file, general procedure, verdict by kernel pattern | 599 lines | none | installed locally |
| 3 | `v3-progressive-disclosure/` | SKILL.md plus 5 reference files and an evaluations file | 290 lines | 687 lines | **recommended** |

Only v3 stays under the 500-line limit for a SKILL.md body, because only v3 splits.

## What changed between versions

**v1 to v2** -- the spine was rebuilt. v1 opened with a fourteen-row verdict table naming specific
corpus kernels, so it was only usable on those kernels. v2 opens with a three-question test that
works on any solver kernel, and reduces the named kernels to a single section at the end that gives
the verdict by kernel pattern. Every measured number survived; each is now attached to the general
rule it supports rather than to the kernel it came from.

**v2 to v3** -- restructured to Anthropic's skill authoring best practices. v2 is 585 lines in one
file, over the 500-line limit. v3 splits it by progressive disclosure into a 290-line SKILL.md that
decides and navigates, plus five reference files loaded only when relevant. It also adds a copyable
six-step checklist, marks each step's degree of freedom, states the verification step as an explicit
feedback loop, replaces the thirteen-option ladder in the main file with a default-plus-escape-hatch
table, and bundles three evaluations.

**Corrections applied to v2 and v3, not to v1.** Two later passes removed content that does not
generalise: hardware-specific speed-up ratios and occupancy figures, which do not transfer between
targets, and named benchmarks and matrices, replaced by the patterns they exemplify. Convergence
and iteration counts were kept, since those are algorithmic. v1 is left as the historical record
and still contains both.

v3 also fixes a latent bug present in v1 and v2: the `description` frontmatter contains a colon
followed by a space, which YAML reads as a mapping key. It must be quoted or the skill fails to
load.

## Installing a version

Copy the directory contents over the installed skill:

```bash
cp -r versions/v2-generalised/. ~/.claude/skills/solver-preconditioners/
```

For v3 the reference and measurement files must travel with it, since its links are relative:

```bash
cp -r versions/v3-progressive-disclosure/. ~/.claude/skills/solver-preconditioners/
```

After installing, update the `status:` lines here so the live version is unambiguous.

## Measurements

v2 and v3 each carry their own copy of the seven validators behind every measured figure, so each
version stays self-contained and installable on its own. See `../README.md` for how to run them.
