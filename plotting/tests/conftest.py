"""Minimal conftest for the vendored plotting tests.

Upstream's tests/conftest.py starts the judge service and imports the harness; the plotting tests
need none of that. This file gives them the same import path run.py gives the scripts, and the one
helper (``script_path``) they share, copied from upstream.
"""

import importlib.util
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]

_spec = importlib.util.spec_from_file_location("_plotting_run", ROOT / "run.py")
_run = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_run)
_run._allow_sandbox_import()
for _src in reversed(_run.SRC_DIRS):
    if str(_src) not in sys.path:
        sys.path.insert(0, str(_src))

#: Upstream tests/conftest.py: the directories a test script lookup searches.
SCRIPT_DIRS: tuple[str, ...] = ("statistics", "scripts", "experiments")


def script_path(name: str, root: pathlib.Path | None = None) -> pathlib.Path:
    """``<name>.py`` in whichever of :data:`SCRIPT_DIRS` holds it; raises naming all of them."""
    base = root if root is not None else ROOT
    for directory in SCRIPT_DIRS:
        candidate = base / directory / f"{name}.py"
        if candidate.is_file():
            return candidate
    searched = ", ".join(f"{d}/{name}.py" for d in SCRIPT_DIRS)
    raise FileNotFoundError(f"no {name}.py under {base}; looked in {searched}")


#: Upstream tests that check files outside the plotting layer. Skipped, not deleted, so the reason
#: stays visible in every run.
OUT_OF_SCOPE: dict[str, str] = {
    "test_display_names.py::test_the_registered_checkpoint_is_what_the_arms_served":
        "reads the campaign arm .env files under experiments/, which plotting does not use",
    "test_signed_rank.py::test_both_paths_read_one_threshold":
        "compares against statistics/ablation_stats.py, a statistics script outside the plotting layer",
}


def pytest_collection_modifyitems(config, items):
    import pytest

    for item in items:
        for suffix, reason in OUT_OF_SCOPE.items():
            if item.nodeid.endswith(suffix) or item.nodeid.split("[")[0].endswith(suffix):
                item.add_marker(pytest.mark.skip(reason=f"out of scope for the vendored copy: {reason}"))
