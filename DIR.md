# Struktur Direktori HAQuests

Dokumen ini menjelaskan struktur direktori lengkap dari HAQuests library beserta deskripsi setiap file.

## 📁 Struktur Direktori

```
haquests/
├── include/haquests/          # Public header files
│   ├── core/                  # Core layer - raw socket & packets
│   ├── tcp/                   # TCP layer - connection management
│   ├── tls/                   # TLS layer - secure connections
│   ├── http/                  # HTTP layer - request/response
│   ├── utils/                 # Utility layer - helpers
│   └── haquests.hpp          # Master include file
├── src/                      # Implementation files
│   ├── core/                 # Core layer implementations
│   ├── tcp/                  # TCP layer implementations
│   ├── tls/                  # TLS layer implementations
│   ├── http/                 # HTTP layer implementations
│   └── utils/                # Utility implementations
├── examples/                 # Example programs
├── tests/                    # Unit and integration tests
├── scripts/                  # Build and utility scripts
├── tools/                    # Additional tools
├── CMakeLists.txt           # Main CMake configuration
├── Dockerfile               # Docker image definition
├── docker-compose.yml       # Docker compose configuration
├── LICENSE                  # MIT License
├── README.md                # Main documentation
└── DIR.md                   # This file
```

---

## 📚 Detail File-per-File

### 🔹 CORE LAYER (`include/haquests/core/` & `src/core/`)

Layer paling dasar yang menangani raw socket operations dan packet manipulation.

#### **types.hpp**
**Tujuan**: Mendefinisikan konstanta dan enumerasi untuk protocol networking  
**Isi**:
- TCP flags (FIN, SYN, RST, PSH, ACK, URG)
- TCP states (CLOSED, LISTEN, SYN_SENT, ESTABLISHED, CLOSE_WAIT, dll)
- Default values (MTU=1500, TTL=64, MAX_PACKET_SIZE=65535)

#### **packet.hpp / packet.cpp**
**Tujuan**: Struktur data dan builder untuk network packets  
**Isi**:
- `IPHeader`: Struktur IP header (version, IHL, TOS, length, TTL, protocol, checksum, src/dst IP)
- `TCPHeader`: Struktur TCP header (src/dst port, seq/ack numbers, flags, window, checksum)
- `PseudoHeader`: Untuk perhitungan TCP checksum
- `Packet` class: Menggabungkan IP + TCP + data payload
- Methods: `buildIPHeader()`, `buildTCPHeader()`, `setData()`, `serialize()`, `getTotalSize()`

#### **checksum.hpp / checksum.cpp**
**Tujuan**: Perhitungan Internet checksum untuk IP dan TCP  
**Isi**:
- `checksum()`: Fungsi umum untuk menghitung checksum dari buffer
- `tcpChecksum()`: Menghitung TCP checksum dengan pseudo-header support
- `verifyChecksum()`: Memverifikasi checksum packet yang diterima
- Implementasi menggunakan algoritma one's complement standard

#### **socket.hpp / socket.cpp**
**Tujuan**: Wrapper untuk raw socket operations  
**Isi**:
- `RawSocket` class: Abstraksi untuk SOCK_RAW socket
- Methods: `open()`, `close()`, `send()`, `receive()`
- Capability checking untuk CAP_NET_RAW
- Socket option handling (IP_HDRINCL)
- File descriptor management

---

### 🔹 TCP LAYER (`include/haquests/tcp/` & `src/tcp/`)

Implementasi lengkap TCP protocol dengan state machine sesuai RFC 793.

#### **connection.hpp / connection.cpp**
**Tujuan**: High-level TCP connection management  
**Isi**:
- `Connection` class: Abstraksi lengkap untuk TCP connection
- Methods: `connect()`, `send()`, `receive()`, `close()`
- Pimpl pattern untuk menyembunyikan implementation details
- Integration dengan raw socket dari core layer
- Sequence number management
- SYN/ACK/FIN packet handling
- Receive callbacks untuk asynchronous I/O

#### **state_machine.hpp / state_machine.cpp**
**Tujuan**: TCP state machine implementation (RFC 793)  
**Isi**:
- `StateMachine` class: Tracking dan validasi TCP connection states
- State transition methods:
  - `onSendSYN()`: CLOSED → SYN_SENT
  - `onReceiveSYNACK()`: SYN_SENT → ESTABLISHED
  - `onSendFIN()`: ESTABLISHED → FIN_WAIT_1
  - `onReceiveFIN()`: Berbagai states → CLOSE_WAIT
  - dll sesuai RFC 793
- Validasi legal state transitions
- Error handling untuk invalid transitions

