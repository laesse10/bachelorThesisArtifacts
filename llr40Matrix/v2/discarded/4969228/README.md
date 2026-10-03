# Discarded: debug chain job 4969228 (2026-10-02 15:22, cancelled at ~15:25)

Every row this job wrote is INVALID and is not in results.csv. `chain_unit.sh` passed a relative
`--scratch scratch/<kernel>`; at 26a4f0cf the harness driver resolves `--output` against its own
cwd (the bench checkout), so it wrote its JSONL there (`driver_jsonl_in_bench_tree/`) while
sweep.py looked under v2/, found nothing, and fell into its build_error branch. The harness itself
had validated and timed those cells (see the notes: "validation: SUCCESS", "median: ...ms").

Fixed in sweep.py / sweep_variant.py by resolving every path argument to an absolute path. The
job was cancelled after the first cells; all cells were re-measured by later chain jobs.
