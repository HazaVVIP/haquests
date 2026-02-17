#include "haquests/utils/buffer.hpp"
#include <algorithm>
#include <stdexcept>
#include <arpa/inet.h>

namespace haquests {
namespace utils {

Buffer::Buffer() : read_pos_(0), write_pos_(0) {
    buffer_.reserve(4096);
}

Buffer::Buffer(size_t capacity) : read_pos_(0), write_pos_(0) {
    buffer_.reserve(capacity);
}

void Buffer::write(const uint8_t* data, size_t len) {
    if (write_pos_ + len > buffer_.capacity()) {
        buffer_.reserve(std::max(buffer_.capacity() * 2, write_pos_ + len));
    }
    
    if (write_pos_ + len > buffer_.size()) {
        buffer_.resize(write_pos_ + len);
    }
    
    std::memcpy(buffer_.data() + write_pos_, data, len);
    write_pos_ += len;
}

void Buffer::write(const std::vector<uint8_t>& data) {
    write(data.data(), data.size());
}

void Buffer::writeU8(uint8_t value) {
    write(&value, 1);
}

void Buffer::writeU16(uint16_t value) {
    uint16_t net_value = htons(value);
    write(reinterpret_cast<uint8_t*>(&net_value), sizeof(net_value));
}

void Buffer::writeU32(uint32_t value) {
    uint32_t net_value = htonl(value);
    write(reinterpret_cast<uint8_t*>(&net_value), sizeof(net_value));
}

std::vector<uint8_t> Buffer::read(size_t len) {
    if (read_pos_ + len > write_pos_) {
        len = write_pos_ - read_pos_;
    }
    
    std::vector<uint8_t> result(buffer_.begin() + read_pos_, 
                                buffer_.begin() + read_pos_ + len);
    read_pos_ += len;
    
    return result;
}

uint8_t Buffer::readU8() {
    if (read_pos_ >= write_pos_) {
        throw std::runtime_error("Buffer underflow");
    }
    return buffer_[read_pos_++];
}

uint16_t Buffer::readU16() {
    if (read_pos_ + sizeof(uint16_t) > write_pos_) {
        throw std::runtime_error("Buffer underflow");
    }
    
    uint16_t net_value;
    std::memcpy(&net_value, buffer_.data() + read_pos_, sizeof(net_value));
    read_pos_ += sizeof(net_value);
    
    return ntohs(net_value);
}

uint32_t Buffer::readU32() {
    if (read_pos_ + sizeof(uint32_t) > write_pos_) {
        throw std::runtime_error("Buffer underflow");
    }
    
    uint32_t net_value;
    std::memcpy(&net_value, buffer_.data() + read_pos_, sizeof(net_value));
    read_pos_ += sizeof(net_value);
    
    return ntohl(net_value);
}

std::vector<uint8_t> Buffer::peek(size_t len) const {
    if (read_pos_ + len > write_pos_) {
        len = write_pos_ - read_pos_;
    }
    
    return std::vector<uint8_t>(buffer_.begin() + read_pos_,
                                buffer_.begin() + read_pos_ + len);
}

uint8_t Buffer::peekU8() const {
    if (read_pos_ >= write_pos_) {
        throw std::runtime_error("Buffer underflow");
    }
    return buffer_[read_pos_];
}

void Buffer::clear() {
    read_pos_ = 0;
    write_pos_ = 0;
    buffer_.clear();
}

void Buffer::reserve(size_t capacity) {
    buffer_.reserve(capacity);
}

void Buffer::compact() {
    if (read_pos_ > 0) {
        size_t remaining = write_pos_ - read_pos_;
        if (remaining > 0) {
            std::memmove(buffer_.data(), buffer_.data() + read_pos_, remaining);
        }
        read_pos_ = 0;
        write_pos_ = remaining;
    }
}

size_t Buffer::size() const {
    return write_pos_ - read_pos_;
}

size_t Buffer::capacity() const {
    return buffer_.capacity();
}

size_t Buffer::available() const {
    return write_pos_ - read_pos_;
}

bool Buffer::empty() const {
    return read_pos_ >= write_pos_;
}

const uint8_t* Buffer::data() const {
    return buffer_.data() + read_pos_;
}

uint8_t* Buffer::data() {
    return buffer_.data() + read_pos_;
}

} // namespace utils
} // namespace haquests
