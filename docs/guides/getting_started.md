# Getting Started with HAQuests

## Prerequisites

- Linux system (tested on Ubuntu 22.04)
- C++17 compatible compiler
- CMake 3.15+
- OpenSSL development libraries
- libpcap development libraries
- CAP_NET_RAW capability (or root access)

## Installation

### 1. Install Dependencies

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake libssl-dev libpcap-dev

# Fedora/RHEL
sudo dnf install gcc-c++ cmake openssl-devel libpcap-devel
```

### 2. Clone and Build

```bash
git clone https://github.com/HazaVVIP/haquests.git
cd haquests
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 3. Set Capabilities

```bash
# Option 1: Set capabilities (recommended)
sudo ../scripts/setup_caps.sh

# Option 2: Run with sudo
sudo ./examples/simple_http_get http://example.com
```

## Quick Start

### HTTP GET Request

```bash
./examples/simple_http_get http://example.com
```

### HTTPS Request

```bash
./examples/tls_connection https://www.google.com
```

## Docker Usage

```bash
docker-compose up -d haquests-dev
docker-compose exec haquests-dev bash
cd build
./examples/simple_http_get http://test-server/
```

## Next Steps

- Read the [API Documentation](../api/)
- Learn about [HTTP Smuggling](smuggling_guide.md)
- Check [Troubleshooting](troubleshooting.md)
