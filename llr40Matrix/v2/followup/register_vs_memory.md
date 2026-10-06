# Experiment 4: is "register versus memory" really about 6 cycles of store forwarding?

**Hypothesis.** A loop-carried value that is stored and immediately reloaded costs about 6 extra
cycles per element on this core. So far this was inferred from min-of-k at an assumed 3.26 GHz:
`versioned_distance_update` (K = 1), translated C 10.4 cycles/element against the agent's 4.2;
`wf_triangular`, translated C 2.1 against Numba's 8.2.

**Verdict: confirmed, now measured.** perf counts over the timed reps give the clock as
3.257-3.264 GHz and the cost of the store-and-reload as **6.08 cycles per element**
(`versioned_distance_update`: 10.13 -> 4.04) and **5.96 cycles per element** (`wf_triangular`:
8.14 -> 2.18). In each case the variant changes only how the value is carried, and the disassembly
shows the reload gone. Each variant then runs at the speed of the cell that already kept the value
in a register: 4.04 against the agent's 4.19, and 2.18 against translated C's 2.16.

## Setup

- **4a**, translated C of `versioned_distance_update` with only a `K == 1` specialisation that
  carries the value in a scalar (`prev = ((0.75 * prev) + (b[i] * c[i])); a[i] = prev;`, starting
  from `prev = a[0]`). The original loop is kept for every other K
  (`variants/versioned_distance_update_fp64.k1scalar.c.patch`). The harness runs this kernel at
  K = 1, the first value of its config domain `[1, 5, 64, 251]` (`spec.ConfigKnob.representative`).
- **4b**, the Numba version of `wf_triangular` with only the west neighbour carried in a scalar
  across the `j` loop (`west = a[i, i-1]` per row, then `west = a[i, j] + a[i-1, j] + west;
  a[i, j] = west`). The summation order is the original's
  (`variants/wf_triangular_numba_np.westscalar.py.patch`).
- The cells they are compared with were **re-measured in the same job, on the same node**, with
  the same perf gating: `versioned_distance_update` `c` and `agent` (v2's pick, Fortran, sha256
  `01661d85...`), and `wf_triangular` `numba` and `c`. The timed unchanged sources are
  byte-identical to v2's (sha256 in `source_sha256`). v2's minima are listed beside them.
- perf: `perf stat -e cycles,instructions`, enabled only around each of the 30 kept reps
  (`perfgate/`, `DEVIATIONS.md` item 7). The count is user-space only (`perf_event_paranoid=2`),
  and every row has 30 gated reps. Measured clock = cycles / counter-enabled time. Cycles per element =
  cycles / (30 x elements per call), with elements = LEN_1D - K = 94,999,999 and
  N(N-1)/2 = 151,527,936 (N = 17409).
- Preset M, `float64`, 5 + 30 reps, v2 environment and step geometry, job 4982639
  (`versioned_distance_update` on nid005667, `wf_triangular` on nid005937). Every cell validated
  against the oracle before it was timed: all six `ok`.
- Disassembly: `opt_reports/regmem/unchanged/...` and `opt_reports/regmem/<variant>/...`
  (`objdump -d` for C/Fortran, `inspect_asm()` for Numba), regenerated in job 4982663
  (`DEVIATIONS.md` item 9).

## Per cell

| kernel | cell | oracle | min-of-k (ms) | v2 min (ms) | measured GHz | **cycles/element** (perf, mean of 30) | at min-of-k | at assumed 3.26 GHz | instr./element | IPC | reload of the carried value in the hot loop? |
|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `versioned_distance_update` | `c`, unchanged | ok | 295.10 | 302.92 | 3.2575 | **10.13** | 10.12 | 10.13 | 9.02 | 0.89 | **yes** |
| `versioned_distance_update` | `c`, 4a K==1 scalar | ok | 117.60 | -- | 3.2574 | **4.04** | 4.03 | 4.04 | 8.01 | 1.98 | no |
| `versioned_distance_update` | `agent`, unchanged | ok | 121.92 | 123.10 | 3.2573 | **4.19** | 4.18 | 4.18 | 8.27 | 1.97 | no |
| `wf_triangular` | `numba`, unchanged | ok | 377.62 | 382.76 | 3.2635 | **8.14** | 8.13 | 8.12 | 15.02 | 1.85 | **yes** |
| `wf_triangular` | `numba`, 4b west scalar | ok | 100.06 | -- | 3.2626 | **2.18** | 2.15 | 2.15 | 10.01 | 4.59 | no |
| `wf_triangular` | `c`, unchanged | ok | 99.34 | 99.52 | 3.2627 | **2.16** | 2.14 | 2.14 | 8.01 | 3.71 | no |

