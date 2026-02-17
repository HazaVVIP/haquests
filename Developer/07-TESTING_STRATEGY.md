# Testing Strategy

## Overview

Testing strategy untuk HAQuests library mencakup multiple levels: unit testing, integration testing, system testing, dan security testing. Setiap layer network stack memerlukan pendekatan testing yang berbeda.

## Testing Pyramid

```
                    ╱╲
                   ╱  ╲
                  ╱ E2E╲              <- End-to-End (few)
                 ╱______╲
                ╱        ╲
               ╱Integration╲          <- Integration Tests (some)
              ╱____________╲
             ╱              ╲
            ╱  Unit Tests    ╲       <- Unit Tests (many)
           ╱__________________╲
```

## Unit Testing

### Core Layer Tests

#### Checksum Tests
```cpp
// tests/unit/test_checksum.cpp
#include <gtest/gtest.h>
#include <haquests/core/checksum.hpp>

TEST(ChecksumTest, IPHeaderChecksum) {
    // Known IP header dengan known checksum
    uint8_t ip_header[] = {
        0x45, 0x00, 0x00, 0x3c, // Version, IHL, ToS, Total Length
        0x1c, 0x46, 0x40, 0x00, // ID, Flags, Fragment Offset
        0x40, 0x06, 0x00, 0x00, // TTL, Protocol, Checksum (0 for now)
        0xac, 0x10, 0x0a, 0x63, // Source IP
        0xac, 0x10, 0x0a, 0x0c  // Dest IP
    };
    
    uint16_t expected = 0xb1e6;  // Correct checksum
    uint16_t calculated = haquests::calculateIPChecksum(ip_header, 20);
    
    EXPECT_EQ(calculated, expected);
}

TEST(ChecksumTest, TCPChecksum) {
    // Test TCP checksum dengan pseudo-header
    // ... implementation
}

TEST(ChecksumTest, ChecksumWithOddLength) {
    // Test odd-length buffer
    uint8_t data[] = {0x01, 0x02, 0x03};
    uint16_t checksum = haquests::calculateChecksum(data, 3);
    EXPECT_NE(checksum, 0);
}
```

#### Packet Construction Tests
```cpp
// tests/unit/test_packet.cpp
TEST(PacketTest, IPHeaderConstruction) {
    haquests::IPPacket packet;
    packet.setVersion(4);
    packet.setSourceIP("192.168.1.1");
    packet.setDestIP("8.8.8.8");
    packet.setProtocol(haquests::Protocol::TCP);
    
    auto buffer = packet.serialize();
    
    // Verify header fields
    EXPECT_EQ(buffer[0] >> 4, 4);  // Version
    EXPECT_EQ(buffer[9], 6);       // Protocol (TCP = 6)
}

TEST(PacketTest, TCPHeaderConstruction) {
    haquests::TCPSegment segment;
    segment.setSourcePort(12345);
    segment.setDestPort(80);
    segment.setSeqNum(1000);
    segment.setAckNum(0);
    segment.setFlags(haquests::TCP_SYN);
    
    auto buffer = segment.serialize();
    
    // Verify source port
    uint16_t sport = (buffer[0] << 8) | buffer[1];
    EXPECT_EQ(sport, 12345);
}
```

### TCP Layer Tests

#### State Machine Tests
```cpp
// tests/unit/test_tcp_state.cpp
TEST(TCPStateTest, InitialState) {
    haquests::TCPStateMachine sm;
    EXPECT_EQ(sm.getState(), haquests::TCPState::CLOSED);
}

TEST(TCPStateTest, HandshakeTransitions) {
    haquests::TCPStateMachine sm;
    
    // CLOSED -> SYN_SENT
    sm.sendSYN();
    EXPECT_EQ(sm.getState(), haquests::TCPState::SYN_SENT);
    
    // SYN_SENT -> ESTABLISHED
    sm.receiveSYNACK();
    EXPECT_EQ(sm.getState(), haquests::TCPState::ESTABLISHED);
}

TEST(TCPStateTest, InvalidTransitions) {
    haquests::TCPStateMachine sm;
    
    // Cannot send data when not connected
    EXPECT_THROW(sm.sendData("test"), std::runtime_error);
}
```

