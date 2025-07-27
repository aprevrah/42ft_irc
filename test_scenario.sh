#!/bin/bash

# IRC Server Test Scenario: Two clients joining a channel and messaging
# This script tests a basic IRC interaction with two clients

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Server configuration
SERVER_HOST="localhost"
SERVER_PORT="6667"
SERVER_PASSWORD="password"

echo -e "${BLUE}=== IRC Server Test Scenario ===${NC}"
echo "Testing: Two clients joining a channel and sending messages"
echo

# Function to start the IRC server
start_server() {
    echo -e "${YELLOW}Step 1: Starting IRC server...${NC}"
    
    # Build the server first
    make clean && make -j 30
    if [ $? -ne 0 ]; then
        echo -e "${RED}✗ Failed to build IRC server${NC}"
        exit 1
    fi
    
    # Start server in background
    ./ircserv $SERVER_PORT $SERVER_PASSWORD &
    IRCSERV_PID=$!
    
    # Wait for server to start
    sleep 2
    
    # Check if server is running
    if ! kill -0 $IRCSERV_PID 2>/dev/null; then
        echo -e "${RED}✗ IRC server failed to start${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✓ IRC server started (PID: $IRCSERV_PID)${NC}"
    echo
}

# Function to stop the IRC server
stop_server() {
    echo -e "${YELLOW}Cleaning up: Stopping IRC server...${NC}"
    
    if [ ! -z "$IRCSERV_PID" ] && kill -0 $IRCSERV_PID 2>/dev/null; then
        kill $IRCSERV_PID 2>/dev/null
        sleep 1
        
        # Force kill if still running
        if kill -0 $IRCSERV_PID 2>/dev/null; then
            kill -9 $IRCSERV_PID 2>/dev/null
        fi
    fi
    
    echo -e "${GREEN}✓ IRC server stopped${NC}"
}

# Function to send IRC commands to server and capture response
send_irc_command() {
    local commands="$1"
    local timeout_duration="$2"
    
    if [ -z "$timeout_duration" ]; then
        timeout_duration=3
    fi
    
    # Use printf to properly handle \r\n line endings
    printf "%s" "$commands" | timeout $timeout_duration nc $SERVER_HOST $SERVER_PORT 2>/dev/null
}

# Main test scenario
run_test_scenario() {
    echo -e "${YELLOW}Step 2: Testing Client 1 connection and registration...${NC}"
    
    # Client 1: Connect, authenticate, and join channel (proper IRC format with \r\n)
    local client1_commands=$'PASS password\r\nNICK alice\r\nUSER alice 0 * :Alice User\r\nJOIN #testchannel\r\nQUIT :Leaving\r\n'
    
    echo "Connecting Client 1 (alice)..."
    local client1_response=$(send_irc_command "$client1_commands" 5)
    echo "Client 1 response:"
    echo "$client1_response" | head -10
    echo
    echo "$client1_response" > client1_response
    
    echo -e "${YELLOW}Step 3: Testing Client 2 connection and channel interaction...${NC}"
    
    # Give some time for client 1 to establish presence
    sleep 1
    
    # Client 2: Connect, authenticate, join same channel, send message
    local client2_commands=$'PASS password\r\nNICK bob\r\nUSER bob 0 * :Bob User\r\nJOIN #testchannel\r\nPRIVMSG #testchannel :Hello everyone!\r\nQUIT :Leaving\r\n'
    
    echo "Connecting Client 2 (bob)..."
    local client2_response=$(send_irc_command "$client2_commands" 5)
    echo "Client 2 response:"
    echo "$client2_response" | head -10
    echo
    echo "$client2_response" > client2_response
    
    echo -e "${YELLOW}Step 4: Testing message exchange scenario...${NC}"
    
    # For a more realistic test, we'll use background processes
    # to simulate concurrent clients
    test_concurrent_messaging
}

# Function to test concurrent messaging between two clients
test_concurrent_messaging() {
    echo "Setting up concurrent client connections..."
    
    # Create named pipes for communication
    local pipe1="/tmp/irc_client1_$$"
    local pipe2="/tmp/irc_client2_$$"
    
    mkfifo "$pipe1" "$pipe2"
    
    # Start client 1 in background with proper IRC formatting
    {
        printf "PASS password\r\n"
        printf "NICK alice\r\n"
        printf "USER alice 0 * :Alice User\r\n"
        sleep 1
        printf "JOIN #testchannel\r\n"
        sleep 2
        printf "PRIVMSG #testchannel :Hello from Alice!\r\n"
        sleep 1
        printf "QUIT :Alice leaving\r\n"
    } | nc $SERVER_HOST $SERVER_PORT > "$pipe1" &
    local client1_pid=$!
    
    # Start client 2 in background with proper IRC formatting
    {
        sleep 1  # Let alice connect first
        printf "PASS password\r\n"
        printf "NICK bob\r\n"
        printf "USER bob 0 * :Bob User\r\n"
        sleep 1
        printf "JOIN #testchannel\r\n"
        sleep 1
        printf "PRIVMSG #testchannel :Hi Alice! This is Bob.\r\n"
        sleep 1
        printf "QUIT :Bob leaving\r\n"
    } | nc $SERVER_HOST $SERVER_PORT > "$pipe2" &
    local client2_pid=$!
    
    # Wait a bit and then collect responses
    sleep 6
    
    echo "Alice's session output:"
    if [ -r "$pipe1" ]; then
        timeout 1 cat "$pipe1" | tee out_Alice | head -20
    fi
    echo
    
    echo "Bob's session output:"
    if [ -r "$pipe2" ]; then
        timeout 1 cat "$pipe2" | tee out_Bob | head -20
    fi
    echo
    
    # Cleanup
    kill $client1_pid $client2_pid 2>/dev/null
    rm -f "$pipe1" "$pipe2"
    
    echo -e "${GREEN}✓ Concurrent messaging test completed${NC}"
}

# Trap to ensure cleanup on exit
trap stop_server EXIT

# Main execution
main() {
    echo -e "${BLUE}Starting IRC test scenario...${NC}"
    echo
    
    # Step 1: Start the server
    start_server
    
    # Step 2-4: Run the test scenario
    run_test_scenario
    
    # Final status
    echo -e "${GREEN}=== Test Scenario Completed ===${NC}"
    echo
}

# Run the main function
main
