"""One standalone SQLite file for the scicomp40 runs: observations, setups and every source, deduplicated by hash."""
import csv
import hashlib
import pathlib
import sqlite3

import pandas as pd

HERE = pathlib.Path(__file__).parent
DB = HERE / "scicomp40.sqlite"
DAMAGED: set[str] = set()  # 714849/714850 rerun the two qwen38 HIP arms the HIP grading wedge damaged

observations = pd.read_csv(HERE / "extract/llr40_observations.csv", low_memory=False)
index = pd.read_csv(HERE / "extract/llr40_sources_index.csv", low_memory=False)
DB.unlink(missing_ok=True)
with sqlite3.connect(DB) as db:
    observations.to_sql("observations", db, index=False)
    index.to_sql("source_index", db, index=False)
    setups = observations.groupby(["setup", "job"], as_index=False).size().rename(columns={"size": "n_rows"})
    setups["usable"] = [0 if s in DAMAGED else 1 for s in setups.setup]
    setups["note"] = ["HIP grading wedge, see REPORT" if s in DAMAGED else "" for s in setups.setup]
    setups.to_sql("setups", db, index=False)
    db.execute("CREATE TABLE sources (sha256 TEXT PRIMARY KEY, text TEXT NOT NULL)")
    for rel in index.rel_path.dropna().unique():
        data = (HERE / "extract" / rel).read_bytes()
        db.execute("INSERT OR IGNORE INTO sources VALUES (?, ?)",
                   (hashlib.sha256(data).hexdigest(), data.decode("utf-8", "replace")))
    db.execute("CREATE INDEX obs_setup_kernel ON observations(setup, kernel)")
    db.execute("CREATE INDEX idx_kernel ON source_index(setup, kernel)")
with sqlite3.connect(DB) as db:
    db.execute("VACUUM")
    unmatched = db.execute("SELECT count(*) FROM source_index i LEFT JOIN sources s ON s.sha256 = i.sha256 "
                           "WHERE s.sha256 IS NULL").fetchone()[0]
    print("sources", db.execute("SELECT count(*) FROM sources").fetchone()[0], "index rows", len(index),
          "index rows with no matching hash", unmatched)
