# HAQuests - HTTP Library dengan Raw Socket C++

> Low-Level HTTP/TLS Security Testing Library untuk Penetration Testing dan Security Research

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](https://www.kernel.org/)

## 📖 Tentang HAQuests

HAQuests adalah library C++ yang dirancang khusus untuk security research dan penetration testing dengan fokus pada manipulasi low-level network protocol. Library ini memberikan kontrol penuh terhadap HTTP/TLS communication melalui raw socket, memungkinkan testing untuk vulnerability seperti HTTP request smuggling.

### Fitur Utama

- **🔧 Raw Socket Control**: Manipulasi langsung TCP packets pada network layer
- **🔒 TLS/SSL Support**: Integrasi OpenSSL untuk HTTPS dengan custom BIO adapter
- **⚡ HTTP Request Smuggling**: Built-in support untuk CL.TE, TE.CL, dan TE.TE techniques
- **🛡️ Security Testing**: Tools untuk WAF bypass dan parser differential analysis
- **📦 Modular Design**: Arsitektur berlapis (Core, TCP, TLS, HTTP, Utils)

### Kasus Penggunaan

- HTTP Request Smuggling vulnerability testing
- Web Application Firewall (WAF) bypass testing
- Network protocol research dan analysis
- Penetration testing dengan custom HTTP behaviors
- TLS/SSL security testing

## ⚠️ Disclaimer

**PENTING**: Library ini dirancang khusus untuk authorized security testing dan research purposes. Penggunaan tanpa izin terhadap sistem yang bukan milik Anda atau tanpa explicit permission dapat melanggar hukum dan regulasi. Selalu pastikan Anda memiliki izin eksplisit sebelum melakukan testing terhadap sistem apapun.

## 🚀 Instalasi

### Prerequisites

- Linux (Ubuntu 20.04+ atau distribusi lain)
- C++17 compiler (GCC 9+ atau Clang 10+)
- CMake 3.15 atau lebih tinggi
- OpenSSL 1.1.1+
- libpcap
- CAP_NET_RAW capability atau root access
- Docker (opsional, untuk development)

### Build dari Source

```bash
# Clone repository
git clone https://github.com/HazaVVIP/haquests.git
cd haquests

# Buat build directory
mkdir build && cd build

# Configure dan compile
cmake ..
make -j$(nproc)

# Install (membutuhkan sudo)
sudo make install
```

### Development dengan Docker

```bash
# Build dan jalankan container
docker-compose up -d haquests-dev
docker-compose exec haquests-dev bash

# Build di dalam container
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

## 📚 Cara Penggunaan

### 1. Basic HTTP Request

```cpp
#include <haquests/haquests.hpp>
#include <iostream>

int main() {
    using namespace haquests;
    
    // Buat HTTP request
    HTTP::Request request;
    request.setMethod("GET");
    request.setPath("/");
    request.setVersion("HTTP/1.1");
    request.addHeader("Host", "example.com");
    request.addHeader("User-Agent", "HAQuests/1.0");
    
    // Buat TCP connection
    TCP::Connection conn("93.184.216.34", 80); // example.com IP
    conn.connect();
    
    // Kirim request
    std::string rawRequest = request.build();
    conn.send(rawRequest);
    
    // Terima response
    std::string response = conn.receive();
    std::cout << response << std::endl;
    
    return 0;
}
```

### 2. HTTPS Request dengan TLS

```cpp
#include <haquests/haquests.hpp>
#include <iostream>

int main() {
    using namespace haquests;
    
    // Buat TLS connection
    TLS::Connection tls_conn("www.google.com", 443);
    tls_conn.connect();
    
    // Buat dan kirim HTTPS request
    HTTP::Request request;
    request.setMethod("GET");
    request.setPath("/");
    request.setVersion("HTTP/1.1");
    request.addHeader("Host", "www.google.com");
    
    tls_conn.send(request.build());
    
    // Baca response
    std::string response = tls_conn.receive();
    std::cout << response << std::endl;
    
    return 0;
}
```

### 3. HTTP Request Smuggling (CL.TE)

```cpp
#include <haquests/haquests.hpp>
#include <iostream>

int main() {
    using namespace haquests;
    
    // CATATAN: Hanya gunakan untuk authorized testing!
    
    // Buat smuggling request
    HTTP::Smuggling smuggler;
    smuggler.setType(HTTP::Smuggling::Type::CL_TE);
    
    // Request pertama (akan dilihat front-end)
    std::string frontendRequest = 
        "GET / HTTP/1.1\r\n"
        "Host: vulnerable-site.com\r\n\r\n";
    
    // Request kedua (akan di-smuggle ke back-end)
    std::string backendRequest = 
        "GET /admin HTTP/1.1\r\n"
        "Host: vulnerable-site.com\r\n\r\n";
    
    // Build smuggling payload
    std::string payload = smuggler.build(frontendRequest, backendRequest);
    
    // Kirim menggunakan raw TCP
    TCP::Connection conn("target-ip", 80);
    conn.connect();
    conn.send(payload);
    
    std::string response = conn.receive();
    std::cout << response << std::endl;
    
    return 0;
}
```

### 4. Compile dan Run

```bash
# Compile contoh program
g++ -std=c++17 my_program.cpp -lhaquests -lssl -lcrypto -o my_program

# Jalankan (membutuhkan CAP_NET_RAW atau root)
sudo ./my_program

# Atau dengan setcap
sudo setcap cap_net_raw+ep ./my_program
./my_program
```

## 🏗️ Arsitektur

HAQuests menggunakan arsitektur berlapis:

1. **Core Layer**: Raw socket, packet structures, checksums
2. **TCP Layer**: Connection management, state machine (RFC 793), flow control
3. **TLS Layer**: OpenSSL wrapper dengan custom BIO adapter
4. **HTTP Layer**: Request/response parsing, smuggling techniques
5. **Utils Layer**: Logging, error handling, buffers, timers

Lihat file **DIR.md** untuk detail struktur direktori dan penjelasan setiap file.

## 🧪 Testing

```bash
# Build dengan tests
cd build
cmake -DBUILD_TESTS=ON ..
make -j$(nproc)

# Jalankan unit tests
sudo ./tests/run_tests

# Atau individual test
sudo ./tests/unit/test_checksum
sudo ./tests/unit/test_http_parser
```

## 📄 Lisensi

Proyek ini dilisensikan di bawah MIT License - lihat file [LICENSE](LICENSE) untuk detail.

## 🙏 Acknowledgments

- OpenSSL team untuk TLS/SSL library
- libpcap developers untuk packet capture support
- Security research community
- PortSwigger Research untuk HTTP Request Smuggling research

## 📞 Kontak

- GitHub: [@HazaVVIP](https://github.com/HazaVVIP)
- Repository: [haquests](https://github.com/HazaVVIP/haquests)

## ⚖️ Legal Notice

This tool is provided for educational and authorized testing purposes only. The authors are not responsible for any misuse or damage caused by this tool. Always ensure you have explicit permission before testing any system.

---

**Made with ❤️ for the security research community**
