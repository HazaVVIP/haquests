# Implementation Roadmap

## Project Phases

Roadmap ini dibagi menjadi phases yang dapat diimplementasikan secara incremental. Setiap phase menghasilkan working prototype yang dapat di-test.

## Phase 0: Foundation & Setup (Week 1-2)

### Goals
- Setup development environment
- Create basic project structure
- Setup build system dan testing framework

### Tasks

#### 1. Repository Setup
- [x] Create repository structure
- [ ] Setup .gitignore
- [ ] Create README.md dengan basic instructions
- [ ] Add LICENSE file
- [ ] Setup git hooks untuk code quality

#### 2. Build System
- [ ] Create root CMakeLists.txt
- [ ] Setup module CMakeLists.txt
- [ ] Configure compiler flags dan warnings
- [ ] Setup dependency management (OpenSSL, libpcap)

#### 3. Docker Environment
- [ ] Create Dockerfile dengan all dependencies
- [ ] Create docker-compose.yml untuk dev environment
- [ ] Add test server container (nginx)
- [ ] Document Docker usage

#### 4. Testing Framework
- [ ] Add GoogleTest as submodule
- [ ] Create test runner
- [ ] Setup basic test structure
- [ ] Configure CTest integration

### Deliverables
- ✅ Working build system
- ✅ Docker environment yang bisa compile code
- ✅ Basic test infrastructure
- ✅ Documentation skeleton

### Testing Criteria
```bash
# Should successfully build
docker-compose up -d
docker-compose exec haquests-dev bash
mkdir build && cd build
cmake ..
make
```

---

## Phase 1: Core Layer - Raw Packets (Week 3-4)

### Goals
- Implement raw socket creation
- Implement packet structures (IP, TCP headers)
- Implement checksum calculations
- Send first raw packet

### Tasks

#### 1. Packet Structures
- [ ] Define IP header struct (`include/haquests/core/packet.hpp`)
- [ ] Define TCP header struct
- [ ] Define TCP options structures
- [ ] Create packet builder class

#### 2. Checksum Implementation
- [ ] Implement IP header checksum (`src/core/checksum.cpp`)
- [ ] Implement TCP checksum with pseudo-header
- [ ] Add checksum validation functions
- [ ] Unit tests untuk checksum

#### 3. Raw Socket Wrapper
- [ ] Create RawSocket class (`include/haquests/core/socket.hpp`)
- [ ] Implement socket creation dengan error handling
- [ ] Implement IP_HDRINCL option setting
- [ ] Add privilege checking

#### 4. Basic Packet Sending
- [ ] Implement packet serialization
- [ ] Implement sendto wrapper
- [ ] Add destination address handling
- [ ] Create example: send single TCP packet

### Code Structure
```
include/haquests/core/
  ├── packet.hpp      # IP, TCP, UDP structures
  ├── socket.hpp      # Raw socket wrapper
  ├── checksum.hpp    # Checksum functions
  └── types.hpp       # Common types

src/core/
  ├── packet.cpp
  ├── socket.cpp
  └── checksum.cpp

tests/unit/
  ├── test_checksum.cpp
  └── test_packet.cpp
```

### Deliverables
- ✅ Can create raw socket
- ✅ Can construct IP + TCP packet
- ✅ Can calculate checksums correctly
- ✅ Can send packet to destination

### Testing Criteria
```bash
# Run unit tests
./build/tests/run_tests --gtest_filter=Checksum*

# Run example - send single SYN packet
sudo ./build/examples/send_syn_packet 8.8.8.8 80

# Verify dengan tcpdump
sudo tcpdump -i any -n 'tcp and host 8.8.8.8'
```

---

## Phase 2: TCP Layer - State Machine (Week 5-7)

### Goals
- Implement basic TCP state machine
- Implement 3-way handshake (client side)
- Establish first TCP connection
- Send/receive data over TCP

### Tasks

#### 1. TCP State Machine
- [ ] Define TCP states enum (`include/haquests/tcp/state_machine.hpp`)
- [ ] Implement state transitions
- [ ] Add state validation
- [ ] Unit tests untuk state transitions