#### Sequence Number Tests
```cpp
TEST(SequenceTest, SequenceIncrement) {
    haquests::SequenceTracker tracker(1000);
    
    tracker.onSend(100);  // Send 100 bytes
    EXPECT_EQ(tracker.getNextSeq(), 1100);
    
    tracker.onSend(50);
    EXPECT_EQ(tracker.getNextSeq(), 1150);
}

TEST(SequenceTest, SequenceWrapAround) {
    haquests::SequenceTracker tracker(UINT32_MAX - 10);
    
    tracker.onSend(20);  // Should wrap around
    EXPECT_EQ(tracker.getNextSeq(), 9);  // Wrapped
}

TEST(SequenceTest, AckValidation) {
    haquests::SequenceTracker tracker(1000);
    tracker.onSend(100);
    
    // Valid ACK
    EXPECT_TRUE(tracker.isValidAck(1050));
    EXPECT_TRUE(tracker.isValidAck(1100));
    
    // Invalid ACK (future)
    EXPECT_FALSE(tracker.isValidAck(1200));
}
```

### HTTP Layer Tests

#### Request Builder Tests
```cpp
// tests/unit/test_http_request.cpp
TEST(HTTPRequestTest, BasicGET) {
    haquests::HTTPRequest req("GET", "/index.html");
    req.setHeader("Host", "example.com");
    req.setHeader("User-Agent", "HAQuests/1.0");
    
    std::string request = req.build();
    
    EXPECT_NE(request.find("GET /index.html HTTP/1.1"), std::string::npos);
    EXPECT_NE(request.find("Host: example.com"), std::string::npos);
}

TEST(HTTPRequestTest, POSTWithBody) {
    haquests::HTTPRequest req("POST", "/api/data");
    req.setHeader("Host", "api.example.com");
    req.setBody("key=value");
    
    std::string request = req.build();
    
    EXPECT_NE(request.find("Content-Length: 9"), std::string::npos);
    EXPECT_NE(request.find("key=value"), std::string::npos);
}
```

#### Response Parser Tests
```cpp
TEST(HTTPResponseTest, ParseStatusLine) {
    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<html>test</html>";
    
    auto parsed = haquests::HTTPResponse::parse(response);
    
    EXPECT_EQ(parsed.status_code, 200);
    EXPECT_EQ(parsed.status_message, " OK");
    EXPECT_EQ(parsed.headers["Content-Type"], "text/html");
    EXPECT_EQ(parsed.body, "<html>test</html>");
}

TEST(HTTPResponseTest, ParseChunked) {
    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "5\r\n"
        "hello\r\n"
        "6\r\n"
        "world!\r\n"
        "0\r\n"
        "\r\n";
    
    auto parsed = haquests::HTTPResponse::parse(response);
    EXPECT_TRUE(parsed.isChunked());
    
    auto decoded = parsed.decodeChunked();
    EXPECT_EQ(decoded, "helloworld!");
}
```

#### Chunked Encoding Tests
```cpp
TEST(ChunkedTest, EncodeBasic) {
    std::string data = "Hello, World!";
    std::string chunked = haquests::ChunkedEncoder::encode(data);
    
    EXPECT_NE(chunked.find("d\r\n"), std::string::npos);  // 13 in hex
    EXPECT_NE(chunked.find("Hello, World!"), std::string::npos);
    EXPECT_NE(chunked.find("0\r\n\r\n"), std::string::npos);  // Final chunk
}

TEST(ChunkedTest, VariableChunkSizes) {
    std::string data = "1234567890";
    std::vector<size_t> sizes = {3, 3, 4};  // 3+3+4 = 10
    
    std::string chunked = haquests::ChunkedEncoder::encodeWithVariableChunks(data, sizes);
    
    EXPECT_NE(chunked.find("3\r\n123\r\n"), std::string::npos);
    EXPECT_NE(chunked.find("3\r\n456\r\n"), std::string::npos);
}
```

## Integration Testing

### TCP Connection Tests

```cpp
// tests/integration/test_tcp_handshake.cpp
TEST(TCPIntegrationTest, HandshakeWithLocalhost) {
    // Start test server
    TestServer server(8080);
    server.start();
    
    // Create connection
    haquests::TCPConnection conn("127.0.0.1", 8080);
    
    // Perform handshake
    ASSERT_NO_THROW(conn.connect());
    EXPECT_EQ(conn.getState(), haquests::TCPState::ESTABLISHED);
    
    // Cleanup
    conn.close();
    server.stop();
}

TEST(TCPIntegrationTest, DataTransfer) {
    TestServer server(8080);
    server.start();
    
    haquests::TCPConnection conn("127.0.0.1", 8080);
    conn.connect();
    
    // Send data
    std::string data = "Hello, Server!";
    conn.send(data.c_str(), data.length());
    
    // Server should echo back
    auto response = conn.receive(1024);
    EXPECT_EQ(std::string(response.begin(), response.end()), data);
    
    conn.close();
    server.stop();
}
```

