#!/usr/bin/env bash
set -euo pipefail

# ----------------------
# Benchmark .exe-like tools from ../bin on all *.b from ../b
# Outputs test/results.csv and prints summary.
#
# Options:
#   -r ROOT_DIR   Root folder containing bin, b, test (default: parent of script dir)
#   -t TIMEOUT    Per-run timeout in seconds (default: 0 = no timeout)
# ----------------------

# --- Parse options ---
ROOT_DIR=""
TIMEOUT=0
while getopts ":r:t:" opt; do
  case "$opt" in
    r) ROOT_DIR="$OPTARG" ;;
    t) TIMEOUT="$OPTARG" ;;
    \?) echo "Unknown option: -$OPTARG" >&2; exit 2 ;;
    :)  echo "Option -$OPTARG requires an argument." >&2; exit 2 ;;
  esac
done
shift $((OPTIND-1))

# --- Resolve paths ---
SCRIPT_DIR="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
if [[ -z "${ROOT_DIR}" ]]; then
  ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
else
  ROOT_DIR="$(cd "${ROOT_DIR}" && pwd)"
fi

BIN_DIR="${ROOT_DIR}/bin"
B_DIR="${ROOT_DIR}/b"
OUT_CSV="${SCRIPT_DIR}/results.csv"

[[ -d "${BIN_DIR}" ]] || { echo "Tools folder not found: ${BIN_DIR}" >&2; exit 1; }
[[ -d "${B_DIR}"   ]] || { echo "Input folder not found: ${B_DIR}" >&2; exit 1; }

# --- Discover tools and inputs ---
# Take executable files directly under bin/; sort by name.
mapfile -d '' TOOLS < <(find "${BIN_DIR}" -maxdepth 1 -type f -name '*bf' -executable -print0 | sort -z)
mapfile -d '' INPUTS < <(find "${B_DIR}"  -maxdepth 1 -type f -name '*.b' -print0 | sort -z)

(( ${#TOOLS[@]} > 0 ))  || { echo "No executable tools found in ${BIN_DIR}." >&2; exit 1; }
(( ${#INPUTS[@]} > 0 )) || { echo "No *.b files found in ${B_DIR}." >&2; exit 1; }

echo "Tools: ${#TOOLS[@]}. Inputs: ${#INPUTS[@]}."
echo "CSV output: ${OUT_CSV}"

# --- CSV header ---
# NOTE: to keep awk summary simple, avoid commas in filenames (CSV is quoted, but summary ignores commas).
echo 'Tool,InputFile,DurationMs,ExitCode,TimedOut' > "${OUT_CSV}.tmp"

# --- Summaries (for pretty print at the end) ---
declare -A SUM_MS=()    # by tool name
declare -A COUNT=()
TOTAL_MS=0
TOTAL_RUNS=0

run_one() {
  local tool="$1" input="$2"
  local tool_name input_name
  tool_name="$(basename -- "$tool")"
  input_name="$(basename -- "$input")"

  printf '[%s] %s -> %s\n' "$(date +%H:%M:%S)" "$tool_name" "$input_name"

  # High-res timing (nanoseconds)
  local start_ns end_ns dur_ms status timed_out exit_field
  start_ns="$(date +%s%N || true)"

  if (( TIMEOUT > 0 )); then
    # timeout returns 124 on timeout
    if timeout --preserve-status "${TIMEOUT}s" "$tool" "$input" >/dev/null 2>&1; then
      status=0
    else
      status=$?
    fi
  else
    if "$tool" "$input" >/dev/null 2>&1; then
      status=0
    else
      status=$?
    fi
  fi

  end_ns="$(date +%s%N || true)"
  # Fallback if date +%s%N unsupported
  if [[ -z "$start_ns" || -z "$end_ns" || "$start_ns" == "$end_ns" ]]; then
    # Fallback to /usr/bin/time if needed
    # But in most Linux environments %N is supported; keep simple here
    start_ns=0; end_ns=0
  fi

  if [[ "$start_ns" != 0 && "$end_ns" != 0 ]]; then
    dur_ms=$(( (end_ns - start_ns) / 1000000 ))
  else
    # crude fallback: 0
    dur_ms=0
  fi

  timed_out="false"
  if (( TIMEOUT > 0 )) && (( status == 124 )); then
    timed_out="true"
    exit_field=""    # match previous behavior: no ExitCode when timed out
  else
    exit_field="${status}"
  fi

  # Append to CSV (quote strings)
  printf '"%s","%s",%d,%s,%s\n' "$tool_name" "$input_name" "$dur_ms" "$exit_field" "$timed_out" >> "${OUT_CSV}.tmp"

  # Update summaries (include timed-out durations as well)
  SUM_MS["$tool_name"]=$(( ${SUM_MS["$tool_name"]:-0} + dur_ms ))
  COUNT["$tool_name"]=$(( ${COUNT["$tool_name"]:-0} + 1 ))
  TOTAL_MS=$(( TOTAL_MS + dur_ms ))
  TOTAL_RUNS=$(( TOTAL_RUNS + 1 ))
}

# --- Main loop ---
for tool in "${TOOLS[@]}"; do
  for input in "${INPUTS[@]}"; do
    run_one "$tool" "$input"
  done
done

mv -f "${OUT_CSV}.tmp" "${OUT_CSV}"

# --- Print CSV ---
echo
echo "=== CSV results ==="
# Display nicely; fall back to cat if 'column' not available
if command -v column >/dev/null 2>&1; then
  column -s, -t < "${OUT_CSV}"
else
  cat "${OUT_CSV}"
fi

# --- Per-tool totals (pretty) ---
echo
echo "=== Per-tool totals ==="
# Header
printf "%-20s %8s %12s %14s %10s\n" "Tool" "Runs" "TotalMs" "TotalTime" "AvgMs"
# Rows
for tool_name in "${!SUM_MS[@]}"; do
  sum="${SUM_MS[$tool_name]}"
  cnt="${COUNT[$tool_name]}"
  avg=$(( sum / (cnt>0?cnt:1) ))
  # Format TotalTime as HH:MM:SS.mmm (approx)
  total_sec=$(( sum / 1000 ))
  ms_remainder=$(( sum % 1000 ))
  hh=$(( total_sec / 3600 ))
  mm=$(( (total_sec % 3600) / 60 ))
  ss=$(( total_sec % 60 ))
  printf "%-20s %8d %12d %02d:%02d:%02d.%03d %10d\n" \
    "$tool_name" "$cnt" "$sum" "$hh" "$mm" "$ss" "$ms_remainder" "$avg"
done | sort -k3,3n

# --- Overall total ---
echo
echo "=== Overall total ==="
total_sec=$(( TOTAL_MS / 1000 ))
ms_remainder=$(( TOTAL_MS % 1000 ))
hh=$(( total_sec / 3600 ))
mm=$(( (total_sec % 3600) / 60 ))
ss=$(( total_sec % 60 ))
printf "%d run(s), total: %02d:%02d:%02d.%03d (%d ms)\n" \
  "$TOTAL_RUNS" "$hh" "$mm" "$ss" "$ms_remainder" "$TOTAL_MS"
