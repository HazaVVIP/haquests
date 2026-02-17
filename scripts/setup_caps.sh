#!/bin/bash

echo "Setting up CAP_NET_RAW capability..."

if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (sudo)"
    exit 1
fi

# Set capabilities on example binaries
if [ -f "build/examples/simple_http_get" ]; then
    setcap cap_net_raw,cap_net_admin=eip build/examples/simple_http_get
    echo "✓ Set capabilities on simple_http_get"
fi

if [ -f "build/examples/tls_connection" ]; then
    setcap cap_net_raw,cap_net_admin=eip build/examples/tls_connection
    echo "✓ Set capabilities on tls_connection"
fi

echo "Done! You can now run examples without sudo."
