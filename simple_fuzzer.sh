#!/bin/bash

# Simple network fuzzer client
# This sends random IRC commands to test the server

if [ $# -ne 2 ]; then
    echo "Usage: $0 <host> <port>"
    echo "Example: $0 localhost 6667"
    exit 1
fi

HOST=$1
PORT=$2

# Array of IRC commands to fuzz
COMMANDS=(
    "PASS testpass"
    "PASS wrongpass"
    "NICK testuser"
    "NICK "
    "USER testuser 0 * :Test User"
    "USER"
    "JOIN #test"
    "JOIN "
    "PRIVMSG #test :Hello"
    "PRIVMSG"
    "PART #test"
    "QUIT"
    "MODE #test +o testuser"
    "KICK #test baduser"
    "TOPIC #test :New topic"
    "INVITE user #test"
    "PING"
    "PONG"
    ""
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "NICK $(printf 'A%.0s' {1..1000})"
)

echo "Fuzzing IRC server at $HOST:$PORT"
echo "Press Ctrl+C to stop"

counter=0
while true; do
    # Pick a random command
    cmd=${COMMANDS[$RANDOM % ${#COMMANDS[@]}]}
    
    # Send it via netcat with timeout
    echo -e "$cmd\r\n" | timeout 1 nc $HOST $PORT >/dev/null 2>&1
    
    counter=$((counter + 1))
    if [ $((counter % 100)) -eq 0 ]; then
        echo "Sent $counter commands..."
    fi
    
    # Small delay to not overwhelm
    sleep 0.01
done
