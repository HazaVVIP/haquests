# Research: Raw Socket Programming di Linux

## Overview

Raw socket memberikan akses langsung ke network layer, melewati protokol stack normal OS. Ini memungkinkan kita untuk:
- Membuat custom protocol headers
- Mengirim malformed packets
- Mengimplementasikan custom TCP/IP stack

## Socket Types untuk Proyek Ini

### 1. SOCK_RAW dengan IPPROTO_TCP
```cpp
int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
```

**Karakteristik:**
- Akses langsung ke IP layer
- Harus membuat sendiri TCP header dan payload
- OS tidak menangani acknowledgments atau sequencing
- **Kelebihan**: Full control atas TCP packets
- **Kekurangan**: Harus implement full TCP state machine

### 2. AF_PACKET Socket
```cpp
int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
```

**Karakteristik:**
- Akses di data link layer (Layer 2)
- Harus membuat ethernet frame, IP header, dan TCP header
- Paling low-level, paling kompleks
- **Kelebihan**: Kontrol absolut, bahkan atas MAC addresses
- **Kekurangan**: Sangat kompleks, perlu handle routing

### 3. Hybrid Approach (RECOMMENDED)
Menggunakan kombinasi:
- Raw socket untuk sending custom TCP packets
- Normal socket API untuk receiving (libpcap atau raw socket dengan filtering)

## Privilege Requirements

### CAP_NET_RAW Capability
```bash
# Check current capabilities
getcap /path/to/binary

# Set capability (instead of running as root)
sudo setcap cap_net_raw=eip /path/to/binary

# Running in Docker
docker run --cap-add=NET_RAW --cap-add=NET_ADMIN ...
```

### Alternatif Jika Tidak Ada CAP_NET_RAW
1. **User-mode networking**: DPDK atau similar (overkill untuk use case ini)
2. **Virtual network namespaces**: Testing dalam isolated namespace
3. **Docker dengan privileges**: Development environment

## IP Header Structure

```cpp
struct iphdr {
    uint8_t  ihl:4;        // Header length
    uint8_t  version:4;    // Version (IPv4 = 4)
    uint8_t  tos;          // Type of service
    uint16_t tot_len;      // Total length
    uint16_t id;           // Identification
    uint16_t frag_off;     // Fragment offset
    uint8_t  ttl;          // Time to live
    uint8_t  protocol;     // Protocol (TCP = 6)
    uint16_t check;        // Header checksum
    uint32_t saddr;        // Source address
    uint32_t daddr;        // Destination address
};
```

## TCP Header Structure

```cpp
struct tcphdr {
    uint16_t source;       // Source port
    uint16_t dest;         // Destination port
    uint32_t seq;          // Sequence number
    uint32_t ack_seq;      // Acknowledgment number
    uint16_t res1:4;       // Reserved
    uint16_t doff:4;       // Data offset
    uint16_t fin:1;        // FIN flag
    uint16_t syn:1;        // SYN flag
    uint16_t rst:1;        // RST flag
    uint16_t psh:1;        // PSH flag
    uint16_t ack:1;        // ACK flag
    uint16_t urg:1;        // URG flag
    uint16_t res2:2;       // Reserved
    uint16_t window;       // Window size
    uint16_t check;        // Checksum
    uint16_t urg_ptr;      // Urgent pointer
};
```

## Checksum Calculation

**Critical**: IP dan TCP checksums harus benar atau packets akan di-drop.

```cpp
uint16_t checksum(uint16_t *buffer, int size) {
    unsigned long cksum = 0;
    while (size > 1) {
        cksum += *buffer++;
        size -= sizeof(uint16_t);
    }
    if (size) {
        cksum += *(uint8_t*)buffer;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    return (uint16_t)(~cksum);
}
```

### TCP Pseudo-Header untuk Checksum
TCP checksum dihitung dengan pseudo-header:
```cpp
struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t  placeholder;
    uint8_t  protocol;
    uint16_t tcp_length;
};
```

## Sending Raw Packets

