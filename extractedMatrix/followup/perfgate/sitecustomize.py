"""Follow-up perf gate: count `perf stat` events over the TIMED reps only.

Inert unless FOLLOWUP_PERF_CTL is set. sweep_followup.py --perf runs the harness driver as

    perf stat -x, -e cycles,instructions --delay=-1 --control fifo:<ctl>,<ack> -- ...

with this directory first on PYTHONPATH. The counters start disabled. This hook wraps
``Framework.measure`` (hpcagent_bench/frameworks/framework.py) and brackets every rep it KEEPS
with `enable` / `disable` on perf's control fifo:

    before_each() -> [enable, ack] -> start_timer -> runner() -> stop_timer -> [disable, ack]

The fifo round-trips sit outside start_timer/stop_timer, so the timed bracket is unchanged.
Warm-up reps, input staging (before_each), the build, the oracle and the validation run are never
counted. Each measure() call appends one line to FOLLOWUP_PERF_LOG, so the number of gated reps
can be checked against n_reps.
"""
import importlib.abc
import importlib.util
import os
import sys

_CTL = os.environ.get("FOLLOWUP_PERF_CTL")
_ACK = os.environ.get("FOLLOWUP_PERF_ACK")
_LOG = os.environ.get("FOLLOWUP_PERF_LOG")
_TARGET = "hpcagent_bench.frameworks.framework"
_fds = {}


def _ctl(cmd):
    if "ctl" not in _fds:
        # perf holds both fifos open O_RDWR, so neither open blocks
        _fds["ctl"] = os.open(_CTL, os.O_WRONLY)
        _fds["ack"] = os.open(_ACK, os.O_RDONLY)
    os.write(_fds["ctl"], cmd + b"\n")
    got = b""
    while b"\n" not in got:
        chunk = os.read(_fds["ack"], 64)
        if not chunk:
            raise RuntimeError("perf control fifo closed")
        got += chunk


def _patch(mod):
    Framework = mod.Framework
    orig_measure = Framework.measure

    def measure(self, impl, runner, repeat, before_each=None, warmup=None):
        w = warmup
        if w is None:
            w = max(0, mod.config.get_int("measurement.warmup", 1))
        state = {"rep": 0, "gated": 0}
        orig_start, orig_stop = self.start_timer, self.stop_timer

        def start_timer(timer):
            if state["rep"] >= w:
                _ctl(b"enable")
                state["gated"] += 1
            return orig_start(timer)

        def stop_timer(timer):
            sample = orig_stop(timer)
            if state["rep"] >= w:
                _ctl(b"disable")
            state["rep"] += 1
            return sample

        self.start_timer, self.stop_timer = start_timer, stop_timer
        try:
            return orig_measure(self, impl, runner, repeat, before_each=before_each, warmup=warmup)
        finally:
            del self.start_timer, self.stop_timer
            if _LOG:
                with open(_LOG, "a") as fh:
                    fh.write(f"measure framework={getattr(self, 'fname', '?')} warmup={w} repeat={repeat} "
                             f"reps_run={state['rep']} gated={state['gated']} pid={os.getpid()}\n")

    Framework.measure = measure


class _Finder(importlib.abc.MetaPathFinder):
    def find_spec(self, name, path, target=None):
        if name != _TARGET:
            return None
        sys.meta_path.remove(self)
        try:
            spec = importlib.util.find_spec(name)
        finally:
            sys.meta_path.insert(0, self)
        if spec is None or spec.loader is None:
            return spec
        loader = spec.loader
        orig_exec = loader.exec_module

        def exec_module(module):
            orig_exec(module)
            _patch(module)

        loader.exec_module = exec_module
        return spec


if _CTL and _ACK:
    sys.meta_path.insert(0, _Finder())
