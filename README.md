# HAQuests - HTTP Library dengan Raw Socket C++

> Low-Level Packet Crafting untuk Security Research dan Penetration Testing

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](https://www.kernel.org/)

## 🎯 Overview

HAQuests adalah library HTTP berbasis raw socket C++ yang dirancang untuk memberikan kontrol penuh atas network layers (L4 TCP dan L7 HTTP) untuk keperluan penetration testing dan security research.

### Key Features

- **🔧 Raw Socket Control**: Manipulasi langsung TCP packets tanpa interference dari OS
- **🔒 TLS/SSL Support**: Custom OpenSSL BIO integration untuk HTTPS
- **⚡ HTTP Request Smuggling**: Built-in support untuk CL.TE, TE.CL, TE.TE techniques
- **🛡️ WAF Bypass**: Advanced evasion techniques dan TCP segmentation control
- **📦 Modular Design**: Clean separation antara Core, TCP, TLS, dan HTTP layers

### Use Cases

- HTTP Request Smuggling research dan testing
- Web Application Firewall (WAF) bypass testing
- Penetration testing dengan custom HTTP behaviors
- Network protocol research dan education
- Parser differential analysis

## ⚠️ Disclaimer

**IMPORTANT**: This tool is designed for authorized security testing and research purposes only. Unauthorized use against systems you don't own or have explicit permission to test may violate laws and regulations.

## 📚 Documentation

Comprehensive research dan planning documentation tersedia di direktori `Developer/`:

- **[Project Overview](Developer/00-PROJECT_OVERVIEW.md)**: Tujuan, scope, dan filosofi proyek
- **[Raw Socket Research](Developer/01-RAW_SOCKET_RESEARCH.md)**: Deep dive ke raw socket programming
- **[TCP State Machine](Developer/02-TCP_STATE_MACHINE.md)**: TCP protocol implementation
- **[TLS/SSL Integration](Developer/03-TLS_SSL_INTEGRATION.md)**: OpenSSL BIO custom integration
- **[HTTP Parser & Smuggling](Developer/04-HTTP_PARSER_SMUGGLING.md)**: Request smuggling techniques
- **[Directory Structure](Developer/05-DIRECTORY_STRUCTURE.md)**: Proposed project organization
- **[Implementation Roadmap](Developer/06-IMPLEMENTATION_ROADMAP.md)**: Development phases dan timeline
- **[Testing Strategy](Developer/07-TESTING_STRATEGY.md)**: Comprehensive testing approach
- **[Research Summary](Developer/README.md)**: Executive summary dari semua research

## 🚀 Quick Start

### Prerequisites

- Linux (Ubuntu 20.04+ recommended)
- C++17 compiler (GCC 9+ atau Clang 10+)
- CMake 3.15+
- OpenSSL 1.1.1+
- libpcap
- Docker (recommended untuk development)
- CAP_NET_RAW capability atau root access

### Installation (Coming Soon)

```bash
# Clone repository
git clone https://github.com/HazaVVIP/haquests.git
cd haquests

# Initialize submodules
git submodule update --init --recursive

# Build dengan Docker (recommended)
docker-compose up -d haquests-dev
docker-compose exec haquests-dev bash

# Atau build langsung (membutuhkan CAP_NET_RAW)
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run tests
sudo ./tests/run_tests

# Install
sudo make install
```

## 📖 Usage Example (Planned)

```cpp
#include <haquests/haquests.hpp>

int main() {
    // Simple HTTP GET
    haquests::HTTPClient client;
    auto response = client.get("http://example.com/");
    std::cout << response.body << std::endl;
    
    // HTTPS dengan TLS
    auto secure_response = client.get("https://www.google.com/");
    
    // HTTP Request Smuggling (CL.TE)
    haquests::HTTPSmugglingRequest smuggling;
    std::string smuggled = "GET /admin HTTP/1.1\r\nHost: target.com\r\n\r\n";
    std::string payload = smuggling.buildCLTE(smuggled);
    client.sendRaw(payload);
    
    return 0;
}
```

## 🏗️ Project Status

**Current Phase**: Phase 0 - Foundation & Research ✅

- [x] Research completed
- [x] Development plan finalized
- [x] Directory structure planned
- [ ] Build system setup (Phase 0)
- [ ] Core layer implementation (Phase 1)
- [ ] TCP layer implementation (Phase 2)
- [ ] HTTP layer implementation (Phase 3)
- [ ] TLS layer implementation (Phase 4)
- [ ] Smuggling features (Phase 5)
- [ ] Advanced features (Phase 6)
- [ ] Testing & hardening (Phase 7)

**Estimated Timeline**: 3-5 months untuk v1.0

## 🛠️ Development

### Requirements untuk Development

- CAP_NET_RAW capability
- tcpdump/wireshark untuk debugging
- Docker (recommended)

### Development Workflow

```bash
# Setup environment
docker-compose up -d
docker-compose exec haquests-dev bash

# Build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Run tests
./tests/run_tests

# Debug dengan gdb
sudo gdb ./examples/simple_http_get
```

### Contributing

Contributions are welcome! Please read our contributing guidelines (coming soon).

## 📋 Roadmap

### Version 0.1.0 (MVP)
- Raw socket creation dan packet sending
- Basic TCP 3-way handshake
- Simple HTTP GET request

### Version 0.2.0
- Full TCP state machine
- HTTP POST/PUT/DELETE
- Chunked transfer encoding

### Version 0.3.0
- TLS/SSL support
- HTTPS requests
- Certificate verification

### Version 0.4.0
- HTTP Request Smuggling (CL.TE, TE.CL, TE.TE)
- Smuggling detection framework

### Version 1.0.0
- Complete documentation
- Performance optimization
- Production ready

## 📄 License

This project will be licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- OpenSSL team untuk TLS library
- libpcap developers
- Security research community
- PortSwigger Research (HTTP Request Smuggling)

## 📞 Contact

- GitHub: [@HazaVVIP](https://github.com/HazaVVIP)
- Repository: [haquests](https://github.com/HazaVVIP/haquests)

## ⚖️ Legal Notice

This tool is provided for educational and authorized testing purposes only. The authors are not responsible for any misuse or damage caused by this tool. Always ensure you have explicit permission before testing any system.

---

**Note**: Project ini masih dalam tahap research dan planning. Implementation akan dimulai setelah research phase selesai.
