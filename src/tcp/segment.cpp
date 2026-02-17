#include "haquests/tcp/segment.hpp"
#include <cstring>

namespace haquests {
namespace tcp {

Segment::Segment() 
    : src_port(0), dst_port(0), seq_num(0), ack_num(0), flags(0), window(0) {}

Segment::Segment(uint16_t src, uint16_t dst, uint32_t seq, uint32_t ack, uint8_t f)
    : src_port(src), dst_port(dst), seq_num(seq), ack_num(ack), flags(f), window(65535) {}

void Segment::setData(const uint8_t* payload, size_t len) {
    data.assign(payload, payload + len);
}

size_t Segment::getDataLength() const {
    return data.size();
}

bool Segment::hasFlag(uint8_t flag) const {
    return (flags & flag) != 0;
}

} // namespace tcp
} // namespace haquests
