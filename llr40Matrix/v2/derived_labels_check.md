# Derived labels: hand check against the kernel source

`labels.csv` declares 31 labels from the manifest's `loop_level_reasoning.category`. The other
9 kernels' manifests declare no category, so `build_labels.py` gives them a label derived from
the loop body (`DERIVED`). Each derived label is checked below against `<kernel>_numpy.py` at
`26a4f0cf`. Where the kernel is an `ext_`/renamed form of a TSVC loop, the corpus's own manifest
for that TSVC original is cited as a second opinion, because it is the corpus's label rather
than ours.

| kernel | derived label | verdict | justification (one line) |
|---|---|---|---|
| `argmax_with_index` | reduction | **agree** | Running max that carries value and index (`if a[i] > x: x=a[i]; idx=i`); TSVC `s315`, which the corpus labels `reductions`. |
| `quasi_affine_reduce_odd` | reduction | **agree** | `out[0] += a[i]` over `range(1, LEN_1D, 2)`: a strided sum reduction. |
| `ext_break_capture` | control_flow | **disputable** | A find-first with a data-dependent `break`. Control flow is the obstacle, but the corpus labels its original TSVC `s332` as `search loops`, a category the normaliser has no class for. |
| `ext_war_unit` | dependence_distance | **disputable** | `a[i] = a[i+1] + b[i]` is a WAR at distance 1, but the corpus labels its original TSVC `s121` as `induction variables`. Under that label the roster would have one `induction_variable` member, not zero. |
| `fuse_diamond` | loop_fusion | **agree** | Four loops: one producer `t=a*a` feeds two consumers `u`, `v`, then `out=u*v`, a fusion target with a diamond-shaped dataflow. |
| `fuse_move_ifs` | loop_fusion | **agree** | Two 2-D nests under different guards (`cond[i] > 0`, `K > 0`) whose fusion needs the guards moved inward. The guards make `control_flow` a defensible second label. |
| `fuse_stencil_through_transient` | loop_fusion | **agree** | A 3-point stencil producer into `tmp`, then a consumer reading `tmp[i]` and `tmp[i+1]`: vertical fusion needs a one-element offset correction. |
| `wf_diff_skew` | wavefront | **disputed** | Dependences are (1,0) and (1,-1): both carried by the outer `i` loop, and row `i` reads only row `i-1`. The inner `j` loop is already parallel and vectorisable as written, so no wavefront or skewing is needed; this is an outer-loop-carried row recurrence. |
| `wf_triangular` | wavefront | **agree** | Dependences (1,0) north and (0,1) west, so the parallel front is the anti-diagonal `i+j`, a genuine wavefront (cf. TSVC `s2111`, labelled `wavefronts` by the corpus). |

Summary: 6 agree, 2 disputable (`ext_break_capture`, `ext_war_unit`: the corpus's own label for
the TSVC original differs), 1 disputed (`wf_diff_skew`: no wavefront is needed). `labels.csv` is
not changed here, because it is shared with v1 and the analysis keys on it. The effect of the
three alternatives on the class table is stated in `summary.md`.
