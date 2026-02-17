#include "haquests/core/checksum.hpp"
#include "haquests/core/packet.hpp"
#include <cstring>
#include <arpa/inet.h>

namespace haquests {
namespace core {

uint16_t checksum(const uint16_t* buffer, size_t size) {
    unsigned long cksum = 0;
    
    while (size > 1) {
        cksum += *buffer++;
        size -= sizeof(uint16_t);
    }
    
    if (size) {
        cksum += *reinterpret_cast<const uint8_t*>(buffer);
    }
    
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    
    return static_cast<uint16_t>(~cksum);
}

uint16_t tcpChecksum(uint32_t src_ip, uint32_t dst_ip,
                     const uint8_t* tcp_segment, size_t tcp_len) {
    // Create pseudo-header
    PseudoHeader pseudo;
    pseudo.src_addr = src_ip;
    pseudo.dst_addr = dst_ip;
    pseudo.zero = 0;
    pseudo.protocol = IPPROTO_TCP_NUM;
    pseudo.tcp_length = htons(tcp_len);
    
    // Calculate checksum over pseudo-header + TCP segment
    size_t total_len = sizeof(PseudoHeader) + tcp_len;
    uint8_t* temp_buffer = new uint8_t[total_len];
    
    std::memcpy(temp_buffer, &pseudo, sizeof(PseudoHeader));
    std::memcpy(temp_buffer + sizeof(PseudoHeader), tcp_segment, tcp_len);
    
    uint16_t result = checksum(reinterpret_cast<uint16_t*>(temp_buffer), total_len);
    
    delete[] temp_buffer;
    return result;
}

bool verifyChecksum(const uint16_t* buffer, size_t size) {
    return checksum(buffer, size) == 0;
}

} // namespace core
} // namespace haquests
