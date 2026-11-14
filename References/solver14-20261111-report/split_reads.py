"""Per episode of the rerun's two v2-split jobs: which of the five skill pages the agent read, by a Read of the
page or a Bash command (cat, sed, ...) naming it or /skills/*.

    cd References/solver14-20261111 && python3 ../solver14-20261111-report/split_reads.py 734919 734911

734919 is oss120b v2-split, 734911 qwen38 v2-split. Output: split_reads.txt.
"""
import json
import pathlib
import re
import sys

PAGES = ["verdict", "samples", "ladder", "verify", "frozen"]

def pages_in(text):
    got = set()
    if re.search(r"/skills/(\*|solver-preconditioners-\*)", text):
        got |= set(PAGES)
    for p in PAGES:
        if f"/skills/solver-preconditioners-{p}" in text:
            got.add(p)
    return got

for job in sys.argv[1:]:
    rows = []
    for log in sorted(pathlib.Path(job, "agents").glob("*/problem-*/claude.log"), key=lambda p: int(p.parent.name.split("-")[1])):
        read, bash = set(), set()
        for line in log.open(errors="replace"):
            try:
                ev = json.loads(line)
            except json.JSONDecodeError:
                continue
            if ev.get("type") != "assistant":
                continue
            for b in ev.get("message", {}).get("content", []):
                if b.get("type") != "tool_use":
                    continue
                inp = b.get("input", {})
                if b["name"] == "Read":
                    read |= pages_in(inp.get("file_path", ""))
                elif b["name"] == "Bash":
                    cmd = inp.get("command", "")
                    if re.search(r"\b(cat|head|tail|sed|less|more|awk|grep)\b", cmd):
                        bash |= pages_in(cmd)
        rows.append((log.parent.name, sorted(read), sorted(bash - read)))
    opened = [r for r in rows if r[1] or r[2]]
    allfive = [r for r in opened if len(set(r[1]) | set(r[2])) == 5]
    allfive_read = [r for r in opened if len(r[1]) == 5]
    print(f"{job}: episodes {len(rows)}, opened {len(opened)}, all five (any tool) {len(allfive)}, all five via Read {len(allfive_read)}")
    for r in rows:
        print("   ", r[0], "Read:", ",".join(r[1]) or "-", "| Bash only:", ",".join(r[2]) or "-")
