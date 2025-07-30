#!/usr/bin/env python3
"""
AFL++ Network Fuzzer for IRC Server
This script acts as a bridge between AFL++ and your IRC server for network fuzzing.
"""

import socket
import sys
import time
import signal
import subprocess
import os
from threading import Thread

class IRCFuzzer:
    def __init__(self, host="127.0.0.1", port=6667, password="testpass"):
        self.host = host
        self.port = port
        self.password = password
        self.server_process = None
        
    def start_server(self):
        """Start the IRC server process"""
        try:
            # Start the AFL++ instrumented server
            self.server_process = subprocess.Popen(
                ["./ircserv_fuzz", str(self.port), self.password],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            # Give the server a moment to start
            time.sleep(0.5)
            return True
        except Exception as e:
            print(f"Failed to start server: {e}")
            return False
    
    def stop_server(self):
        """Stop the IRC server process"""
        if self.server_process:
            self.server_process.terminate()
            try:
                self.server_process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.server_process.kill()
            self.server_process = None
    
    def send_commands(self, commands):
        """Send IRC commands to the server"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5)  # 5 second timeout
            sock.connect((self.host, self.port))
            
            # Send each command
            for cmd in commands:
                if cmd.strip():  # Skip empty lines
                    sock.send((cmd.strip() + "\r\n").encode())
                    time.sleep(0.1)  # Small delay between commands
            
            # Try to receive response (optional, to trigger more code paths)
            try:
                response = sock.recv(1024)
                print(f"Response: {response.decode(errors='ignore')[:100]}")
            except:
                pass
                
            sock.close()
            return True
            
        except Exception as e:
            print(f"Connection error: {e}")
            return False
    
    def fuzz_from_stdin(self):
        """Read fuzzing input from stdin and send to server"""
        # Read the fuzzing input from AFL++
        fuzz_data = sys.stdin.buffer.read()
        
        if not fuzz_data:
            return False
            
        # Convert to string and split into commands
        try:
            commands = fuzz_data.decode('utf-8', errors='ignore').split('\n')
        except:
            commands = [fuzz_data.decode('latin-1', errors='ignore')]
        
        return self.send_commands(commands)

def signal_handler(signum, frame):
    """Handle shutdown signals"""
    print("Shutting down...")
    sys.exit(0)

def main():
    if len(sys.argv) != 3:
        print("Usage: python3 afl_network_fuzzer.py <port> <password>")
        sys.exit(1)
    
    port = int(sys.argv[1])
    password = sys.argv[2]
    
    # Set up signal handlers
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    fuzzer = IRCFuzzer(port=port, password=password)
    
    # Start server
    if not fuzzer.start_server():
        print("Failed to start server")
        sys.exit(1)
    
    try:
        # Perform fuzzing
        success = fuzzer.fuzz_from_stdin()
        if not success:
            sys.exit(1)
    finally:
        # Always clean up
        fuzzer.stop_server()

if __name__ == "__main__":
    main()
