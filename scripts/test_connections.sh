#!/bin/bash
# HAQuests - Connection Test Script
#
# This script tests the HTTP and TLS connection functionality.
# It requires external network access to work properly.

set -e

echo "HAQuests Connection Test"
echo "========================"
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Error: This script must be run as root (use sudo)"
    echo "Raw sockets require CAP_NET_RAW capability"
    exit 1
fi

# Check if firewall rules are set up
if ! iptables -L OUTPUT -n | grep -q "tcp spts:10000:65535"; then
    echo "Warning: Firewall rules not found!"
    echo "Run './scripts/setup_firewall.sh' first to prevent kernel RST interference."
    echo ""
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Find the examples directory
if [ -d "./build/examples" ]; then
    EXAMPLES_DIR="./build/examples"
elif [ -d "../examples" ]; then
    EXAMPLES_DIR="../examples"
else
    echo "Error: Could not find examples directory"
    echo "Please run this script from the repository root or build directory"
    exit 1
fi

echo "Testing HTTP connection..."
echo "--------------------------"
echo ""
echo "Testing with example.com (HTTP)..."
timeout 15 "$EXAMPLES_DIR/simple_http_get" http://example.com || {
    echo "Warning: HTTP test to example.com failed or timed out"
    echo "This might be due to network restrictions or example.com not responding to raw HTTP"
}
echo ""

echo "Testing TLS connection..."
echo "-------------------------"
echo ""
echo "Testing with www.google.com (HTTPS)..."
timeout 15 "$EXAMPLES_DIR/tls_connection" www.google.com || {
    echo "Warning: TLS test to www.google.com failed or timed out"
    echo "This might be due to network restrictions"
}
echo ""

echo "Testing with github.com (HTTPS)..."
timeout 15 "$EXAMPLES_DIR/tls_connection" github.com || {
    echo "Warning: TLS test to github.com failed or timed out"
    echo "This might be due to network restrictions"
}
echo ""

echo "Test complete!"
echo ""
echo "Notes:"
echo "- If tests failed, check your network connectivity"
echo "- Some hosts (like github.com) may not respond to HTTP on port 80"
echo "- The examples use raw sockets which may be blocked by some firewalls"
echo "- For best results, test against servers you control"
