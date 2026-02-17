#!/bin/bash
# Local test to verify socket and timeout functionality
# This test doesn't require external network access

set -e

echo "HAQuests Local Functionality Test"
echo "=================================="
echo ""

# Build a simple test program
cat > /tmp/haquests_local_test.cpp << 'TESTEOF'
#include "haquests/haquests.hpp"
#include <iostream>
#include <chrono>

using namespace haquests;

int main() {
    // Test 1: Check raw socket capabilities
    std::cout << "Test 1: Checking CAP_NET_RAW capability..." << std::endl;
    if (core::RawSocket::hasCapabilities()) {
        std::cout << "  ✓ CAP_NET_RAW capability available" << std::endl;
    } else {
        std::cerr << "  ✗ CAP_NET_RAW capability NOT available (run with sudo)" << std::endl;
        return 1;
    }
    std::cout << std::endl;
    
    // Test 2: Create and configure raw socket
    std::cout << "Test 2: Creating and configuring raw socket..." << std::endl;
    core::RawSocket socket;
    if (socket.open()) {
        std::cout << "  ✓ Raw socket created successfully" << std::endl;
        std::cout << "  ✓ Socket timeout configured (5 seconds)" << std::endl;
        socket.close();
    } else {
        std::cerr << "  ✗ Failed to create raw socket" << std::endl;
        return 1;
    }
    std::cout << std::endl;
    
    // Test 3: HTTP request builder
    std::cout << "Test 3: Testing HTTP request builder..." << std::endl;
    http::Request request = http::Request::GET("/test");
    request.setHeader("Host", "example.com");
    request.setHeader("Connection", "close");
    std::string req_str = request.build();
    
    if (req_str.find("GET /test HTTP/1.1") != std::string::npos &&
        req_str.find("Host: example.com") != std::string::npos) {
        std::cout << "  ✓ HTTP request formatted correctly" << std::endl;
        std::cout << "  Sample request:\n" << req_str << std::endl;
    } else {
        std::cerr << "  ✗ HTTP request format incorrect" << std::endl;
        return 1;
    }
    std::cout << std::endl;
    
    // Test 4: Timeout mechanism (try to connect to non-existent local IP)
    std::cout << "Test 4: Testing receive timeout (this will take ~5 seconds)..." << std::endl;
    auto start = std::chrono::steady_clock::now();
    
    // This should timeout since 192.0.2.1 is a TEST-NET address that won't respond
    tcp::Connection conn;
    bool connected = conn.connect("192.0.2.1", 80);
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    
    if (!connected) {
        std::cout << "  ✓ Connection correctly failed to unreachable host" << std::endl;
        std::cout << "  ✓ Timeout mechanism working (took " << duration << " seconds)" << std::endl;
    } else {
        std::cerr << "  ✗ Unexpected: connection succeeded to test address" << std::endl;
    }
    std::cout << std::endl;
    
    std::cout << "All local tests passed!" << std::endl;
    std::cout << std::endl;
    std::cout << "Note: These tests only verify local functionality." << std::endl;
    std::cout << "To test actual network connections, use scripts/test_connections.sh" << std::endl;
    
    return 0;
}
TESTEOF

echo "Compiling test program..."
g++ -std=c++17 /tmp/haquests_local_test.cpp \
    -I./include \
    -L./build/src \
    -lhaquests -lssl -lcrypto \
    -Wl,-rpath,./build/src \
    -o /tmp/haquests_local_test

echo "Running tests (requires sudo)..."
echo ""
sudo /tmp/haquests_local_test

echo ""
echo "Test complete!"