#### 2. Sequence Number Management
- [ ] Implement ISN generation
- [ ] Track send sequence numbers (SND.NXT, SND.UNA)
- [ ] Track receive sequence numbers (RCV.NXT)
- [ ] Handle sequence number wrap-around

#### 3. TCP Connection Class
- [ ] Create TCPConnection class (`include/haquests/tcp/connection.hpp`)
- [ ] Implement connect() - 3-way handshake
- [ ] Implement send() - data transmission
- [ ] Implement receive() - data reception
- [ ] Implement close() - connection termination

#### 4. Packet Reception
- [ ] Setup libpcap for packet capture
- [ ] Implement BPF filter untuk relevant packets
- [ ] Create packet processing loop
- [ ] Handle received ACKs

#### 5. Retransmission Timer (Basic)
- [ ] Implement simple timeout timer
- [ ] Add retransmission logic
- [ ] Exponential backoff
- [ ] Unit tests untuk timer

### Code Structure
```
include/haquests/tcp/
  ├── connection.hpp
  ├── state_machine.hpp
  ├── segment.hpp
  └── window.hpp

src/tcp/
  ├── connection.cpp
  ├── state_machine.cpp
  ├── segment.cpp
  └── window.cpp

tests/integration/
  └── test_tcp_handshake.cpp
```

### Deliverables
- ✅ Successful 3-way handshake dengan real server
- ✅ Can send HTTP request over raw TCP
- ✅ Can receive HTTP response
- ✅ Clean connection termination

### Testing Criteria
```bash
# Integration test - connect to localhost server
# Start test server
docker-compose up -d test-server

# Run TCP connection test
sudo ./build/tests/test_tcp_handshake

# Run example - HTTP GET via raw TCP
sudo ./build/examples/tcp_http_get http://localhost:8080/

# Verify dengan wireshark
sudo tcpdump -i lo -w tcp_test.pcap 'tcp port 8080'
wireshark tcp_test.pcap
```

---

## Phase 3: HTTP Layer (Week 8-9)

### Goals
- Implement HTTP request builder
- Implement HTTP response parser
- Complete first HTTP GET request
- Support chunked transfer encoding

### Tasks

#### 1. HTTP Request Builder
- [ ] Create HTTPRequest class (`include/haquests/http/request.hpp`)
- [ ] Implement method, path, version
- [ ] Implement headers management
- [ ] Implement body handling
- [ ] Unit tests untuk request building

#### 2. HTTP Response Parser
- [ ] Create HTTPResponse class (`include/haquests/http/response.hpp`)
- [ ] Parse status line
- [ ] Parse headers
- [ ] Parse body
- [ ] Unit tests dengan sample responses

#### 3. Chunked Encoding
- [ ] Implement chunked encoder (`include/haquests/http/chunked.hpp`)
- [ ] Implement chunked decoder
- [ ] Handle chunk extensions
- [ ] Unit tests

#### 4. Complete HTTP Client
- [ ] Combine TCP + HTTP layers
- [ ] Implement GET, POST, PUT, DELETE
- [ ] Handle redirects (optional)
- [ ] Handle different content types

### Code Structure
```
include/haquests/http/
  ├── request.hpp
  ├── response.hpp
  ├── chunked.hpp
  └── headers.hpp

src/http/
  ├── request.cpp
  ├── response.cpp
  ├── chunked.cpp
  └── headers.cpp
```

### Deliverables
- ✅ Working HTTP GET request
- ✅ Working HTTP POST request
- ✅ Parse HTTP responses correctly
- ✅ Handle chunked responses

### Testing Criteria
```bash
# Unit tests
./build/tests/run_tests --gtest_filter=HTTP*

# Integration test
sudo ./build/examples/simple_http_get http://example.com/

# POST test
sudo ./build/examples/http_post http://httpbin.org/post
```

---

## Phase 4: TLS Layer (Week 10-12)

### Goals
- Integrate OpenSSL dengan raw TCP
- Implement custom BIO
- Complete first HTTPS request
- Certificate verification

### Tasks

#### 1. Custom BIO Implementation
- [ ] Create BIO method structure (`include/haquests/tls/bio_adapter.hpp`)
- [ ] Implement BIO read callback
- [ ] Implement BIO write callback
- [ ] Implement BIO control callback

