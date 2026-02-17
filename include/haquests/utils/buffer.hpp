#pragma once

#include <vector>
#include <cstdint>
#include <cstring>

namespace haquests {
namespace utils {

class Buffer {
public:
    Buffer();
    explicit Buffer(size_t capacity);
    
    // Write operations
    void write(const uint8_t* data, size_t len);
    void write(const std::vector<uint8_t>& data);
    void writeU8(uint8_t value);
    void writeU16(uint16_t value);
    void writeU32(uint32_t value);
    
    // Read operations
    std::vector<uint8_t> read(size_t len);
    uint8_t readU8();
    uint16_t readU16();
    uint32_t readU32();
    
    // Peek operations (without advancing read position)
    std::vector<uint8_t> peek(size_t len) const;
    uint8_t peekU8() const;
    
    // Buffer management
    void clear();
    void reserve(size_t capacity);
    void compact();
    
    // Get buffer info
    size_t size() const;
    size_t capacity() const;
    size_t available() const;
    bool empty() const;
    
    // Direct access
    const uint8_t* data() const;
    uint8_t* data();

private:
    std::vector<uint8_t> buffer_;
    size_t read_pos_;
    size_t write_pos_;
};

} // namespace utils
} // namespace haquests
