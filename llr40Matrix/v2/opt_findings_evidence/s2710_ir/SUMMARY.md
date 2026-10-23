# s2710: why Numba's loop vectorizes and the C loop does not (2026-10-23)

`tsvc_2_s2710` reads `x[0]` inside its loop, on one branch only (`if (x[0] > 0.0)` in the `else` part).
GCC does not vectorize the translated C: it will not load a value the source may never read, so if-conversion
turns the read into a masked load from one fixed address, which its vectorizer does not support
(`../../opt_reports/tsvc_2_s2710/c/tsvc_2_s2710_fp64.c.optreport.txt`: "relevant stmt not supported:
.MASK_LOAD"). clang does not vectorize it either. Numba (0.67, LLVM) does, with SVE.

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