#### **segment.hpp / segment.cpp**
**Tujuan**: Representasi TCP segment  
**Isi**:
- `Segment` class: Wrapper untuk TCP segment data
- Properties: source/destination ports, sequence/ack numbers, flags, window size, data payload
- Methods: `getData()`, `getLength()`, flag checkers (`isSYN()`, `isACK()`, `isFIN()`)
- Data management dan serialization

#### **window.hpp / window.cpp**
**Tujuan**: TCP flow control dengan sliding window  
**Isi**:
- `Window` class: Management untuk send/receive windows
- Properties: window size, used space, available space
- Methods: 
  - `update()`: Update window berdasarkan received data
  - `canSend()`: Cek apakah ada space untuk mengirim data
  - `getAvailableSpace()`: Hitung space yang tersedia
- Flow control logic untuk mencegah buffer overflow

---

### 🔹 TLS LAYER (`include/haquests/tls/` & `src/tls/`)

OpenSSL wrapper untuk secure communication di atas TCP.

#### **connection.hpp / connection.cpp**
**Tujuan**: TLS/SSL secure connection wrapper  
**Isi**:
- `Connection` class: Wrapper untuk TLS connection over TCP
- Methods: `connect()`, `send()`, `receive()`, `close()`
- TLS handshake automation
- Certificate verification control
- Cipher suite queries
- TLS version negotiation
- OpenSSL context management
- Session reuse support

#### **certificate.hpp / certificate.cpp**
**Tujuan**: X.509 certificate handling  
**Isi**:
- `Certificate` class: Wrapper untuk OpenSSL X509 objects
- Methods:
  - `loadFromFile()`, `loadFromMemory()`: Load certificate
  - `getSubject()`, `getIssuer()`: Extract certificate info
  - `isExpired()`: Check validity period
  - `getFingerprint()`: Calculate SHA-256 fingerprint
  - `getSANs()`: Extract Subject Alternative Names
- Certificate validation dan verification

#### **session.hpp / session.cpp**
**Tujuan**: TLS session persistence untuk connection reuse  
**Isi**:
- `Session` class: Wrapper untuk OpenSSL SSL_SESSION
- Methods:
  - `save()`, `load()`: Serialize/deserialize session
  - `saveToFile()`, `loadFromFile()`: File-based persistence
  - `getSessionId()`: Get unique session identifier
- Session ticket support
- Session resumption logic

#### **bio_adapter.hpp / bio_adapter.cpp**
**Tujuan**: Custom BIO adapter untuk bridging TCP dengan OpenSSL  
**Isi**:
- `BIOAdapter` class: Custom BIO_METHOD implementation
- Custom callbacks:
  - `bioRead()`: Read dari underlying TCP connection
  - `bioWrite()`: Write ke underlying TCP connection
  - `bioCtrl()`: Control operations
- Integration antara `TCP::Connection` dengan SSL library
- Penting untuk menggunakan raw socket dengan OpenSSL

---

### 🔹 HTTP LAYER (`include/haquests/http/` & `src/http/`)

HTTP protocol implementation dengan fokus pada security testing.

#### **request.hpp / request.cpp**
**Tujuan**: HTTP request builder  
**Isi**:
- `Request` class: Builder untuk HTTP requests
- Support methods: GET, POST, PUT, DELETE, HEAD, OPTIONS
- Methods:
  - `setMethod()`, `setPath()`, `setVersion()`
  - `addHeader()`, `removeHeader()`, `getHeaders()`
  - `setBody()`, `getBody()`
  - `build()`: Generate complete raw HTTP request
- Automatic Content-Length calculation
- Proper CRLF line endings
- Header management

#### **response.hpp / response.cpp**
**Tujuan**: HTTP response parser  
**Isi**:
- `Response` class: Parser untuk HTTP responses
- Methods:
  - `parse()`: Parse raw HTTP response
  - `getStatusCode()`, `getReasonPhrase()`
  - `getHeaders()`, `getBody()`
  - `isComplete()`: Check if response is complete
  - `isChunked()`: Detect chunked transfer encoding
- Status line parsing
- Header extraction
- Body parsing dengan support untuk incomplete responses

#### **headers.hpp / headers.cpp**
**Tujuan**: HTTP header management  
**Isi**:
- `Headers` class: Container untuk HTTP headers
- Support untuk duplicate headers (menggunakan multimap)
- Methods:
  - `add()`, `remove()`, `get()`, `getAll()`
  - `has()`: Check header existence
  - `parse()`: Parse dari raw header text
  - `build()`: Generate raw header string
- Case-insensitive key lookup
- Multi-value header support (e.g., Set-Cookie)