The assumed 3.26 GHz was right to within 0.2%. The earlier 10.4 for translated C came from v2's
302.9 ms minimum; this node ran the same code 2.6% faster.

## The assembly: is the reload gone?

- **4a, unchanged C** (`opt_reports/regmem/unchanged/versioned_distance_update/c/versioned_distance_update_fp64.c.s.txt:43-51`).
  GCC versions the loop on the distance, and K = 1 takes the scalar loop at 0xa0-0xc0. Its
  `ldr d26, [x5, x3, lsl #3]` reloads `a[i-1]`, which the previous iteration has just written
  with `str d26`. The chain is store -> load -> `fmadd d26, d26, d27, d25`.
- **4a, variant** (`opt_reports/regmem/vdu_k1_scalar/versioned_distance_update/c/versioned_distance_update_fp64.c.s.txt:44-51`).
  The K==1 loop at 0xa4-0xc0 loads only `b[i]` and `c[i]`, and `fmadd d24, d24, d23, d21` keeps
  `prev` in `d24`. **The reload is gone.** The FP code is otherwise identical: the same
  `fmul b*c` and the same `fmadd` with 0.75. So the experiment changes the carry and nothing else
  (9 -> 8 instructions per element: exactly the one load).
- **Agent** (`.../unchanged/versioned_distance_update/agent/versioned_distance_update_fp64.f90.s.txt:53-60`):
  `fmadd d1, d1, d3, d0` with the value carried in `d1`.
- **4b, unchanged Numba** (`opt_reports/regmem/unchanged/wf_triangular/numba/wf_triangular_numba_np.py.s.txt:56-86`).
  The inner loop `.LBB0_7` (unrolled x2) reloads `a[i, j-1]` (`ldur d1, [x3, #-8]` line 69, and
  `ldr d1, [x4, x2]` line 82). The address comes from the negative-index wraparound `csel`s at
  lines 59 and 61, and the loaded value feeds `fadd` and then `str` to the element the next
  iteration reloads.
- **4b, variant** (`opt_reports/regmem/wf_west_scalar/wf_triangular/numba/wf_triangular_numba_np.py.s.txt:52-72`).
  `.LBB0_7` loads only `a[i, j]` and `a[i-1, j]`, and `fadd d0, d0, d1` carries `west` in `d0`.
  **The reload and the `j-1` `csel`s are gone** (15 -> 10 instructions per element).
- **Translated C** (`opt_reports/regmem/unchanged/wf_triangular/c/wf_triangular_fp64.c.s.txt:14-21`):
  GCC already does this itself, with `west` in `d31` (`fadd d31, d31, d29`).

## Reading the numbers

- With the value in a register, each loop runs at the latency of its one dependent FP operation
  on Neoverse V2: `fmadd`, 4.04 cycles (and the agent's `fmadd` loop 4.19), and `fadd`, 2.16-2.18
  cycles. With the store and reload on the chain, both loops pay about 6 cycles more: 6.08 in GCC
  code and 5.96 in LLVM (Numba) code. That is two kernels, two compilers, and the same cost.
- The reloaded address was written by the immediately preceding iteration, a few cycles earlier,
  so the value can only come from store-to-load forwarding. These counters measure what the round
  trip adds to the dependence chain. They do not measure forwarding separately from an L1 hit of
  the same latency.
- In 4b the variant also removes the wraparound `csel`s, so it changes two things at once. The
  `csel`s compute an address that depends only on `j`, not on the data, so they sit off the
  dependence chain. The variant retires 10 instructions per element at IPC 4.6, so 5 more could
  not cost anywhere near 6 cycles. 4a has no `csel` at all and shows the same 6 cycles. Both point
  to the memory round trip, not the selects, as the cost.

## Spread

Five of the six series have RSD 0.24-0.49%. The unchanged `versioned_distance_update` `c` series
has RSD 11.8% because of one rep: 494.6 ms, while the other 29 are 295.1-296.9 ms. In that rep the
process was off the CPU for about 0.2 s: the counters were enabled for 8.859 s against 9.068 s of
timed reps, and per-task counters stop while the task is descheduled. Min-of-k and the perf
cycles per element are both unaffected.
