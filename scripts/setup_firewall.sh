#!/bin/bash
# HAQuests - Firewall Setup Script
# 
# This script sets up iptables rules to prevent the kernel from sending RST packets
# for connections created using raw sockets. This is necessary because the kernel's
# TCP stack doesn't know about connections we create with raw sockets, so it would
# normally send RST packets when it receives SYN-ACK or data packets.
#
# IMPORTANT: Only run this on systems where you have permission to modify firewall rules!

set -e

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Error: This script must be run as root (use sudo)"
    exit 1
fi

echo "HAQuests Firewall Setup"
echo "======================="
echo ""
echo "This script will add iptables rules to prevent kernel RST packets"
echo "for raw socket connections. This is required for HAQuests to work properly."
echo ""

# Port range used by HAQuests for source ports (random ports between 10000-65535)
PORT_RANGE="10000:65535"

# Check if iptables is available
if ! command -v iptables &> /dev/null; then
    echo "Error: iptables is not installed"
    exit 1
fi

echo "Adding iptables rule to drop outgoing RST packets from ports $PORT_RANGE..."

# Drop outgoing RST packets from the port range used by HAQuests
# This prevents the kernel from interfering with our raw socket connections
iptables -I OUTPUT -p tcp --tcp-flags RST RST --sport $PORT_RANGE -j DROP

echo "✓ Firewall rule added successfully"
echo ""
echo "To remove this rule later, run:"
echo "  sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST --sport $PORT_RANGE -j DROP"
echo ""
echo "To make this rule persistent across reboots (on Ubuntu/Debian):"
echo "  sudo apt-get install iptables-persistent"
echo "  sudo netfilter-persistent save"
echo ""
echo "Note: This rule will be lost on reboot unless you make it persistent."
