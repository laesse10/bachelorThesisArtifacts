"""Put a v0.2.1-emitted Numba module into a 26a4f0cf bench tree so that the unchanged harness times it.

The 26a4f0cf harness (``autogen.ensure``) takes ``<module>_numba_np.py`` from the kernel's ``.cache/``
when the ``.fp`` sidecar there equals ``source_fingerprint(<module>_numpy.py, bench_info)``; it then
restores that file to the canonical path and does not emit. This script writes the v0.2.1 file and the
fingerprint the 26a4f0cf code computes for the kernel, so the harness, the reference and the protocol
stay 26a4f0cf and only the module's bytes change. A hand-written override in the kernel folder (no
autogen marker) would win over the cache, so it is moved aside to ``<name>.hand_override``.

It also checks that the bench-info the tree builds now is byte-identical to the one the v0.2.1
translator was given (``sources/<k>.bench_info.26a4f0cf.json``).

    LLR40_BENCH=<26a4f0cf tree> python seed_cache.py KERNEL SOURCES_DIR
"""
import hashlib, os, pathlib, shutil, sys

BENCH = pathlib.Path(os.environ["LLR40_BENCH"]).resolve()
sys.path[:0] = [str(BENCH), str(BENCH / "hpcagent_bench/numpy_translators/src")]
from hpcagent_bench import autogen, framework_cache, paths  # noqa: E402
from hpcagent_bench.spec import BenchSpec  # noqa: E402
from numpyto_common.emit_io import is_generated, is_override  # noqa: E402

k, src_dir = sys.argv[1], pathlib.Path(sys.argv[2]).resolve()
sha = lambda p: hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
spec = BenchSpec.load(k)
kdir = paths.BENCHMARKS / spec.relative_path
numpy_py = kdir / f"{spec.module_name}_numpy.py"
with autogen.bench_info_tempfile(spec) as bi:
    bi_bytes = bi.read_bytes()
want_bi = (src_dir / f"{k}.bench_info.26a4f0cf.json").read_bytes()
assert bi_bytes == want_bi, "bench-info differs from the one the v0.2.1 translator was given"
fp = framework_cache.source_fingerprint(numpy_py, bi_bytes)

new = src_dir / f"{k}_numba.v021.py"
assert is_generated(new), f"{new} has no autogen marker"
canonical = kdir / f"{spec.module_name}_numba_np.py"
if canonical.exists() and is_override(canonical):
    shutil.move(canonical, canonical.with_name(canonical.name + ".hand_override"))
    print(f"{k}: moved hand override aside -> {canonical.name}.hand_override")
cache = framework_cache.kernel_cache_dir(kdir)
shutil.copyfile(new, cache / canonical.name)
(cache / (canonical.name + ".fp")).write_text(fp)
shutil.copyfile(new, canonical)
assert framework_cache.load_generated(cache, canonical, fp)        # what ensure() will see: a HIT
print(f"{k}: seeded {canonical.relative_to(BENCH)} sha256 {sha(canonical)} fingerprint {fp[:16]}")
