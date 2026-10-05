"""Run one of the vendored HPCAgent-Bench plotting scripts without installing HPCAgent-Bench.

    python plotting/run.py plot_llr40_compilers --help
    python plotting/run.py statistics/plot_speedup.py ...

Upstream installs ``hpcagent_bench`` and the translator package ``numpyto_common`` with pip.
Here both live in this directory under their upstream paths, so this launcher puts them on
``sys.path`` and runs the script as ``__main__``. The vendored files themselves are unmodified.
"""

import pathlib
import runpy
import sys

HERE = pathlib.Path(__file__).resolve().parent
SRC_DIRS = (HERE, HERE / "hpcagent_bench" / "numpy_translators" / "src")

# hpcagent_bench/seal.py (the judge's sandbox, reached through the harness imports of
# figures/{results,scaling,transfer}) computes a Linux namespace mask at import time. Plotting never
# enters the sandbox, so on a platform without these constants (macOS) define them as 0: the import
# succeeds, and os.unshare -- which does not exist there either -- is never called.
LINUX_NAMESPACE_FLAGS = ("CLONE_NEWUSER", "CLONE_NEWNS", "CLONE_NEWPID", "CLONE_NEWNET", "CLONE_NEWIPC")


def _allow_sandbox_import() -> None:
    import os

    for flag in LINUX_NAMESPACE_FLAGS:
        if not hasattr(os, flag):
            setattr(os, flag, 0)


def _script(name: str) -> pathlib.Path:
    path = pathlib.Path(name)
    if path.suffix != ".py":
        path = path.with_suffix(".py")
    for candidate in (path, HERE / path, HERE / "statistics" / path.name):
        if candidate.is_file():
            return candidate.resolve()
    available = sorted(p.stem for p in (HERE / "statistics").glob("*.py"))
    sys.exit(f"run.py: no script {name!r}; available: {', '.join(available)}")


def main() -> None:
    if len(sys.argv) < 2 or sys.argv[1] in ("-h", "--help"):
        sys.exit(__doc__)
    script = _script(sys.argv[1])
    _allow_sandbox_import()
    for src in reversed(SRC_DIRS):
        sys.path.insert(0, str(src))
    sys.argv = [str(script), *sys.argv[2:]]
    runpy.run_path(str(script), run_name="__main__")


if __name__ == "__main__":
    main()
