#! /bin/bash

# test STRING_TO_SEND EXPECTED_RESPONSE
function test {
    local string_to_send="$1"
    local expected_response="$2"
    
    if [ $# -ne 2 ]; then
        echo "Usage: test STRING_TO_SEND EXPECTED_RESPONSE"
        return 1
    fi
    
    echo "Testing: $string_to_send"
    echo "Expected: $expected_response"
    
    # Send the string to ircserv and capture the response
    # Use timeout and -w flag to ensure connection closes properly
    local actual_response=$(echo "$string_to_send" | timeout 2 nc -w 2 localhost 6667 | tr -d '\0')
    
    echo "Actual: $actual_response"
    
    # Compare the responses
    if [ "$actual_response" = "$expected_response" ]; then
        echo "✓ TEST PASSED"
        return 0
    else
        echo "✗ TEST FAILED"
        return 1
    fi
}

# Start ircserv in background
echo "Starting IRC server..."
./ircserv 6667 password &
IRCSERV_PID=$!

# Wait a moment for the server to start
sleep 2

# Check if the server is still running
if ! kill -0 $IRCSERV_PID 2>/dev/null; then
    echo "✗ ERROR: IRC server failed to start or exited immediately"
    exit 1
fi

echo "IRC server started with PID: $IRCSERV_PID"

# Run the tests
echo "Running tests..."
test "Hello World" "Hello World"
test \
"long long string aaaaaaaaaaaaaaaaaaaaaaaaaaaa
bbbbbbbbbbbbbbbbbbbbbbbb cccccccccccccccc ddddddddddddddeeeeeeeeeeffff" \
"long long string aaaaaaaaaaaaaaaaaaaaaaaaaaaa
bbbbbbbbbbbbbbbbbbbbbbbb cccccccccccccccc ddddddddddddddeeeeeeeeeeffff"

# Check if the server is still running after tests
if ! kill -0 $IRCSERV_PID 2>/dev/null; then
    echo "✗ ERROR: IRC server exited unexpectedly during tests"
    exit 1
fi

# Terminate the IRC server
echo "Terminating IRC server..."
kill $IRCSERV_PID 2>/dev/null

# Wait for the server to terminate gracefully
sleep 1

# Force kill if it's still running
if kill -0 $IRCSERV_PID 2>/dev/null; then
    echo "Force killing IRC server..."
    kill -9 $IRCSERV_PID 2>/dev/null
fi

echo "IRC server terminated successfully"