### Basic Send Flow
```cpp
// 1. Create socket
int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);

// 2. Set IP_HDRINCL to tell kernel we provide IP header
int one = 1;
setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

// 3. Build packet (IP header + TCP header + data)
char packet[4096];
// ... populate headers ...

// 4. Send
struct sockaddr_in dest;
dest.sin_family = AF_INET;
dest.sin_addr.s_addr = destination_ip;
sendto(sock, packet, packet_size, 0, 
       (struct sockaddr*)&dest, sizeof(dest));
```

## Receiving Packets

### Option 1: Raw Socket dengan BPF Filter
```cpp
int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
// Attach BPF filter untuk hanya terima packets yang relevant
struct sock_filter code[] = { /* BPF program */ };
struct sock_fprog prog = { .len = ..., .filter = code };
setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &prog, sizeof(prog));
```

### Option 2: libpcap
```cpp
pcap_t *handle = pcap_open_live("eth0", BUFSIZ, 1, 1000, errbuf);
struct bpf_program fp;
pcap_compile(handle, &fp, "tcp port 80", 0, PCAP_NETMASK_UNKNOWN);
pcap_setfilter(handle, &fp);
pcap_loop(handle, -1, packet_handler, NULL);
```

**Recommendation**: libpcap untuk receiving, raw socket untuk sending.

## Common Pitfalls

### 1. Kernel Interference
**Problem**: Kernel's TCP stack mungkin kirim RST packets untuk ports yang kita "bajak".

**Solution**: 
- Gunakan iptables untuk block kernel RST:
  ```bash
  iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP
  ```
- Atau gunakan port yang tidak di-listen oleh services lain

### 2. Network Byte Order
**Problem**: Semua multi-byte values harus dalam network byte order (big-endian).

**Solution**:
```cpp
// Host to network
uint16_t port_network = htons(port_host);
uint32_t ip_network = htonl(ip_host);

// Network to host
uint16_t port_host = ntohs(port_network);
```

### 3. MTU dan Fragmentation
**Problem**: Packets lebih besar dari MTU akan di-fragment atau di-drop.

**Solution**:
- Keep packets under 1500 bytes (typical Ethernet MTU)
- Atau handle IP fragmentation sendiri

## Testing Strategy

### Phase 1: Loopback Testing
```bash
# Test pada loopback interface (tidak butuh CAP_NET_RAW untuk receive)
tcpdump -i lo -n tcp port 8080
```

### Phase 2: Virtual Network
```bash
# Setup virtual network pair
ip netns add test1
ip netns add test2
ip link add veth0 type veth peer name veth1
ip link set veth0 netns test1
ip link set veth1 netns test2
```

### Phase 3: Docker Testing
```dockerfile
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    build-essential \
    libpcap-dev \
    tcpdump
# Copy and build
COPY . /app
WORKDIR /app
RUN make
# Run with capabilities
CMD ["./haquests"]
```

```bash
docker build -t haquests .
docker run --cap-add=NET_RAW --cap-add=NET_ADMIN haquests
```

## Libraries dan Tools

### Essential
- **libpcap**: Packet capture library
- **libnl**: Netlink library (untuk network configuration)
- **tcpdump/wireshark**: Untuk debugging packets

### Optional
- **scapy**: Python tool untuk prototyping packet structures
- **hping3**: Reference implementation untuk custom TCP

## Performance Considerations

### Zero-Copy Techniques
- `sendmsg()` dengan `MSG_ZEROCOPY`
- Memory-mapped packet sockets (PACKET_MMAP)
- Batch processing multiple packets

### Buffer Management
- Pre-allocate packet buffers
- Ring buffers untuk send/receive queues
- Avoid dynamic allocation in hot path

## Security Considerations

### Input Validation
- Validate all received packets (bisa saja malicious)
- Bounds checking pada semua buffer operations
- Prevent buffer overflows dalam packet parsing

### Rate Limiting
- Implement rate limiting untuk prevent flooding
- Connection tracking untuk manage state

## References

1. **RFC 793**: TCP Specification
2. **RFC 791**: IP Specification
3. **Berkeley Packet Filter (BPF)**: Filter specification
4. **Linux Socket Programming**: Advanced guide
5. **TCP/IP Illustrated**: Stevens, Vol. 1

## Next Steps

1. Prototype simple packet sending
2. Verify checksums dengan wireshark
3. Test 3-way handshake minimal
4. Integrate dengan existing socket untuk receive
