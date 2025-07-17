#! /bin/bash

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Test counters
TESTS_PASSED=0
TESTS_FAILED=0

# test STRING_TO_SEND EXPECTED_RESPONSE
function test {
    local string_to_send="$1"
    local expected_response="$2"
    
    if [ $# -ne 2 ]; then
        echo "Usage: test STRING_TO_SEND EXPECTED_RESPONSE"
        return 1
    fi
    
    echo "========================"
    echo
    echo "Testing: '$string_to_send'"
    
    # Send the string to ircserv and capture the response
    # Use timeout and -w flag to ensure connection closes properly
    local actual_response=$(echo "$string_to_send" | timeout 1 nc -w 2 localhost 6667 | tr -d '\0')
    
    echo ""
    echo -e "Expected: '$expected_response\n'"
    echo -e "Actual: '$actual_response\n'"
    
    # Compare the responses
    if [ "$actual_response" = "$expected_response" ]; then
        echo -e "${GREEN}✓ TEST PASSED${NC}"
        ((TESTS_PASSED++))
        return 0
    else
        echo -e "${RED}✗ TEST FAILED${NC}"
        ((TESTS_FAILED++))
        echo ""
        return 1
    fi
}

# Function to display test report and exit appropriately
function report_and_exit {
    local total_tests=$((TESTS_PASSED + TESTS_FAILED))
    
    echo "========================"
    echo "TEST REPORT"
    echo "========================"
    echo "Total tests: $total_tests"
    echo -e "Passed: ${GREEN}$TESTS_PASSED${NC}"
    echo -e "Failed: ${RED}$TESTS_FAILED${NC}"
    echo "========================"
    
    if [ $TESTS_FAILED -gt 0 ]; then
        echo -e "${RED}Some tests failed!${NC}"
        exit 1
    else
        echo -e "${GREEN}All tests passed!${NC}"
        exit 0
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
WELCOME=$'001 tester :Welcome to the Internet Relay Networktester!\r
002 tester :Your host is our.server42.at.\r
003  :This server was created today.\r'

test $'PASS\r\n' $'461 * PASS :Not enough parameters\r'
test $'PASS password\r\nNICK tester\r\nUSER username 2 3 4\r\n' "$WELCOME"
test $'PASS wrong_password\r\nNICK tester565\r\nUSER username 2 3 4\r\n' $'464 tester565 :Password incorrect\r'
test $'PING :hello\r\n' $'PONG AmazingServer hello\r'
test $'CAP LS 302\r\n' $'CAP * LS :\r'

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
echo

# Display final report and exit
report_and_exit