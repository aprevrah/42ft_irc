#!/bin/bash
# AFL++ Network Fuzzing Script for IRC Server

set -e

# Configuration
PORT=${1:-6667}
PASSWORD=${2:-"testpass"}
FUZZER_DIR="$(dirname "$0")"
PROJECT_DIR="$(dirname "$FUZZER_DIR")"
INPUT_DIR="$FUZZER_DIR/inputs"
OUTPUT_DIR="$FUZZER_DIR/outputs"
TARGET="$PROJECT_DIR/ircserv_fuzz"

echo "=== AFL++ Network Fuzzing Setup for IRC Server ==="
echo "Port: $PORT"
echo "Password: $PASSWORD"
echo "Target: $TARGET"
echo "Input dir: $INPUT_DIR"
echo "Output dir: $OUTPUT_DIR"
echo

# Check if AFL++ is installed
if ! command -v afl-fuzz &> /dev/null; then
    echo "ERROR: AFL++ not found. Please install AFL++:"
    echo "  sudo apt update"
    echo "  sudo apt install afl++"
    echo "  # OR build from source:"
    echo "  git clone https://github.com/AFLplusplus/AFLplusplus"
    echo "  cd AFLplusplus && make && sudo make install"
    exit 1
fi

# Check if target exists
if [ ! -f "$TARGET" ]; then
    echo "ERROR: Target binary not found: $TARGET"
    echo "Please build the AFL++ instrumented binary first:"
    echo "  cd $PROJECT_DIR && make afl"
    exit 1
fi

# Create necessary directories
mkdir -p "$OUTPUT_DIR"

# AFL++ environment setup
export AFL_SKIP_CPUFREQ=1
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1

echo "Starting AFL++ network fuzzing..."
echo "Note: This will start the IRC server on port $PORT"
echo "Press Ctrl+C to stop fuzzing"
echo

# Method 1: Using AFL++ with external network fuzzer
echo "Starting AFL++ with network fuzzer..."
afl-fuzz -i "$INPUT_DIR" -o "$OUTPUT_DIR" -n -- python3 "$FUZZER_DIR/afl_network_fuzzer.py" "$PORT" "$PASSWORD"
