#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace haquests {
namespace http {

class Chunked {
public:
    // Encode data as chunked
    static std::vector<uint8_t> encode(const uint8_t* data, size_t len);
    static std::string encodeString(const std::string& data);
    
    // Decode chunked data
    static std::vector<uint8_t> decode(const uint8_t* data, size_t len);
    static std::string decodeString(const std::string& data);
    
    // Check if data is chunked encoded
    static bool isChunked(const uint8_t* data, size_t len);
    
    // Parse single chunk
    static size_t parseChunkSize(const uint8_t* data, size_t len, size_t& chunk_size);
};

} // namespace http
} // namespace haquests
