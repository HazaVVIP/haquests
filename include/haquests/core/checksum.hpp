#pragma once

#include <cstdint>
#include <cstddef>

namespace haquests {
namespace core {

// Calculate checksum for IP/TCP headers
uint16_t checksum(const uint16_t* buffer, size_t size);

// Calculate TCP checksum with pseudo-header
uint16_t tcpChecksum(uint32_t src_ip, uint32_t dst_ip,
                     const uint8_t* tcp_segment, size_t tcp_len);

// Verify checksum
bool verifyChecksum(const uint16_t* buffer, size_t size);

} // namespace core
} // namespace haquests
