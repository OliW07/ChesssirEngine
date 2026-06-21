#!/bin/bash
set -e

NAME="$1"
DEPTH="${2:-5}"
ENGINE="build/chess_engine"
LOG="bench_${NAME}.txt"
PERF_DATA="perf_${NAME}.data"

[ $# -lt 1 ] && echo "Usage: $0 <name> [depth]" && exit 1
[ ! -f "$ENGINE" ] && echo "Build first: cd build && cmake .. && make -j" && exit 1

nps=$(echo -e "bench $DEPTH\nquit" | timeout 120 "$ENGINE" 2>/dev/null | grep "^Total" | awk '{print $(NF-1)}')

raw=$(echo -e "bench $DEPTH\nquit" | timeout 120 perf stat -r 3 "$ENGINE" 2>&1)

instructions=$(echo "$raw" | grep -E '^\s+[0-9,]+.*instructions' | head -1 | awk '{print $1}')
cycles=$(echo "$raw" | grep -E '^\s+[0-9,]+.*cpu-cycles' | head -1 | awk '{print $1}')
ipc=$(echo "$raw" | grep "insn_per_cycle" | awk '{print $4}')
branch_misses=$(echo "$raw" | grep "branch-misses" | head -1 | awk '{print $1}')
branches=$(echo "$raw" | grep -E '^\s+[0-9,]+.*branches ' | head -1 | awk '{print $1}')
elapsed=$(echo "$raw" | grep "seconds time elapsed" | awk '{print $1}')

echo -e "bench $DEPTH\nquit" | timeout 120 perf record -g -o "$PERF_DATA" "$ENGINE" > /dev/null 2>&1

hot=$(perf report -i "$PERF_DATA" --stdio --no-children --percent-limit 2 2>/dev/null \
  | grep -E '^\s+[0-9]+\.[0-9]+%' \
  | head -15 \
  | sed 's/^[ \t]*//' \
  | sed 's/\[\.\]//g')

cat > "$LOG" <<EOF
Benchmark: $NAME
Depth:     $DEPTH
----------------------------------------
Nodes per second : $nps
Instructions     : $instructions
Cycles           : $cycles
IPC              : $ipc
Branch misses    : $branch_misses
Total branches   : $branches
Wall time        : ${elapsed}s
----------------------------------------
Hot functions (top 15):
$hot
EOF

cat "$LOG"
