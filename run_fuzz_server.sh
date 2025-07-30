#!/bin/bash

# Simple AFL++ network fuzzing script
# Run this in one terminal, then use netcat or telnet to connect to port 6667

PORT=6667
PASSWORD="testpass"

echo "Starting AFL++ instrumented IRC server on port $PORT"
echo "Password: $PASSWORD"
echo ""
echo "To test manually, open another terminal and run:"
echo "  nc localhost $PORT"
echo "Then type IRC commands like:"
echo "  PASS $PASSWORD"
echo "  NICK testuser" 
echo "  USER testuser 0 * :Test User"
echo ""

# Set AFL environment
export AFL_SKIP_CPUFREQ=1

# Start the server
echo "Starting server..."
./ircserv_fuzz $PORT $PASSWORD
