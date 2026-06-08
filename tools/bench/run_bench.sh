#!/usr/bin/env bash
set -euo pipefail

# Build non-sanitized multi preset (heaptrack requires non-sanitized binaries)
cmake --preset multi
cmake --build --preset release-deb

mkdir -p artifacts
# Collect JSON object strings in a bash array to avoid sed quoting issues
declare -a results_arr=()

# Test executable names expected by CMakeLists
bins=(Set Map Delete StableSet)

echo "Looking for test executables under build-multi/"
for name in "${bins[@]}"; do
  bin=$(find build-multi -type f -executable -name "$name" -print -quit || true)
  if [ -z "$bin" ]; then
    echo "Warning: binary $name not found; skipping"
    continue
  fi
  # Make path absolute so heaptrack can run it from artifacts dir
  bin="$(realpath "$bin")"

  # Use absolute paths so we can cd into artifacts safely
  repo_root="$(pwd)"
  artifacts_dir="$repo_root/artifacts"
  out_heap="$artifacts_dir/heap-${name}.zst"   # prefer .zst (heaptrack default)
  out_txt="$artifacts_dir/heap-${name}.txt"
  out_time="$artifacts_dir/time-${name}.txt"

  echo "Profiling $name -> $out_heap"

  # Run under /usr/bin/time to capture wall-time and max RSS, and run heaptrack in the artifacts dir
  pushd "$artifacts_dir" >/dev/null
  /usr/bin/time -f "%e %M" -o "$out_time" heaptrack "$bin" || true

  # Find the produced heaptrack file (heaptrack.<name>.<pid>.(zst|gz)) and rename it to a stable name
  latest_heaptrack=$(ls -t heaptrack*.gz heaptrack*.zst 2>/dev/null | head -n1 || true)
  if [ -n "$latest_heaptrack" ]; then
    ext="${latest_heaptrack##*.}"
    mv -- "$latest_heaptrack" "heap-${name}.${ext}" || true
    out_heap="$artifacts_dir/heap-${name}.${ext}"
  else
    echo "Warning: heaptrack did not produce an output file for $name"
  fi

  # Convert heaptrack database to a textual summary (if available)
  if command -v heaptrack_print >/dev/null 2>&1 && [ -f "$out_heap" ]; then
    heaptrack_print "$out_heap" > "$out_txt" || true
  fi
  popd >/dev/null

  # Parse time file
  if [ -f "$out_time" ]; then
    read -r wall_sec max_rss_kb < "$out_time"
  else
    wall_sec=0
    max_rss_kb=0
  fi

  # Append JSON entry
  entry=$(printf '{"name":"%s","binary":"%s","wall_seconds":%s,"peak_rss_kb":%s,"heaptrack":"%s","heaptrack_summary":"%s"}' \
    "$name" "$bin" "$wall_sec" "$max_rss_kb" "$out_heap" "$out_txt")

  # Append to array
  results_arr+=("$entry")

done

# Serialize results array to JSON
if [ ${#results_arr[@]} -eq 0 ]; then
  results_json='[]'
else
  # Join entries with commas
  IFS=,
  joined="${results_arr[*]}"
  unset IFS
  results_json="[${joined}]"
fi

# Write results
mkdir -p artifacts
echo "$results_json" > artifacts/current_results.json

echo "Results written to artifacts/current_results.json"
ls -lah artifacts || true