#### **chunked.hpp / chunked.cpp**
**Tujuan**: Chunked transfer encoding support  
**Isi**:
- `Chunked` class: Encoder/decoder untuk chunked format
- Static methods:
  - `encode()`: Convert data ke chunked format
  - `decode()`: Parse chunked data ke raw data
- Chunk size parsing (hexadecimal)
- Support untuk chunk extensions
- Trailer headers handling

#### **smuggling.hpp / smuggling.cpp**
**Tujuan**: HTTP request smuggling techniques  
**Isi**:
- `Smuggling` class: Builder untuk smuggling attacks
- Smuggling types:
  - `CL.TE`: Content-Length di front-end, Transfer-Encoding di back-end
  - `TE.CL`: Transfer-Encoding di front-end, Content-Length di back-end
  - `TE.TE`: Obfuscated Transfer-Encoding headers
- Methods:
  - `setType()`: Set smuggling technique
  - `build()`: Generate malformed request
  - `detectVulnerability()`: Test for vulnerability
- Creates ambiguous requests yang di-parse berbeda oleh front-end vs back-end

---

### 🔹 UTILITY LAYER (`include/haquests/utils/` & `src/utils/`)

Helper classes dan utilities yang digunakan di seluruh library.

#### **buffer.hpp / buffer.cpp**
**Tujuan**: Efficient byte buffer dengan position tracking  
**Isi**:
- `Buffer` class: Dynamic byte buffer
- Read/write position tracking
- Typed operations:
  - `readU8()`, `readU16()`, `readU32()`: Read integers
  - `writeU8()`, `writeU16()`, `writeU32()`: Write integers
  - `peek()`: Read tanpa menggerakkan position
- Methods:
  - `compact()`: Remove already-read data
  - `clear()`, `reset()`: Buffer management
  - `available()`: Check available bytes
- Automatic sizing dan reallocation

#### **logger.hpp / logger.cpp**
**Tujuan**: Centralized logging system  
**Isi**:
- `Logger` class: Singleton logger
- 5 log levels: DEBUG, INFO, WARNING, ERROR, FATAL
- Output destinations: console dan/atau file
- Methods:
  - `setLogLevel()`: Set minimum log level
  - `setLogFile()`: Enable file logging
  - `log()`: Main logging function
- Macro helpers: `LOG_DEBUG()`, `LOG_INFO()`, `LOG_WARNING()`, `LOG_ERROR()`, `LOG_FATAL()`
- Thread-safe logging
- Timestamp formatting
- Source file/line number tracking

#### **timer.hpp / timer.cpp**
**Tujuan**: Timing utilities untuk profiling dan timeouts  
**Isi**:
- `Timer` class: High-precision timer
- Using `std::chrono::steady_clock` untuk accuracy
- Methods:
  - `start()`, `stop()`, `reset()`
  - `elapsed()`: Get elapsed time in milliseconds
  - `hasExpired()`: Check timeout
- `ScopeTimer` class: RAII-style timer untuk profiling code blocks
- Callback support untuk timeout events

#### **error.hpp / error.cpp**
**Tujuan**: Custom exception hierarchy  
**Isi**:
- `Error` base class: Inherits dari `std::runtime_error`
- Specialized exceptions:
  - `SocketError`: Raw socket errors
  - `ConnectionError`: TCP connection errors
  - `TLSError`: TLS/SSL errors
  - `HTTPError`: HTTP protocol errors
  - `ParseError`: Parsing errors
- Error codes dan kategori
- Methods: `what()`, `getErrorCode()`, `getCategory()`

---

### 🔹 MAIN HEADER

#### **haquests.hpp**
**Tujuan**: Master include file - single entry point untuk library  
**Isi**:
- Includes semua public headers dari semua layers:
  - Core: types, packet, checksum, socket
  - TCP: connection, state_machine, segment, window
  - TLS: connection, certificate, session, bio_adapter
  - HTTP: request, response, headers, chunked, smuggling
  - Utils: buffer, logger, timer, error
- Convenience untuk user - cukup `#include <haquests/haquests.hpp>`

---

### 🔹 EXAMPLES (`examples/`)

Contoh program demonstrating library usage.

#### **simple_http_get.cpp**
**Tujuan**: Basic HTTP GET request over raw TCP  
**Demonstrates**:
- Capability checking (CAP_NET_RAW)
- Creating raw TCP connection
- Building HTTP request
- Sending/receiving raw packets
- Parsing HTTP response

#### **smuggling_clte.cpp**
**Tujuan**: HTTP request smuggling demonstration (CL.TE technique)  
**Demonstrates**:
- Creating CL.TE smuggling payload
- Exploiting Content-Length vs Transfer-Encoding ambiguity
- Educational example untuk understanding smuggling attacks
- **WARNING**: Hanya untuk authorized testing!

