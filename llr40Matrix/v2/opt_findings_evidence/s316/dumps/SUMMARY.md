# s316: where GCC 13 and GCC 14 part (tree and RTL dumps, 2026-10-23)

`dump_passes.sh` compiled `../s316.c` (the translated C of `tsvc_2_s316`) with `/usr/bin/gcc-13` and
`/usr/bin/gcc-14` on a daint login node, with the harness's C flags and every tree and RTL dump.

- Result as in the matrix: gcc-13 keeps the branch (`fcsel=0 b.gt=1`), gcc-14 emits the select
  (`fcsel=1 b.gt=0`); `gcc-1?/s316.s.txt`.
- The first pass where they differ is **`phiopt1`**, the first tree-level PHI optimization:
  - gcc-14 (`gcc-14/s316.c.046t.phiopt1`): "COND_EXPR in block 3 and PHI in block 5 converted to
    straightline code". The loop body becomes `_6 = _4 < x_9; _7 = _6 ? _4 : x_9;`, a conditional
    expression in GIMPLE, which is expanded to `fcsel`.
  - gcc-13 (`gcc-13/s316.c.044t.phiopt1`): no conversion; the update stays a branch with
    `# x_8 = PHI <x_9(3), _4(4)>`, and the backend keeps the branch (`b.gt`).
- Neither version vectorizes the loop (`optinfo.txt`: "unsupported use in stmt").
