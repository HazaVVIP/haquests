#pragma once

#include <cstdint>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

namespace haquests {
namespace core {

// IP header structure
struct IPHeader {
    uint8_t  version_ihl;    // Version (4 bits) + Header length (4 bits)
    uint8_t  tos;            // Type of service
    uint16_t total_length;   // Total length
    uint16_t id;             // Identification
    uint16_t frag_off;       // Fragment offset
    uint8_t  ttl;            // Time to live
    uint8_t  protocol;       // Protocol (TCP = 6)
    uint16_t checksum;       // Header checksum
    uint32_t src_addr;       // Source address
    uint32_t dst_addr;       // Destination address
} __attribute__((packed));

// TCP header structure
struct TCPHeader {
    uint16_t src_port;       // Source port
    uint16_t dst_port;       // Destination port
    uint32_t seq_num;        // Sequence number
    uint32_t ack_num;        // Acknowledgment number
    uint8_t  data_offset;    // Data offset (4 bits) + reserved (4 bits)
    uint8_t  flags;          // TCP flags
    uint16_t window;         // Window size
    uint16_t checksum;       // Checksum
    uint16_t urgent_ptr;     // Urgent pointer
} __attribute__((packed));

// Pseudo header for TCP checksum calculation
struct PseudoHeader {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint8_t  zero;
    uint8_t  protocol;
    uint16_t tcp_length;
} __attribute__((packed));

// Complete packet structure
class Packet {
public:
    IPHeader ip_header;
    TCPHeader tcp_header;
    uint8_t data[65535 - sizeof(IPHeader) - sizeof(TCPHeader)];
    size_t data_len;

    Packet();
    
    // Build IP header
    void buildIPHeader(uint32_t src_ip, uint32_t dst_ip, uint16_t total_len);
    
    // Build TCP header
    void buildTCPHeader(uint16_t src_port, uint16_t dst_port, 
                        uint32_t seq, uint32_t ack, uint8_t flags);
    
    // Set data payload
    void setData(const uint8_t* payload, size_t len);
    
    // Get total packet size
    size_t getTotalSize() const;
    
    // Serialize packet to buffer
    size_t serialize(uint8_t* buffer, size_t buffer_size) const;
};

} // namespace core
} // namespace haquests
