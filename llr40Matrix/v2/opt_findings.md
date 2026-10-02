# Optimisation-report findings (v2, task 2)

Reports: `opt_reports/<kernel>/<column>/<source>.optreport.txt` (the first line is the exact compile argv: the sweep's harness compile line plus `-fopt-info-vec-optimized -fopt-info-vec-missed -fopt-info-loop-optimized`), and the kernel's disassembly in `<source>.s.txt`. Citations are `file:line`. Explanatory compile-only experiments, each changing one thing against that compile line, are in `opt_findings_evidence/` (`opt_evidence.sh`). Times are v2 min-of-k at preset M.

Toolchain note that matters for every C-vs-C++/Fortran comparison below: the C columns are built by **gcc 14.2.0** (harness floor for `-std=c23`), and C++ and Fortran by **13.3.1**.

## 1. `loop_interchange` and `loop_distribution`: did GCC interchange or distribute anywhere?

(`vec` = number of 'loop vectorized' remarks in that report.)

**No.** In none of the 6 kernels, in none of the compiled columns, does any report contain an interchange or distribution remark. With `-fopt-info-loop-optimized`, GCC reports a successful interchange ('loops interchanged in loop nest') or distribution ('Loop N distributed'):

| kernel | class | `c` | `c_reference` | `cpp` | `fortran` | `agent` | agent vs fastest non-agent column |
|---|---|---|---|---|---|---|---|
| `tsvc_2_s1232` | loop_interchange | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 1 | 3.0x faster (13.1 ms vs 39.1 ms) |
| `tsvc_2_s2233` | loop_interchange | interch 0, distrib 0, vec 1 | interch 0, distrib 0, vec 1 | interch 0, distrib 0, vec 1 | interch 0, distrib 0, vec 1 | n/a | no agent cell |
| `tsvc_2_s2275` | loop_distribution | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 2 | 5.7x faster (75.1 ms vs 426.5 ms) |
| `tsvc_2_s231` | loop_interchange | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 1 | 7.8x faster (69.9 ms vs 547.5 ms) |
| `tsvc_2_s233` | loop_interchange | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 1 | 9.9x faster (66.7 ms vs 657.1 ms) |
| `tsvc_2_s235` | loop_interchange | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 0 | interch 0, distrib 0, vec 4 | 7.9x faster (58.0 ms vs 460.2 ms) |

What GCC reports instead, autogen C and Fortran:

- `tsvc_2_s1232`:
  - c: `opt_reports/tsvc_2_s1232/c/tsvc_2_s1232_fp64.c.optreport.txt:4`: `tsvc_2_s1232_fp64.c:179:31: missed: not vectorized: unsupported outerloop form.`
  - fortran: `opt_reports/tsvc_2_s1232/fortran/tsvc_2_s1232_fp64.f90.optreport.txt:4`: `tsvc_2_s1232_fp64.f90:11:13: missed: not vectorized: control flow in loop.`
- `tsvc_2_s2233`:
  - c: `opt_reports/tsvc_2_s2233/c/tsvc_2_s2233_fp64.c.optreport.txt:4`: `tsvc_2_s2233_fp64.c:179:31: missed: not vectorized: loop nest containing two or more consecutive inner loops cannot be vectorized`
  - fortran: `opt_reports/tsvc_2_s2233/fortran/tsvc_2_s2233_fp64.f90.optreport.txt:4`: `tsvc_2_s2233_fp64.f90:10:13: missed: not vectorized: loop nest containing two or more consecutive inner loops cannot be vectorized`
- `tsvc_2_s2275`:
  - c: `opt_reports/tsvc_2_s2275/c/tsvc_2_s2275_fp64.c.optreport.txt:4`: `tsvc_2_s2275_fp64.c:181:41: missed: not vectorized: not suitable for strided load _6 = *_5;`
  - fortran: `opt_reports/tsvc_2_s2275/fortran/tsvc_2_s2275_fp64.f90.optreport.txt:4`: `tsvc_2_s2275_fp64.f90:17:26: missed: not vectorized: not suitable for strided load _6 = (*aa_28(D))[_5];`
- `tsvc_2_s231`:
  - c: `opt_reports/tsvc_2_s231/c/tsvc_2_s231_fp64.c.optreport.txt:4`: `tsvc_2_s231_fp64.c:180:33: missed: not vectorized: unsupported outerloop form.`
  - fortran: `opt_reports/tsvc_2_s231/fortran/tsvc_2_s231_fp64.f90.optreport.txt:4`: `tsvc_2_s231_fp64.f90:10:17: missed: not vectorized: control flow in loop.`
- `tsvc_2_s233`:
  - c: `opt_reports/tsvc_2_s233/c/tsvc_2_s233_fp64.c.optreport.txt:4`: `tsvc_2_s233_fp64.c:179:31: missed: not vectorized: loop nest containing two or more consecutive inner loops cannot be vectorized`
  - fortran: `opt_reports/tsvc_2_s233/fortran/tsvc_2_s233_fp64.f90.optreport.txt:4`: `tsvc_2_s233_fp64.f90:10:13: missed: not vectorized: loop nest containing two or more consecutive inner loops cannot be vectorized`
- `tsvc_2_s235`:
  - c: `opt_reports/tsvc_2_s235/c/tsvc_2_s235_fp64.c.optreport.txt:4`: `tsvc_2_s235_fp64.c:179:31: missed: not vectorized: unsupported outerloop form.`
  - fortran: `opt_reports/tsvc_2_s235/fortran/tsvc_2_s235_fp64.f90.optreport.txt:4`: `tsvc_2_s235_fp64.f90:13:73: missed: not vectorized: control flow in loop.`

`-fopt-info` reports only SUCCESSFUL interchanges, so for the cleanest perfect nest (`tsvc_2_s231`, dependence vector (0,1), interchange legal) the interchange pass was dumped (`opt_findings_evidence/s231/`, `-fdump-tree-linterchange-details`): dump: s231.c.168t.linterchange, 83 lines; lines mentioning interchange/consider/loop nest: 0. The pass never took up the nest as a candidate: it printed no analysis and no rejection. The dump does not say why.

**What the agents did differently.** Every agent submission does the restructuring by hand, and GCC then vectorises the unit-stride inner loop it was given:

- `tsvc_2_s1232`: (Fortran) linearised the triangular update into a contiguous 1-D run `aa(k) = bb(k) + cc(k)` per column (column-major), i.e. unit stride. `opt_reports/tsvc_2_s1232/agent/tsvc_2_s1232_fp64.f90.optreport.txt:7`: `tsvc_2_s1232_fp64.f90:21:11: optimized: loop vectorized using 16 byte vectors`
- `tsvc_2_s2275`: flattened the 2-D update into one 1-D loop over all `LEN_2D^2` elements and moved the `a[i]` statement to its own loop (distribution); both loops vectorise. `opt_reports/tsvc_2_s2275/agent/tsvc_2_s2275_fp64.c.optreport.txt:3`: `tsvc_2_s2275_fp64.c:9:21: optimized: loop vectorized using 16 byte vectors`
- `tsvc_2_s231`: interchanged: `j` outer, `i` inner, so the inner loop is unit-stride and carries no dependence (`aa[j][i] = aa[j-1][i] + bb[j][i]`). `opt_reports/tsvc_2_s231/agent/tsvc_2_s231_fp64.c.optreport.txt:5`: `tsvc_2_s231_fp64.c:18:40: optimized: loop vectorized using 16 byte vectors`
- `tsvc_2_s233`: distributed the two `j`-loops into separate nests and interchanged each to an `i`-inner, unit-stride loop. `opt_reports/tsvc_2_s233/agent/tsvc_2_s233_fp64.c.optreport.txt:15`: `tsvc_2_s233_fp64.c:27:30: optimized: loop vectorized using 16 byte vectors`
- `tsvc_2_s235`: (Fortran) split the `a(i)` update into its own `!$omp simd` loop (distribution) and interchanged the 2-D update to `j` outer, `!$omp do simd` over `i`. `opt_reports/tsvc_2_s235/agent/tsvc_2_s235_fp64.f90.optreport.txt:5`: `tsvc_2_s235_fp64.f90:22:47: optimized: loop vectorized using 16 byte vectors`
- `tsvc_2_s2233` has no agent cell. GCC vectorises its second inner loop in every compiled column (`opt_reports/tsvc_2_s2233/c/tsvc_2_s2233_fp64.c.optreport.txt:5`: `tsvc_2_s2233_fp64.c:183:33: optimized: loop vectorized using 16 byte vectors`), which is why this kernel looked like an outlier in v1's normalisation.

So the 3.0-9.9x agent advantage on these classes is restructuring that GCC 13/14 at `-O3` does not do on these sources: its interchange pass does not consider the nests, and its vectoriser rejects them as outer-loop, multi-inner-loop or strided forms.

## 2. `tsvc_2_s3111`: why is `fortran` 5.8x slower?

min-of-k: c 126.5 ms, c_reference 126.3 ms, cpp 124.9 ms, **fortran 728.6 ms**, numba 314.0 ms, agent 125.8 ms. (The task's '4.4x' is the deviation from the reduction class geomean in `outliers.csv`. The raw ratio to `c` is the one in the heading.)

The kernel is a conditional sum, `if a(i) > 0: sum += a(i)`. Its input is uniform on [-1000, 1000), so exactly half the elements are positive, and `LEN_1D` = 2e8.

- c: `opt_reports/tsvc_2_s3111/c/tsvc_2_s3111_fp64.c.optreport.txt:3`: `tsvc_2_s3111_fp64.c:181:31: optimized: loop vectorized using variable length vectors`
- cpp: `opt_reports/tsvc_2_s3111/cpp/tsvc_2_s3111_fp64.cpp.optreport.txt:3`: `tsvc_2_s3111_fp64.cpp:170:31: optimized: loop vectorized using variable length vectors`
  The loop is a predicated SVE compare feeding an ordered, predicated add, with no branch: `opt_reports/tsvc_2_s3111/c/tsvc_2_s3111_fp64.c.s.txt:13`: `28:	65d03fd6 	fcmgt	p6.d, p7/z, z30.d, #0.0`, `opt_reports/tsvc_2_s3111/c/tsvc_2_s3111_fp64.c.s.txt:15`: `30:	65d83bdf 	fadda	d31, p6, d31, z30.d`.
- fortran: `opt_reports/tsvc_2_s3111/fortran/tsvc_2_s3111_fp64.f90.optreport.txt:4`: `tsvc_2_s3111_fp64.f90:2:28: missed: not vectorized: unsupported use in stmt.`
  The loop stays scalar with one data-dependent branch per element: `opt_reports/tsvc_2_s3111/fortran/tsvc_2_s3111_fp64.f90.s.txt:8`: `14:	1e602018 	fcmpe	d0, #0.0`, `opt_reports/tsvc_2_s3111/fortran/tsvc_2_s3111_fp64.f90.s.txt:9`: `18:	540000cc 	b.gt	30 <tsvc_2_s3111_fp64+0x30>`.

**Cause: the translator's parentheses.** The emitted Fortran is `sum_val = (sum_val + a((i_l0) + 1))`. In Fortran, parentheses are semantically binding: gfortran must keep them, as a `PAREN_EXPR` in GIMPLE, and the vectoriser does not recognise a reduction through it. From the vectoriser dump (`opt_findings_evidence/s3111/orig.vect.excerpt.txt`):

```
65-orig.f90:10:13: note:   mark relevant 1, live 0: sum_val_10 = ((_3));
66-orig.f90:10:13: note:   vect_is_simple_use: operand sum_val_16 = PHI <sum_val_6(8), 0.0(18)>, type of def: unknown
67-orig.f90:10:13: missed:   Unsupported pattern.
68:orig.f90:2:28: missed:   not vectorized: unsupported use in stmt.
69-orig.f90:10:13: missed:  unexpected pattern.
```
Removing only those parentheses (`noparen.diff`) makes gfortran vectorise the loop: noparen: vectorized=2 vector_fadd=10 faddp=3 fadda=0. A 50%-taken branch over 2e8 elements mispredicts about half the time; 11.9 cycles/element at 3.26 GHz is consistent with that (a ~20-cycle penalty at a 50% miss rate), though branch misses were not counted here. In C the same parentheses carry no meaning and create no `PAREN_EXPR`.

## 3. `tsvc_2_s316`: why are `c` and `c_reference` about 3.4x slower than `cpp` and `fortran`?

min-of-k: c 275.8 ms, c_reference 276.0 ms, cpp 80.0 ms, fortran 80.2 ms. The kernel is a running minimum, `if a[i] < x: x = a[i]`, with `LEN_1D` = 2e8. No column vectorises it, and the C and C++ sources are identical.

- c (gcc 14.2.0) if-converts the update into a branch-free select: `opt_reports/tsvc_2_s316/c/tsvc_2_s316_fp64.c.s.txt:13`: `28:	1e7f4fdf 	fcsel	d31, d30, d31, mi	// mi = first`. Both the compare and the select sit on the loop-carried chain through `x`, so each element pays their latency: about 4.5 cycles/element at 3.26 GHz.
- cpp and fortran (13.3.1) keep a branch to an out-of-line move: `opt_reports/tsvc_2_s316/cpp/tsvc_2_s316_fp64.cpp.s.txt:13`: `28:	540000cc 	b.gt	40 <tsvc_2_s316_fp64+0x40>`. A running minimum over random data almost never updates, so the branch predicts well and nothing serialises the loop. It runs at about 20 GB/s, the single-core bandwidth fast state of protocol.md section 4.
- **Cause: compiler version, not language** (`opt_findings_evidence/s316/summary.txt`, the same sources rebuilt with the other version): c_gcc14 fcsel=1 branch_b.gt=0; c_gcc13 fcsel=0 branch_b.gt=1; cpp_gxx13 fcsel=0 branch_b.gt=1; cpp_gxx14 fcsel=1 branch_b.gt=0. This was already true in v1, whose C columns were also gcc 14.2.0.

## 4. Reductions: does gfortran vectorise reductions that gcc-C does not (reassociation)?

**No, the opposite.** gfortran vectorises none of the 10. gcc-C/C++ vectorise exactly two, and only as in-order (non-reassociating) SVE `fadda` reductions. The fortran/c ratio column shows the result:

| kernel | c: vec / fadda | cpp: vec / fadda | fortran: vec / fadda / reassoc. vector fadd | fortran/c min-of-k |
|---|---|---|---|---:|
| `argmax_with_index` | 0 / 0 | 0 / 0 | 0 / 0 / 0 | 1.00 |
| `quasi_affine_reduce_odd` | 0 / 0 | 0 / 0 | 0 / 0 / 0 | 1.00 |
| `scan_affine_decay` | 0 / 0 | 0 / 0 | 0 / 0 / 0 | 1.25 |
| `tsvc_2_s311` | 1 / 1 | 1 / 1 | 0 / 0 / 0 | 1.00 |
| `tsvc_2_s3110` | 0 / 0 | 0 / 0 | 0 / 0 / 0 | 1.01 |
| `tsvc_2_s3111` | 1 / 1 | 1 / 1 | 0 / 0 / 0 | 5.76 |
| `tsvc_2_s3112` | 0 / 0 | 0 / 0 | 0 / 0 / 0 | 1.00 |
| `tsvc_2_s316` | 0 / 0 | 0 / 0 | 0 / 0 / 0 | 0.29 |
| `tsvc_2_s318` | 0 / 0 | 0 / 0 | 0 / 0 / 0 | 1.00 |
| `tsvc_2_s319` | 0 / 0 | 0 / 0 | 0 / 0 / 0 | 1.00 |

- gfortran *would* reassociate. With the parentheses removed it vectorises `s3111` into vector partial sums plus a horizontal `faddp` (`opt_findings_evidence/s3111/summary.txt`), which is the reassociation licence protocol.md section 5.1 warns about. But the translator writes every accumulation in parentheses, and parentheses forbid it. The protocol's '3.1x Fortran advantage on reductions' therefore does not occur in this matrix. Where C and Fortran differ on a reduction, Fortran is the slower one.
- The same failure on the plain sum `tsvc_2_s311`: `opt_reports/tsvc_2_s311/fortran/tsvc_2_s311_fp64.f90.optreport.txt:4`: `tsvc_2_s311_fp64.f90:11:61: missed: not vectorized: unsupported use in stmt.`. Here gcc's in-order `fadda` (`opt_reports/tsvc_2_s311/c/tsvc_2_s311_fp64.c.s.txt:13`: `28:	65d83fdf 	fadda	d31, p7, d31, z30.d`) buys nothing: c 135.5 ms vs scalar fortran 135.4 ms, because an in-order reduction is still latency-bound. Only the agent, whose OpenMP `reduction` clause licenses reassociation, vectorises it with partial sums: 10 vector `fadd`, 76.2 ms.
- The parentheses also block FMA contraction in scalar Fortran. `scan_affine_decay` (`((c*y) + x)`) compiles to `fmul`+`fadd` in Fortran and `fmadd` in C (`opt_findings_evidence/scan_affine_decay/summary.txt`). On a loop-carried recurrence that is 3+2 vs 4 cycles: fortran 148.0 ms vs c 118.2 ms.
- In vectorised Fortran code the parentheses do NOT stop contraction: `tsvc_2_s115`'s Fortran body uses a fused vector `fmls` (`opt_findings_evidence/s115/summary.txt`: 1 fmls, 2 fmul, 2 fsub -- count, instruction; the scalar peel/tail uses separate `fmul`/`fsub`). That is why its Fortran column is `incorrect` with `-ffp-contract=fast`, like the C columns, and `ok` with `-ffp-contract=off`.

