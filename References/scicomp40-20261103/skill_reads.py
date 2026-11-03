"""Which skill pages the agents of the two lang-skills arms opened, per setup: the number of episodes with a Read of
each page under /skills/.

    python3 References/scicomp40-20261103/skill_reads.py <scicomp40-data-20261103>/tool_calls.csv

``tool_calls.csv`` (141 MB) is in the supervisor's larger archive, scicomp40-data-20261103 (1).tar.xz, not in this
folder. The skill pages are plain files under /skills/ that the prompt tells the agent to Read; the README's count
(Skill calls and Reads of a SKILL.md) misses them. Episodes are counted as the database keeps them: for the two
Qwen3.8-27B HIP arms, the first run's episodes of rerun kernels are dropped, as in keep_reruns.py.
"""

import collections
import csv
import json
import pathlib
import sys

HERE = pathlib.Path(__file__).resolve().parent
FIRST = {"scicomp40-qwen38-hip": "710590", "scicomp40-qwen38-hip-lang-skills": "711539"}


def main() -> None:
    csv.field_size_limit(sys.maxsize)
    rerun = {s: set((HERE / "reruns" / f"{s}.txt").read_text().split()) for s in FIRST}
    episodes: dict[str, set] = collections.defaultdict(set)
    reads: dict[str, dict[str, set]] = collections.defaultdict(lambda: collections.defaultdict(set))
    with open(sys.argv[1], newline="") as f:
        for row in csv.DictReader(f):
            setup, job, kernel = row["setup"], row["job"], row["kernel"]
            if FIRST.get(setup) == job and kernel in rerun[setup]:
                continue
            episodes[setup].add((job, kernel))
            if row["tool"] != "Read":
                continue
            try:
                path = json.loads(row["input"]).get("file_path", "")
            except json.JSONDecodeError:
                continue
            if path.startswith("/skills/"):
                reads[setup][path.removeprefix("/skills/")].add((job, kernel))
    for setup in sorted(episodes):
        pages = reads[setup]
        any_page = set().union(*pages.values()) if pages else set()
        print(f"{setup:34s} episodes {len(episodes[setup]):2d}  read a skill page {len(any_page):2d}")
        for page, eps in sorted(pages.items(), key=lambda kv: (-len(kv[1]), kv[0])):
            print(f"    {page:22s} {len(eps):2d}")


if __name__ == "__main__":
    main()
