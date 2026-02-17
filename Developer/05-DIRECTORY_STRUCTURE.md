# Proposed Directory Structure

## Overview

Struktur direktori dirancang untuk modular, testable, dan maintainable. Setiap komponen network layer terpisah untuk memudahkan development dan testing.

## Directory Tree

```
haquests/
├── README.md
├── LICENSE
├── .gitignore
├── CMakeLists.txt                 # Build configuration
├── Dockerfile                     # Container untuk testing dengan CAP_NET_RAW
├── docker-compose.yml             # Development environment
│
├── include/                       # Public headers
│   └── haquests/
│       ├── core/
│       │   ├── packet.hpp        # Raw packet structures (IP, TCP headers)
│       │   ├── socket.hpp        # Raw socket wrapper
│       │   ├── checksum.hpp      # Checksum calculations
│       │   └── types.hpp         # Common types dan constants
│       │
│       ├── tcp/
│       │   ├── connection.hpp    # TCP connection management
│       │   ├── state_machine.hpp # TCP state machine
│       │   ├── segment.hpp       # TCP segment handling
│       │   └── window.hpp        # Window management
│       │
│       ├── tls/
│       │   ├── connection.hpp    # TLS connection wrapper
│       │   ├── bio_adapter.hpp   # Custom BIO for raw socket
│       │   ├── certificate.hpp   # Certificate handling
│       │   └── session.hpp       # Session management
│       │
│       ├── http/
│       │   ├── request.hpp       # HTTP request builder
│       │   ├── response.hpp      # HTTP response parser
│       │   ├── smuggling.hpp     # Smuggling techniques
│       │   ├── chunked.hpp       # Chunked encoding
│       │   └── headers.hpp       # Header manipulation
│       │
│       ├── utils/
│       │   ├── buffer.hpp        # Buffer management
│       │   ├── logger.hpp        # Logging utilities
│       │   ├── timer.hpp         # Timing utilities
│       │   └── error.hpp         # Error handling
│       │
│       └── haquests.hpp          # Main include (includes all)
│
├── src/                          # Implementation files
│   ├── core/
│   │   ├── packet.cpp
│   │   ├── socket.cpp
│   │   └── checksum.cpp
│   │
│   ├── tcp/
│   │   ├── connection.cpp
│   │   ├── state_machine.cpp
│   │   ├── segment.cpp
│   │   └── window.cpp
│   │
│   ├── tls/
│   │   ├── connection.cpp
│   │   ├── bio_adapter.cpp
│   │   ├── certificate.cpp
│   │   └── session.cpp
│   │
│   ├── http/
│   │   ├── request.cpp
│   │   ├── response.cpp
│   │   ├── smuggling.cpp
│   │   ├── chunked.cpp
│   │   └── headers.cpp
│   │
│   └── utils/
│       ├── buffer.cpp
│       ├── logger.cpp
│       ├── timer.cpp
│       └── error.cpp
│
├── tests/                        # Unit tests dan integration tests
│   ├── unit/
│   │   ├── test_checksum.cpp
│   │   ├── test_packet.cpp
│   │   ├── test_tcp_state.cpp
│   │   ├── test_http_parser.cpp
│   │   └── test_chunked.cpp
│   │
│   ├── integration/
│   │   ├── test_tcp_handshake.cpp
│   │   ├── test_tls_connection.cpp
│   │   ├── test_http_request.cpp
│   │   └── test_smuggling.cpp
│   │
│   └── fixtures/
│       ├── test_packets/         # Pre-crafted packet samples
│       └── test_responses/       # Sample HTTP responses
│
├── examples/                     # Example programs
│   ├── simple_http_get.cpp      # Basic HTTP GET request
│   ├── tls_connection.cpp       # HTTPS connection
│   ├── tcp_handshake.cpp        # Raw TCP handshake
│   ├── smuggling_clte.cpp       # CL.TE smuggling demo
│   ├── smuggling_tecl.cpp       # TE.CL smuggling demo
│   └── waf_bypass.cpp           # WAF bypass techniques
│
├── tools/                        # Utility tools
│   ├── packet_inspector/        # Tool untuk inspect packets
│   │   ├── main.cpp
│   │   └── CMakeLists.txt
│   │
│   ├── smuggling_tester/        # Tool untuk test smuggling
│   │   ├── main.cpp
│   │   └── CMakeLists.txt
│   │
│   └── benchmark/               # Performance benchmarks
│       ├── main.cpp
│       └── CMakeLists.txt
│
├── scripts/                     # Build dan development scripts
│   ├── build.sh                # Build script
│   ├── test.sh                 # Run tests
│   ├── format.sh               # Code formatting
│   ├── setup_caps.sh           # Setup CAP_NET_RAW
│   └── docker_build.sh         # Build Docker image
│
├── docs/                       # Documentation
│   ├── api/                    # API documentation
│   │   ├── core.md
│   │   ├── tcp.md
│   │   ├── tls.md
│   │   └── http.md
│   │
│   ├── guides/                 # User guides
│   │   ├── getting_started.md
│   │   ├── smuggling_guide.md
│   │   ├── waf_bypass.md
│   │   └── troubleshooting.md
│   │
│   ├── internals/              # Internal documentation
│   │   ├── tcp_state_machine.md
│   │   ├── tls_integration.md
│   │   └── performance.md
│   │
│   └── legal/                  # Legal dan ethical guidelines
│       ├── disclaimer.md
│       ├── responsible_use.md
│       └── license.md
│
├── Developer/                  # Research dan planning (current)
│   ├── 00-PROJECT_OVERVIEW.md
│   ├── 01-RAW_SOCKET_RESEARCH.md
│   ├── 02-TCP_STATE_MACHINE.md
│   ├── 03-TLS_SSL_INTEGRATION.md
│   ├── 04-HTTP_PARSER_SMUGGLING.md
│   ├── 05-DIRECTORY_STRUCTURE.md (this file)
│   ├── 06-IMPLEMENTATION_ROADMAP.md
│   └── 07-TESTING_STRATEGY.md
│
└── third_party/               # Third-party dependencies
    ├── googletest/            # Testing framework (git submodule)
    └── catch2/                # Alternative testing (optional)
```