### TLS Connection Tests

```cpp
// tests/integration/test_tls_connection.cpp
TEST(TLSIntegrationTest, ConnectToGoogle) {
    haquests::TLSConnection conn("www.google.com", 443);
    
    ASSERT_NO_THROW(conn.connect());
    
    // Send HTTP request
    std::string request = "GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n";
    conn.send(request.c_str(), request.length());
    
    // Receive response
    auto response = conn.receive(4096);
    std::string resp_str(response.begin(), response.end());
    
    EXPECT_NE(resp_str.find("HTTP/1.1"), std::string::npos);
    EXPECT_NE(resp_str.find("200"), std::string::npos);
}

TEST(TLSIntegrationTest, CertificateVerification) {
    // Valid certificate
    haquests::TLSConnection conn1("www.google.com", 443);
    EXPECT_NO_THROW(conn1.connect());
    
    // Self-signed certificate (should fail)
    haquests::TLSConnection conn2("self-signed.badssl.com", 443);
    EXPECT_THROW(conn2.connect(), std::runtime_error);
}
```

### HTTP Smuggling Tests

```cpp
// tests/integration/test_smuggling.cpp
TEST(SmugglingTest, CLTEDetection) {
    // Setup vulnerable test server
    VulnerableServer server(8080, VulnerabilityType::CLTE);
    server.start();
    
    haquests::HTTPSmugglingRequest req;
    std::string smuggled = "GET /admin HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::string payload = req.buildCLTE(smuggled);
    
    // Send smuggling request
    haquests::TLSConnection conn("localhost", 8080);
    conn.send(payload.c_str(), payload.length());
    
    // Send normal request
    std::string normal = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    conn.send(normal.c_str(), normal.length());
    
    // If vulnerable, normal request should get /admin response
    auto response = conn.receive(4096);
    std::string resp_str(response.begin(), response.end());
    
    // Check if we got admin page
    bool is_vulnerable = resp_str.find("/admin") != std::string::npos;
    EXPECT_TRUE(is_vulnerable);  // Server is intentionally vulnerable
    
    server.stop();
}
```

## System Testing

### End-to-End Tests

```cpp
// tests/e2e/test_http_client.cpp
TEST(E2ETest, RealWorldHTTP) {
    // Test against real HTTP server
    haquests::HTTPClient client;
    
    auto response = client.get("http://example.com/");
    
    EXPECT_EQ(response.status_code, 200);
    EXPECT_NE(response.body.find("<html>"), std::string::npos);
}

TEST(E2ETest, RealWorldHTTPS) {
    haquests::HTTPClient client;
    
    auto response = client.get("https://www.google.com/");
    
    EXPECT_EQ(response.status_code, 200);
    EXPECT_GT(response.body.length(), 0);
}

TEST(E2ETest, HTTPSWithSessionResumption) {
    haquests::HTTPClient client;
    
    // First request
    auto start1 = std::chrono::steady_clock::now();
    auto response1 = client.get("https://www.google.com/");
    auto end1 = std::chrono::steady_clock::now();
    auto time1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    
    // Second request (should use session resumption)
    auto start2 = std::chrono::steady_clock::now();
    auto response2 = client.get("https://www.google.com/");
    auto end2 = std::chrono::steady_clock::now();
    auto time2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
    
    // Second request should be faster (session resumption)
    EXPECT_LT(time2.count(), time1.count());
}
```

## Performance Testing

### Benchmark Suite

```cpp
// tests/benchmarks/benchmark_tcp.cpp
#include <benchmark/benchmark.h>

static void BM_TCPHandshake(benchmark::State& state) {
    TestServer server(8080);
    server.start();
    
    for (auto _ : state) {
        haquests::TCPConnection conn("127.0.0.1", 8080);
        conn.connect();
        conn.close();
    }
    
    server.stop();
}
BENCHMARK(BM_TCPHandshake);

static void BM_HTTPRequest(benchmark::State& state) {
    for (auto _ : state) {
        haquests::HTTPClient client;
        auto response = client.get("http://localhost:8080/");
    }
}
BENCHMARK(BM_HTTPRequest);
```

### Memory Leak Testing

```bash
# Run dengan valgrind
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         ./build/tests/run_tests

# Expected output: no leaks
# "All heap blocks were freed -- no leaks are possible"
```

## Fuzz Testing

### AFL Fuzzing

