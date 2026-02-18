#!/bin/bash
# HAQuests - Firewall Cleanup Script
# 
# This script removes the iptables rules added by setup_firewall.sh

set -e

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Error: This script must be run as root (use sudo)"
    exit 1
fi

echo "HAQuests Firewall Cleanup"
echo "========================="
echo ""

# Port range used by HAQuests
PORT_RANGE="10000:65535"

# Check if iptables is available
if ! command -v iptables &> /dev/null; then
    echo "Error: iptables is not installed"
    exit 1
fi

echo "Removing iptables rule for RST packets from ports $PORT_RANGE..."

# Remove the rule (might fail if it doesn't exist, so we use || true)
iptables -D OUTPUT -p tcp --tcp-flags RST RST --sport $PORT_RANGE -j DROP 2>/dev/null || echo "Rule not found (already removed or never added)"

echo "✓ Cleanup complete"
