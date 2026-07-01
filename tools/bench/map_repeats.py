#!/usr/bin/env python3
import subprocess
import sys
from pathlib import Path
import json

N = int(sys.argv[1]) if len(sys.argv) > 1 else 7
repo = Path.cwd()
# find Map binary
bin_path = None
for p in repo.glob('build-multi/**/Map'):
    if p.is_file() and p.stat().st_mode & 0o111:
        bin_path = p
        break
if not bin_path:
    print('Map binary not found under build-multi/; run a build first', file=sys.stderr)
    sys.exit(2)

art = repo / 'artifacts'
art.mkdir(exist_ok=True)
results = []

for i in range(1, N+1):
    time_file = art / f'map_time_{i}.txt'
    print(f'Run {i}/{N}: executing {bin_path}')
    # use /usr/bin/time to capture wall seconds and max RSS
    cmd = ['/usr/bin/time', '-f', '%e %M', '-o', str(time_file), str(bin_path)]
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError:
        # still try to read time file
        pass
    # read time file
    if time_file.exists():
        txt = time_file.read_text().strip()
        if txt:
            parts = txt.split()
            wall = float(parts[0])
            rss_kb = int(parts[1])
        else:
            wall = 0.0
            rss_kb = 0
    else:
        wall = 0.0
        rss_kb = 0
    results.append({'run': i, 'wall_seconds': wall, 'peak_rss_kb': rss_kb})

# compute median
walls = sorted(r['wall_seconds'] for r in results)
rsss = sorted(r['peak_rss_kb'] for r in results)
if len(walls) == 0:
    median_wall = 0.0
elif len(walls) % 2 == 1:
    median_wall = walls[len(walls)//2]
else:
    a = walls[len(walls)//2 - 1]
    b = walls[len(walls)//2]
    median_wall = (a + b) / 2.0

if len(rsss) == 0:
    median_rss = 0
elif len(rsss) % 2 == 1:
    median_rss = rsss[len(rsss)//2]
else:
    a = rsss[len(rsss)//2 - 1]
    b = rsss[len(rsss)//2]
    median_rss = int((a + b) / 2)

# read baseline
baseline_file = repo / 'benchmarks' / 'baseline.json'
baseline_map = None
if baseline_file.exists():
    baseline = json.loads(baseline_file.read_text())
    for e in baseline:
        if e.get('name') == 'Map':
            baseline_map = float(e.get('wall_seconds', 0.0))
            break

# compare
pct = None
status = 'no-baseline'
if baseline_map is not None and baseline_map > 0:
    pct = (median_wall - baseline_map) / baseline_map * 100.0
    status = 'REGRESSION' if pct > 5.0 else 'OK'

out = {
    'runs': results,
    'median_wall_seconds': median_wall,
    'median_peak_rss_kb': median_rss,
    'baseline_map_wall_seconds': baseline_map,
    'pct_change_vs_baseline': pct,
    'status': status
}

out_file = art / 'map_repeats_results.json'
out_file.write_text(json.dumps(out, indent=2))

print(json.dumps(out, indent=2))
if status == 'REGRESSION':
    sys.exit(3)
sys.exit(0)
