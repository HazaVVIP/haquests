#include "haquests/http/chunked.hpp"
#include <sstream>
#include <iomanip>

namespace haquests {
namespace http {

std::vector<uint8_t> Chunked::encode(const uint8_t* data, size_t len) {
    std::vector<uint8_t> result;
    
    // Chunk size in hex
    std::ostringstream oss;
    oss << std::hex << len << "\r\n";
    std::string size_str = oss.str();
    
    result.insert(result.end(), size_str.begin(), size_str.end());
    result.insert(result.end(), data, data + len);
    result.push_back('\r');
    result.push_back('\n');
    
    // Last chunk
    result.push_back('0');
    result.push_back('\r');
    result.push_back('\n');
    result.push_back('\r');
    result.push_back('\n');
    
    return result;
}

std::string Chunked::encodeString(const std::string& data) {
    auto vec = encode(reinterpret_cast<const uint8_t*>(data.data()), data.size());
    return std::string(vec.begin(), vec.end());
}

std::vector<uint8_t> Chunked::decode(const uint8_t* data, size_t len) {
    std::vector<uint8_t> result;
    size_t pos = 0;
    
    while (pos < len) {
        // Find chunk size
        size_t chunk_size;
        size_t consumed = parseChunkSize(data + pos, len - pos, chunk_size);
        if (consumed == 0) {
            break;
        }
        
        pos += consumed;
        
        if (chunk_size == 0) {
            break;  // Last chunk
        }
        
        // Copy chunk data
        if (pos + chunk_size <= len) {
            result.insert(result.end(), data + pos, data + pos + chunk_size);
            pos += chunk_size;
        }
        
        // Skip CRLF
        if (pos + 2 <= len) {
            pos += 2;
        }
    }
    
    return result;
}

std::string Chunked::decodeString(const std::string& data) {
    auto vec = decode(reinterpret_cast<const uint8_t*>(data.data()), data.size());
    return std::string(vec.begin(), vec.end());
}

bool Chunked::isChunked(const uint8_t* data, size_t len) {
    // Simple heuristic: check if starts with hex digit
    if (len == 0) return false;
    char c = static_cast<char>(data[0]);
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

size_t Chunked::parseChunkSize(const uint8_t* data, size_t len, size_t& chunk_size) {
    std::string hex_str;
    size_t i = 0;
    
    // Read hex digits
    while (i < len) {
        char c = static_cast<char>(data[i]);
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
            hex_str += c;
            i++;
        } else if (c == '\r' || c == ';') {
            break;
        } else {
            break;
        }
    }
    
    if (hex_str.empty()) {
        return 0;
    }
    
    // Convert hex to size
    chunk_size = std::stoul(hex_str, nullptr, 16);
    
    // Skip to CRLF
    while (i < len && data[i] != '\n') {
        i++;
    }
    if (i < len) {
        i++;  // Skip \n
    }
    
    return i;
}

} // namespace http
} // namespace haquests
