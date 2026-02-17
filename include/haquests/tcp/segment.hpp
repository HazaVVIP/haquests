#pragma once

#include <cstdint>
#include <vector>

namespace haquests {
namespace tcp {

class Segment {
public:
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t flags;
    uint16_t window;
    std::vector<uint8_t> data;
    
    Segment();
    Segment(uint16_t src, uint16_t dst, uint32_t seq, uint32_t ack, uint8_t flags);
    
    // Set data
    void setData(const uint8_t* payload, size_t len);
    
    // Get data length
    size_t getDataLength() const;
    
    // Check if segment has flag
    bool hasFlag(uint8_t flag) const;
};

} // namespace tcp
} // namespace haquests