#### 2. TLS Connection
- [ ] Create TLSConnection class (`include/haquests/tls/connection.hpp`)
- [ ] Initialize SSL context
- [ ] Perform TLS handshake
- [ ] Implement send/receive over TLS

#### 3. Certificate Handling
- [ ] Implement certificate verification (`include/haquests/tls/certificate.hpp`)
- [ ] Add custom verification callback
- [ ] Support certificate pinning
- [ ] Handle verification errors

#### 4. Session Management
- [ ] Implement session caching (`include/haquests/tls/session.hpp`)
- [ ] Session resumption
- [ ] Session tickets

#### 5. ALPN Support
- [ ] Implement ALPN negotiation
- [ ] Support HTTP/1.1 dan HTTP/2
- [ ] Check negotiated protocol

### Code Structure
```
include/haquests/tls/
  ├── connection.hpp
  ├── bio_adapter.hpp
  ├── certificate.hpp
  └── session.hpp

src/tls/
  ├── connection.cpp
  ├── bio_adapter.cpp
  ├── certificate.cpp
  └── session.cpp
```

### Deliverables
- ✅ Successful HTTPS connection
- ✅ Working certificate verification
- ✅ TLS 1.2 dan 1.3 support
- ✅ Session resumption working

### Testing Criteria
```bash
# Test TLS connection
sudo ./build/examples/tls_connection https://www.google.com/

# Test dengan different servers
sudo ./build/examples/tls_connection https://github.com/
sudo ./build/examples/tls_connection https://www.cloudflare.com/

# Verify TLS version
sudo ./build/tools/tls_inspector https://www.google.com/

# Integration tests
./build/tests/test_tls_connection
```

---

## Phase 5: HTTP Smuggling (Week 13-15)

### Goals
- Implement request smuggling techniques
- Support CL.TE, TE.CL, TE.TE
- Create smuggling test tool
- Document smuggling patterns

### Tasks

#### 1. Smuggling Request Builder
- [ ] Create HTTPSmugglingRequest class (`include/haquests/http/smuggling.hpp`)
- [ ] Implement CL.TE builder
- [ ] Implement TE.CL builder
- [ ] Implement TE.TE builder

#### 2. Header Manipulation
- [ ] Support duplicate headers
- [ ] Support malformed headers
- [ ] Support header obfuscation
- [ ] Custom line endings

#### 3. Advanced Chunked Encoding
- [ ] Variable chunk sizes
- [ ] Malformed chunk sizes
- [ ] Missing CRLFs
- [ ] Extra whitespace

#### 4. Detection Framework
- [ ] Timing-based detection
- [ ] Response comparison
- [ ] Differential testing

#### 5. Smuggling Test Tool
- [ ] Create CLI tool
- [ ] Automated testing
- [ ] Report generation
- [ ] Example payloads

### Code Structure
```
include/haquests/http/
  └── smuggling.hpp

src/http/
  └── smuggling.cpp

tools/smuggling_tester/
  ├── main.cpp
  ├── detector.hpp
  ├── detector.cpp
  └── CMakeLists.txt

examples/
  ├── smuggling_clte.cpp
  ├── smuggling_tecl.cpp
  └── smuggling_tete.cpp
```

### Deliverables
- ✅ Working CL.TE smuggling
- ✅ Working TE.CL smuggling
- ✅ Working TE.TE smuggling
- ✅ Smuggling detection tool

### Testing Criteria
```bash
# Manual testing dengan vulnerable server
sudo ./build/examples/smuggling_clte https://vulnerable-server.com/

# Automated testing
sudo ./build/tools/smuggling_tester --target https://target.com/ --all

# Test payloads
sudo ./build/tools/smuggling_tester --payload payloads/clte_basic.txt
```

---

## Phase 6: Advanced Features (Week 16-18)

### Goals
- WAF bypass techniques
- Performance optimization
- HTTP/2 support (optional)
- Comprehensive documentation

### Tasks

