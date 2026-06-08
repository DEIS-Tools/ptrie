#!/usr/bin/env python3
import json
import sys
from pathlib import Path

ARTIFACTS = Path('artifacts')
CURRENT = ARTIFACTS / 'current_results.json'
BASELINE = Path('benchmarks') / 'baseline.json'

PERF_THRESH = float(sys.argv[1]) if len(sys.argv) > 1 else 5.0
MEM_THRESH = float(sys.argv[2]) if len(sys.argv) > 2 else 5.0

if not CURRENT.exists():
    print('No current_results.json found in artifacts/; run tools/bench/run_bench.sh first')
    sys.exit(1)

current = json.loads(CURRENT.read_text())

# Ensure baseline dir exists
BASELINE.parent.mkdir(parents=True, exist_ok=True)

if not BASELINE.exists():
    print('Baseline not found; creating baseline from current results')
    BASELINE.write_text(json.dumps(current, indent=2))
    print('Baseline written to', BASELINE)
    sys.exit(0)

baseline = json.loads(BASELINE.read_text())

# Index by name
bmap = {e['name']: e for e in baseline}

failed = False
reports = []
for e in current:
    name = e['name']
    b = bmap.get(name)
    if not b:
        reports.append((name, 'no-baseline'))
        continue
    # Compare wall_seconds and peak_rss_kb
    new_t = float(e.get('wall_seconds', 0))
    old_t = float(b.get('wall_seconds', 0))
    new_m = float(e.get('peak_rss_kb', 0))
    old_m = float(b.get('peak_rss_kb', 0))

    t_pct = 0.0
    m_pct = 0.0
    if old_t > 0:
        t_pct = (new_t - old_t) / old_t * 100.0
    if old_m > 0:
        m_pct = (new_m - old_m) / old_m * 100.0

    ok_t = (t_pct <= PERF_THRESH)
    ok_m = (m_pct <= MEM_THRESH)

    reports.append((name, round(t_pct,2), round(m_pct,2), ok_t, ok_m))
    if not (ok_t and ok_m):
        failed = True

# Print report
for r in reports:
    if r[1] == 'no-baseline':
        print(f"{r[0]}: no baseline entry")
    else:
        name, t_pct, m_pct, ok_t, ok_m = r
        print(f"{name}: time change {t_pct:+.2f}% ({'OK' if ok_t else 'REGRESSION'}), mem change {m_pct:+.2f}% ({'OK' if ok_m else 'REGRESSION'})")

if failed:
    print('One or more regressions exceed thresholds. FAILING.')
    sys.exit(2)

print('All checks within thresholds')
sys.exit(0)
