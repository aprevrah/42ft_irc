#!/bin/bash
# Alternative fuzzing method using preeny to redirect network calls to stdin/stdout

set -e

FUZZER_DIR="$(dirname "$0")"
PROJECT_DIR="$(dirname "$FUZZER_DIR")"
INPUT_DIR="$FUZZER_DIR/inputs"
OUTPUT_DIR="$FUZZER_DIR/outputs"
TARGET="$PROJECT_DIR/ircserv_fuzz"
PORT=${1:-6667}
PASSWORD=${2:-"testpass"}

echo "=== Alternative AFL++ Fuzzing using preeny ==="
echo

# Check if preeny is available
if ! ldconfig -p | grep -q desock; then
    echo "INFO: preeny not found. Installing preeny for socket redirection..."
    echo "This allows AFL++ to fuzz network applications by redirecting sockets to stdin/stdout"
    echo
    echo "To install preeny:"
    echo "  git clone https://github.com/zardus/preeny"
    echo "  cd preeny && make"
    echo "  sudo cp src/desock.so /usr/local/lib/"
    echo "  sudo ldconfig"
    echo
    echo "After installing preeny, you can run:"
    echo "  LD_PRELOAD=/usr/local/lib/desock.so afl-fuzz -i $INPUT_DIR -o $OUTPUT_DIR -- $TARGET $PORT $PASSWORD"
    echo
else
    echo "preeny found! Running AFL++ with socket redirection..."
    export AFL_SKIP_CPUFREQ=1
    export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
    
    mkdir -p "$OUTPUT_DIR"
    
    echo "Starting AFL++ with preeny desock..."
    LD_PRELOAD=desock.so afl-fuzz -i "$INPUT_DIR" -o "$OUTPUT_DIR" -- "$TARGET" "$PORT" "$PASSWORD"
fi
