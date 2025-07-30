#!/bin/bash

echo "=== Testing AFL++ IRC Server Setup ==="

# Check if binaries exist
if [ ! -f "./ircserv_fuzz" ]; then
    echo "ERROR: ircserv_fuzz not found. Run 'make afl' first."
    exit 1
fi

if [ ! -f "./ircserv" ]; then
    echo "WARNING: ircserv not found. Run 'make' to build normal version."
fi

echo "✓ AFL++ instrumented binary found"

# Test if we can start the server briefly
echo "Testing server startup..."
timeout 2 ./ircserv_fuzz 6667 testpass &
SERVER_PID=$!
sleep 1

if kill -0 $SERVER_PID 2>/dev/null; then
    echo "✓ Server starts successfully"
    kill $SERVER_PID 2>/dev/null
else
    echo "✗ Server failed to start"
    exit 1
fi

echo ""
echo "=== Setup Complete! ==="
echo ""
echo "To run manual testing:"
echo "1. Terminal 1: ./run_fuzz_server.sh"
echo "2. Terminal 2: nc localhost 6667"
echo ""
echo "To run automated fuzzing:"
echo "1. Terminal 1: ./run_fuzz_server.sh" 
echo "2. Terminal 2: ./simple_fuzzer.sh localhost 6667"
echo ""
echo "Monitor server output for crashes or unusual behavior."
echo "The AFL++ instrumentation will provide coverage feedback."
