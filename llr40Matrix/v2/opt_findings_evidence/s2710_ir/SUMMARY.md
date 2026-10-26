# s2710: why Numba's loop vectorizes and the C loop does not (2026-10-23)

`tsvc_2_s2710` reads `x[0]` inside its loop, on one branch only (`if (x[0] > 0.0)` in the `else` part).
GCC does not vectorize the translated C: it will not load a value the source may never read, so if-conversion
turns the read into a masked load from one fixed address, which its vectorizer does not support
(`../../opt_reports/tsvc_2_s2710/c/tsvc_2_s2710_fp64.c.optreport.txt`: "relevant stmt not supported:
.MASK_LOAD"). clang does not vectorize it either, for the same reason (section "clang's side" below). Numba
(0.67, LLVM) does, with SVE.

The difference is made by Numba before LLVM sees the code. The matrix's own Numba file
(`../../opt_reports/tsvc_2_s2710/numba/tsvc_2_s2710_numba_np.py`) was compiled once on a daint login node with
the matrix's venv (`venv_llr40v2`, Numba 0.67.0, aarch64), `NUMBA_NUM_THREADS=1`, by `run_s2710_ir.py`:

- `array_opt.txt` (`NUMBA_DEBUG_ARRAY_OPT=1`): Numba's parfor pass hoists loop-invariant statements out of
  the parallel loop (`_hoist_internal`). The loop writes only `a`, `b` and `c` (`setitems: {'a', 'b', 'c'}`),
  so the read of `x` counts as invariant:
  - line 1637: `Will hoist instruction $246binary_subscr.16 = static_getitem(value=x, index=0, ...)`
  - line 1641: `Will hoist instruction $252compare_op.18 = $246binary_subscr.16 > $const250.17.4`
  The pass hoists the read out of the branch it sits in: it ignores the control flow inside the loop.
- `llvm_opt.txt` (`NUMBA_DUMP_OPTIMIZED=1`): the parallel loop's function receives every array's data pointer
  as `noalias`; `x`'s as `ptr noalias readonly`. LLVM therefore gets a loop whose condition `x[0] > 0` is
  already invariant, unswitches it, and vectorizes each version (the disassembly from the matrix,
  `../../opt_reports/tsvc_2_s2710/numba/tsvc_2_s2710_numba_np.py.s.txt`: one `ldr d0, [x15]` before the loop,
  then `fcmp d0, #0.0` and SVE loops with `ld1d`/`fcmgt`).
- `llvm_raw.txt` (`NUMBA_DUMP_LLVM=1`): the same functions before LLVM's optimization.

Moving the read before the loop by hand does the same for GCC (`../../followup/s2710_hoist.md`): it vectorizes,
and C is 4% faster than Numba.

## clang's side: the same LLVM refuses to hoist the read (2026-10-26)

clang 22.1 is the LLVM version inside Numba 0.67, so comparing clang's IR with Numba's shows where the two
part. `clang/run_clang_ir.sh` compiles the kernel function of the translated C (`clang/s2710.c`) and of the
hand-hoisted variant (`clang/s2710_hoist.c`, from `../../followup/variants/src/`) with the harness's clang flags,
and writes the optimization remarks (`*.remarks.txt`), the optimized IR (`*.ll`) and the disassembly (`*.s.txt`).

**Where it ran.** daint was out of reach (expired CSCS certificate), so the script cross-compiles on a Mac with
Homebrew's clang 22.1.3 for `aarch64-unknown-linux-gnu`, with `-mcpu=neoverse-v2 -mtune=generic` in place of
`-march=native` on a GH200 node. The result is the code the matrix timed: the instruction sequence of
`tsvc_2_s2710_fp64` equals the one daint's clang 22.1.0 built (`../../clang/asm_so/tsvc_2_s2710/clang.s.txt`),
compared by `clang/norm.py` without addresses and branch targets; `clang/insns.diff` holds the one difference,
objdump's spelling of the constant 1.0.

**The translated C.**
- LLVM's loop-invariant code motion refuses to move the read out of the loop
  (`clang/s2710.remarks.txt`: "s2710.c:15:18: failed to hoist load with loop-invariant address because load is
  conditionally executed"). It hoists only `LEN_1D > 10` ("hoisting icmp"), and the loop is unswitched on that
  condition alone.
- In the optimized IR, both loop versions still load `x[0]` in their else block, on every iteration that takes
  it (`clang/s2710.ll` lines 31 and 101, `%x` in blocks `if.else20.us` and `if.else20`), and compare it with 0.
- The loop vectorizer then rejects the loop: "s2710.c:3:9: the cost-model indicates that vectorization is not
  beneficial". The IR has no vector type, and the disassembly is the scalar loop the matrix timed, with
  `ldr d2, [x5]` (`x[0]`) and `fcmp d2, #0.0` inside it.

**Numba's input to LLVM** (`llvm_raw.txt`, the parallel-loop function `__numba_parfor_gufunc_...`, lines 7-1664):
- Block `B0.endif.endif` (lines 1536-1619) runs once, before the loop header `B84`: it loads `x[0]` (line 1581),
  compares it with 0.0 (line 1602, `fcmp ogt`) and stores the result in `$252compare_op.18`; it also computes
  `LEN_1D > 10` (line 1613).
- Inside the loop, the else branch only reads the stored comparison (line 1140, block `B263`); nothing in the
  loop loads from `x`.
- LLVM therefore sees two loop-invariant conditions, unswitches on both and vectorizes the loop versions
  (`llvm_opt.txt`: 314 occurrences of `vscale x`).

**The hand-hoisted C.** With `const double x0 = x[0];` before the loop, the same clang hoists the comparison
("s2710_hoist.c:16:21: hoisting fcmp"), unswitches on both conditions and vectorizes all four loop versions
(`clang/s2710_hoist.remarks.txt`: four times "vectorized loop (vectorization width: vscale x 2, interleaved count:
1)"; `clang/s2710_hoist.ll`: `x[0]` loaded once in `entry`, line 13, and four `vector.body` loops).

**Where they part.** The two IR inputs differ in one place: the position of the read of `x[0]`. In the C, it sits
in the loop's else branch, where neither GCC nor LLVM may move it, because the source might never execute it.
Numba's parfor pass moves it, and with it the comparison, before the loop, ignoring the branch it sits in. From
there on, LLVM does the same for Numba's code as for the hand-hoisted C: it unswitches and vectorizes with SVE.