```cpp
// tests/fuzz/fuzz_http_parser.cpp
#include <haquests/http/response.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string input(reinterpret_cast<const char*>(data), size);
    
    try {
        auto response = haquests::HTTPResponse::parse(input);
        // If parsing succeeds, that's fine
    } catch (...) {
        // Exceptions are fine, just don't crash
    }
    
    return 0;
}
```

```bash
# Compile dengan AFL
afl-clang++ -o fuzz_http_parser fuzz_http_parser.cpp -lhaquests

# Run fuzzer
afl-fuzz -i testcases/ -o findings/ ./fuzz_http_parser
```

## Security Testing

### Static Analysis

```bash
# cppcheck
cppcheck --enable=all \
         --suppress=missingInclude \
         --error-exitcode=1 \
         src/ include/

# clang-tidy
clang-tidy src/*.cpp -- -Iinclude/
```

### Dynamic Analysis

```bash
# AddressSanitizer
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make
./run_tests

# ThreadSanitizer (untuk concurrent testing)
cmake -DCMAKE_CXX_FLAGS="-fsanitize=thread -g" ..
make
./run_tests
```

## Test Fixtures

### Packet Fixtures

```cpp
// tests/fixtures/packets.hpp
namespace TestPackets {
    const std::vector<uint8_t> VALID_SYN = {
        // IP header
        0x45, 0x00, 0x00, 0x3c, /* ... */
        // TCP header dengan SYN flag
        /* ... */
    };
    
    const std::vector<uint8_t> VALID_SYN_ACK = {
        /* ... */
    };
}
```

### HTTP Response Fixtures

```cpp
// tests/fixtures/http_responses.hpp
namespace TestResponses {
    const std::string OK_200 = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";
    
    const std::string CHUNKED_RESPONSE =
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "5\r\nhello\r\n"
        "0\r\n\r\n";
}
```

## Continuous Integration

### GitHub Actions Workflow

```yaml
# .github/workflows/ci.yml
name: CI

on: [push, pull_request]

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
      with:
        submodules: recursive
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y libssl-dev libpcap-dev
    
    - name: Build
      run: |
        mkdir build && cd build
        cmake ..
        make -j$(nproc)
    
    - name: Run unit tests
      run: |
        cd build
        ./tests/run_tests --gtest_output=xml:test_results.xml
    
    - name: Run integration tests (with CAP_NET_RAW)
      run: |
        # Integration tests yang butuh privileges di-skip di CI
        # Atau run dalam Docker container dengan capabilities
        docker-compose up -d
        docker-compose exec -T haquests-dev ./build/tests/run_integration_tests
```

## Test Coverage

### Coverage Report

```bash
# Build dengan coverage flags
cmake -DCMAKE_CXX_FLAGS="--coverage" ..
make

# Run tests
./tests/run_tests

# Generate coverage report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --remove coverage.info 'tests/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_html

# View report
firefox coverage_html/index.html
```

### Coverage Goals
- **Unit tests**: >90% line coverage
- **Integration tests**: >70% line coverage
- **Overall**: >80% line coverage

## Test Organization

```
tests/
├── unit/              # Fast, isolated tests
│   ├── core/
│   ├── tcp/
│   ├── tls/
│   └── http/
│
├── integration/       # Tests dengan real network
│   ├── tcp/
│   ├── tls/
│   └── http/
│
├── e2e/              # End-to-end scenarios
│   └── full_workflow/
│
├── benchmarks/       # Performance tests
│   ├── tcp_bench.cpp
│   └── http_bench.cpp
│
├── fuzz/             # Fuzzing tests
│   ├── http_parser_fuzz.cpp
│   └── tcp_parser_fuzz.cpp
│
└── fixtures/         # Test data
    ├── packets/
    ├── responses/
    └── certificates/
```

## Test Execution Strategy

### Local Development
```bash
# Quick unit tests
make test-unit

# Full test suite
make test-all

# Specific test
./build/tests/run_tests --gtest_filter=TCPState*
```

### Pre-commit
```bash
# Run before every commit
./scripts/pre-commit.sh

# Which includes:
# - Unit tests
# - Static analysis
# - Code formatting check
```

### CI/CD
```bash
# On every push:
# - Build
# - Unit tests
# - Integration tests (dalam container)
# - Coverage report

# On release:
# - Full test suite
# - Performance benchmarks
# - Security scan
# - Generate artifacts
```

## Next Steps

1. ✅ Setup GoogleTest framework
2. Create test fixtures dan helpers
3. Implement unit tests for each module
4. Setup integration test infrastructure
5. Configure CI/CD pipeline
6. Establish coverage goals