#### 1. WAF Bypass Techniques
- [ ] Request splitting
- [ ] TCP segmentation control
- [ ] Timing manipulation
- [ ] Header case tampering
- [ ] Unicode/encoding tricks

#### 2. Performance Optimization
- [ ] Zero-copy operations
- [ ] Connection pooling
- [ ] Buffer optimization
- [ ] Reduce memory allocations

#### 3. HTTP/2 Support (Optional)
- [ ] HTTP/2 frame parsing
- [ ] HPACK header compression
- [ ] HTTP/2 smuggling

#### 4. Benchmarking
- [ ] Create benchmark suite
- [ ] Compare dengan standard libraries
- [ ] Profile memory usage
- [ ] Profile CPU usage

#### 5. Documentation
- [ ] API documentation (Doxygen)
- [ ] User guides
- [ ] Smuggling guide
- [ ] Troubleshooting guide
- [ ] Security considerations

### Deliverables
- ✅ Optimized library
- ✅ Complete documentation
- ✅ Benchmark results
- ✅ Production-ready codebase

---

## Phase 7: Testing & Hardening (Week 19-20)

### Goals
- Comprehensive test coverage
- Security audit
- Stability testing
- Release preparation

### Tasks

#### 1. Test Coverage
- [ ] Achieve >80% code coverage
- [ ] Edge case testing
- [ ] Fuzz testing
- [ ] Stress testing

#### 2. Security Review
- [ ] Code review untuk security issues
- [ ] Memory leak detection (Valgrind)
- [ ] Buffer overflow checks
- [ ] Static analysis (cppcheck)

#### 3. Stability Testing
- [ ] Long-running tests
- [ ] Connection stability
- [ ] Error recovery
- [ ] Resource leak detection

#### 4. Release Preparation
- [ ] Versioning scheme
- [ ] Changelog
- [ ] Release notes
- [ ] Installation guide

---

## Milestones

### M1: Core Complete (End of Phase 2)
- Can send/receive raw TCP packets
- 3-way handshake working
- Basic data transfer

### M2: HTTP Complete (End of Phase 3)
- Working HTTP client
- Request/response handling
- Chunked encoding support

### M3: HTTPS Complete (End of Phase 4)
- TLS integration working
- Can make HTTPS requests
- Certificate verification

### M4: Smuggling Complete (End of Phase 5)
- All smuggling techniques implemented
- Detection framework working
- Tool for automated testing

### M5: Production Ready (End of Phase 7)
- Complete documentation
- Comprehensive tests
- Security reviewed
- Ready for release

---

## Risk Mitigation

### Technical Risks

1. **TCP State Machine Complexity**
   - Risk: Implementation bugs causing connection failures
   - Mitigation: Extensive testing, reference implementation study, incremental development

2. **TLS Integration Difficulty**
   - Risk: OpenSSL BIO integration issues
   - Mitigation: Early prototyping, consult OpenSSL documentation, seek expert advice

3. **Performance Issues**
   - Risk: Library too slow untuk practical use
   - Mitigation: Profile early, optimize hot paths, benchmark against alternatives

### Resource Risks

1. **Time Constraints**
   - Risk: Phases take longer than estimated
   - Mitigation: Prioritize core features, defer optional features, parallel development

2. **Dependencies**
   - Risk: OpenSSL or libpcap API changes
   - Mitigation: Pin dependency versions, monitor upstream changes, abstract dependencies

---

## Success Metrics

### Functional Metrics
- [ ] Can establish TCP connection to any server
- [ ] Can make HTTP requests successfully
- [ ] Can make HTTPS requests successfully
- [ ] Can perform request smuggling attacks
- [ ] >90% test pass rate

### Performance Metrics
- [ ] Connection setup < 100ms
- [ ] Throughput > 50% of libcurl
- [ ] Memory usage < 10MB per connection
- [ ] CPU usage reasonable

### Quality Metrics
- [ ] Code coverage > 80%
- [ ] Zero critical security issues
- [ ] Zero memory leaks
- [ ] Documentation complete

---

## Next Steps

Immediate actions:
1. ✅ Complete Phase 0 setup
2. Begin Phase 1 - Core layer implementation
3. Setup continuous testing
4. Document progress
