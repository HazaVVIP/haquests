#include "haquests/core/packet.hpp"
#include "haquests/core/types.hpp"
#include "haquests/core/checksum.hpp"
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>

namespace haquests {
namespace core {

Packet::Packet() : data_len(0) {
    std::memset(&ip_header, 0, sizeof(ip_header));
    std::memset(&tcp_header, 0, sizeof(tcp_header));
    std::memset(data, 0, sizeof(data));
}

void Packet::buildIPHeader(uint32_t src_ip, uint32_t dst_ip, uint16_t total_len) {
    ip_header.version_ihl = (IP_VERSION_4 << 4) | 5;  // Version 4, IHL 5 (20 bytes)
    ip_header.tos = 0;
    ip_header.total_length = htons(total_len);
    ip_header.id = htons(rand() % 65535);
    ip_header.frag_off = 0;
    ip_header.ttl = DEFAULT_TTL;
    ip_header.protocol = IPPROTO_TCP;
    ip_header.src_addr = src_ip;
    ip_header.dst_addr = dst_ip;
    ip_header.checksum = 0;
    
    // Calculate IP checksum
    ip_header.checksum = checksum(reinterpret_cast<uint16_t*>(&ip_header), 
                                   sizeof(IPHeader));
}

void Packet::buildTCPHeader(uint16_t src_port, uint16_t dst_port,
                           uint32_t seq, uint32_t ack, uint8_t flags) {
    tcp_header.src_port = htons(src_port);
    tcp_header.dst_port = htons(dst_port);
    tcp_header.seq_num = htonl(seq);
    tcp_header.ack_num = htonl(ack);
    tcp_header.data_offset = (TCP_HEADER_SIZE / 4) << 4;  // Data offset in 32-bit words
    tcp_header.flags = flags;
    tcp_header.window = htons(DEFAULT_WINDOW_SIZE);
    tcp_header.urgent_ptr = 0;
    tcp_header.checksum = 0;
    
    // Calculate TCP checksum
    tcp_header.checksum = tcpChecksum(ip_header.src_addr, ip_header.dst_addr,
                                      reinterpret_cast<uint8_t*>(&tcp_header),
                                      TCP_HEADER_SIZE + data_len);
}

void Packet::setData(const uint8_t* payload, size_t len) {
    if (len > sizeof(data)) {
        len = sizeof(data);
    }
    std::memcpy(data, payload, len);
    data_len = len;
}

size_t Packet::getTotalSize() const {
    return sizeof(IPHeader) + sizeof(TCPHeader) + data_len;
}

size_t Packet::serialize(uint8_t* buffer, size_t buffer_size) const {
    size_t total_size = getTotalSize();
    if (buffer_size < total_size) {
        return 0;
    }
    
    std::memcpy(buffer, &ip_header, sizeof(IPHeader));
    std::memcpy(buffer + sizeof(IPHeader), &tcp_header, sizeof(TCPHeader));
    if (data_len > 0) {
        std::memcpy(buffer + sizeof(IPHeader) + sizeof(TCPHeader), data, data_len);
    }
    
    return total_size;
}

} // namespace core
} // namespace haquests