#### **tls_connection.cpp**
**Tujuan**: HTTPS request dengan TLS/SSL  
**Demonstrates**:
- TLS handshake
- Certificate verification
- Cipher suite negotiation
- Secure HTTPS communication
- Error handling untuk TLS errors

---

### 🔹 TESTS (`tests/`)

Unit dan integration tests menggunakan Google Test framework.

#### **test_checksum.cpp**
**Tujuan**: Unit tests untuk checksum calculations  
**Tests**:
- Basic checksum calculation
- Edge cases (zero data, odd-length buffers)
- TCP checksum dengan pseudo-header
- Checksum verification
- Validates correctness menggunakan known test vectors

#### **test_http_parser.cpp**
**Tujuan**: Unit tests untuk HTTP request/response parsing  
**Tests**:
- GET request building
- POST request dengan body
- Header inclusion dan ordering
- Content-Length automatic calculation
- Response parsing
- Edge cases (malformed requests, incomplete responses)

---

### 🔹 CONFIGURATION & BUILD FILES

#### **CMakeLists.txt** (root)
**Tujuan**: Main CMake build configuration  
**Isi**:
- Project configuration (name, version, language)
- C++17 requirement
- Compiler flags dan options
- Dependency finding (OpenSSL, libpcap)
- Subdirectory includes (src, examples, tests)
- Install rules

#### **src/CMakeLists.txt**
**Tujuan**: Library build configuration  
**Isi**:
- Source file collection
- Library target definition (static/shared)
- Include directories
- Link libraries (OpenSSL, pthread)
- Export targets

#### **examples/CMakeLists.txt**
**Tujuan**: Build configuration untuk example programs  
**Isi**:
- Example executable targets
- Link dengan haquests library
- Install rules

#### **tests/CMakeLists.txt**
**Tujuan**: Build configuration untuk tests  
**Isi**:
- Google Test integration
- Test executable targets
- Test discovery
- Coverage options

#### **tools/CMakeLists.txt**
**Tujuan**: Build configuration untuk additional tools  
**Isi**:
- Utility tools compilation
- Debug/analysis tools

---

### 🔹 DEPLOYMENT & SCRIPTS

#### **Dockerfile**
**Tujuan**: Docker image definition untuk development  
**Isi**:
- Base image (Ubuntu dengan build tools)
- Dependencies installation (gcc, cmake, openssl, libpcap)
- Working directory setup
- User permissions untuk CAP_NET_RAW

#### **docker-compose.yml**
**Tujuan**: Docker compose configuration  
**Isi**:
- Development service definition
- Volume mounts (source code)
- Network configuration
- Capability grants (CAP_NET_RAW)
- Port mappings

#### **scripts/build.sh**
**Tujuan**: Build automation script  
**Isi**:
- CMake configuration
- Parallel compilation
- Error handling

#### **scripts/test.sh**
**Tujuan**: Test execution script  
**Isi**:
- Run all tests
- Generate coverage reports
- Summary output

#### **scripts/docker_build.sh**
**Tujuan**: Docker build automation  
**Isi**:
- Docker image building
- Container setup
- Development environment initialization

#### **scripts/setup_caps.sh**
**Tujuan**: Setup CAP_NET_RAW capability  
**Isi**:
- Capability granting untuk executables
- Permission setup
- Security considerations

---

### 🔹 DOCUMENTATION & LEGAL

#### **LICENSE**
**Tujuan**: MIT License text  
**Isi**: Full MIT license untuk open source distribution

#### **.gitignore**
**Tujuan**: Git ignore rules  
**Isi**:
- Build artifacts (build/, *.o, *.a)
- IDE files (.vscode/, .idea/)
- Temporary files
- OS-specific files

---

## 🎯 Navigasi Cepat

**Untuk memulai development:**
1. Baca `README.md` untuk overview dan setup
2. Lihat `examples/` untuk contoh usage
3. Explore `include/haquests/` untuk API documentation

**Untuk understanding internals:**
1. Mulai dari `include/haquests/core/` (layer paling bawah)
2. Naik ke `tcp/`, lalu `tls/`, lalu `http/`
3. Lihat `utils/` untuk supporting infrastructure

**Untuk testing:**
1. Lihat `tests/unit/` untuk existing tests
2. Run dengan `scripts/test.sh`
3. Add new tests sesuai kebutuhan

---

**Last Updated**: 2026-02-17  
**Maintainer**: [@HazaVVIP](https://github.com/HazaVVIP)