## Build System (CMake)

### Root CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.15)
project(HAQuests VERSION 0.1.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Dependencies
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PCAP REQUIRED libpcap)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
    ${PCAP_INCLUDE_DIRS}
)

# Library target
add_subdirectory(src)

# Examples (optional)
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests (optional)
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Tools (optional)
option(BUILD_TOOLS "Build utility tools" ON)
if(BUILD_TOOLS)
    add_subdirectory(tools)
endif()

# Install rules
install(DIRECTORY include/haquests
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")
```

### src/CMakeLists.txt
```cmake
# Collect all source files
file(GLOB_RECURSE HAQUESTS_SOURCES
    core/*.cpp
    tcp/*.cpp
    tls/*.cpp
    http/*.cpp
    utils/*.cpp
)

# Create library
add_library(haquests SHARED ${HAQUESTS_SOURCES})

# Link dependencies
target_link_libraries(haquests
    OpenSSL::SSL
    OpenSSL::Crypto
    ${PCAP_LIBRARIES}
    pthread
)

# Set library properties
set_target_properties(haquests PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    PUBLIC_HEADER include/haquests/haquests.hpp
)

# Install library
install(TARGETS haquests
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/haquests)
```

## Docker Configuration

### Dockerfile
```dockerfile
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libpcap-dev \
    git \
    gdb \
    tcpdump \
    wireshark-common \
    iptables \
    && rm -rf /var/lib/apt/lists/*

# Create working directory
WORKDIR /workspace

# Copy source code
COPY . /workspace

# Build
RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Set capabilities for raw socket
RUN setcap cap_net_raw,cap_net_admin=eip /workspace/build/examples/simple_http_get || true

CMD ["/bin/bash"]
```

### docker-compose.yml
```yaml
version: '3.8'

services:
  haquests-dev:
    build: .
    container_name: haquests-dev
    cap_add:
      - NET_RAW
      - NET_ADMIN
    volumes:
      - .:/workspace
      - /var/run/docker.sock:/var/run/docker.sock
    networks:
      - haquests-net
    command: /bin/bash

  test-server:
    image: nginx:alpine
    container_name: test-server
    networks:
      - haquests-net
    ports:
      - "8080:80"

networks:
  haquests-net:
    driver: bridge
```

## .gitignore

```gitignore
# Build artifacts
build/
bin/
lib/
*.o
*.a
*.so
*.dylib

# CMake
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
Makefile
*.cmake
!CMakeLists.txt

# IDE
.vscode/
.idea/
*.swp
*.swo
*~

# Compiled binaries
examples/simple_http_get
examples/tls_connection
tools/*/bin/

# Test artifacts
tests/results/
*.log

# OS files
.DS_Store
Thumbs.db

# Temporary files
/tmp/
*.tmp
```

## Dependency Management

### Using Git Submodules
```bash
# Add GoogleTest
git submodule add https://github.com/google/googletest.git third_party/googletest

# Add Catch2 (alternative)
git submodule add https://github.com/catchorg/Catch2.git third_party/catch2
```

### CMakeLists.txt untuk Tests
```cmake
# Add GoogleTest
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/googletest)
include_directories(${PROJECT_SOURCE_DIR}/third_party/googletest/googletest/include)

# Test executable
add_executable(run_tests
    unit/test_checksum.cpp
    unit/test_packet.cpp
    # ... other tests
)

target_link_libraries(run_tests
    haquests
    gtest
    gtest_main
    pthread
)

# Add tests to CTest
add_test(NAME AllTests COMMAND run_tests)
```

## Module Organization

### Core Module
**Purpose**: Low-level packet dan socket handling
**Dependencies**: Linux system headers, libc
**Public API**: Packet structures, raw socket creation, checksums

### TCP Module
**Purpose**: TCP protocol implementation
**Dependencies**: Core module
**Public API**: TCP connection, state machine, segment handling

### TLS Module
**Purpose**: TLS/SSL integration
**Dependencies**: TCP module, OpenSSL
**Public API**: TLS connection, certificate handling

### HTTP Module
**Purpose**: HTTP request/response dan smuggling
**Dependencies**: TLS module (optional), TCP module
**Public API**: Request builder, response parser, smuggling techniques

### Utils Module
**Purpose**: Common utilities
**Dependencies**: None (or minimal)
**Public API**: Buffers, logging, error handling

## Development Workflow

### 1. Setup Environment
```bash
# Clone repository
git clone https://github.com/HazaVVIP/haquests.git
cd haquests

# Initialize submodules
git submodule update --init --recursive

# Build with Docker (recommended for development)
docker-compose up -d haquests-dev
docker-compose exec haquests-dev bash
```

### 2. Build
```bash
# Inside container or with CAP_NET_RAW
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 3. Run Tests
```bash
# Unit tests
./tests/run_tests

# Integration tests (needs root or CAP_NET_RAW)
sudo ./tests/run_integration_tests
```

### 4. Run Examples
```bash
# Simple HTTP GET
sudo ./examples/simple_http_get http://example.com

# HTTPS connection
sudo ./examples/tls_connection https://www.google.com

# Smuggling test
sudo ./examples/smuggling_clte https://target.com
```

## Installation

### System-Wide Installation
```bash
cd build
sudo make install

# Library akan terinstall di:
# /usr/local/lib/libhaquests.so
# /usr/local/include/haquests/
```

### Local Installation
```bash
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make install
```

## Documentation Generation

### Using Doxygen
```bash
# Install doxygen
apt-get install doxygen graphviz

# Generate docs
doxygen Doxyfile

# View docs
firefox docs/html/index.html
```

## Advantages of This Structure

1. **Modular**: Each layer dapat di-develop dan di-test independently
2. **Scalable**: Mudah menambahkan features baru
3. **Testable**: Clear separation antara unit dan integration tests
4. **Standard**: Mengikuti C++ project best practices
5. **Documented**: Clear structure untuk documentation
6. **Containerized**: Easy development dengan Docker
7. **Flexible Build**: CMake support multiple platforms

## Next Steps

1. Setup basic CMake structure
2. Create core module skeleton
3. Implement build system
4. Setup Docker environment
5. Create example skeleton files
