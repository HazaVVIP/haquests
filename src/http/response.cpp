#include "haquests/http/response.hpp"
#include <sstream>
#include <algorithm>

namespace haquests {
namespace http {

Response::Response() : status_code_(0), is_complete_(false) {}

bool Response::parse(const uint8_t* data, size_t len) {
    std::string raw(reinterpret_cast<const char*>(data), len);
    return parse(raw);
}

bool Response::parse(const std::string& raw) {
    size_t header_end = raw.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return false;  // Incomplete response
    }
    
    // Parse headers
    std::string header_section = raw.substr(0, header_end);
    std::istringstream iss(header_section);
    std::string line;
    
    // Parse status line
    if (!std::getline(iss, line)) {
        return false;
    }
    if (!parseStatusLine(line)) {
        return false;
    }
    
    // Parse headers
    while (std::getline(iss, line) && !line.empty()) {
        if (line.back() == '\r') {
            line.pop_back();
        }
        
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            
            // Trim whitespace
            value.erase(0, value.find_first_not_of(" \t"));
            
            headers_[key] = value;
        }
    }
    
    // Parse body
    size_t body_start = header_end + 4;
    if (body_start < raw.size()) {
        const uint8_t* body_data = reinterpret_cast<const uint8_t*>(raw.data() + body_start);
        body_.assign(body_data, body_data + (raw.size() - body_start));
    }
    
    is_complete_ = true;
    return true;
}

bool Response::parseStatusLine(const std::string& line) {
    std::istringstream iss(line);
    
    if (line.back() == '\r') {
        std::string clean_line = line.substr(0, line.size() - 1);
        std::istringstream clean_iss(clean_line);
        clean_iss >> version_ >> status_code_;
        std::getline(clean_iss, status_message_);
    } else {
        iss >> version_ >> status_code_;
        std::getline(iss, status_message_);
    }
    
    // Trim status message
    if (!status_message_.empty() && status_message_[0] == ' ') {
        status_message_ = status_message_.substr(1);
    }
    
    return status_code_ > 0;
}

bool Response::parseHeaders(const std::string& header_section) {
    // Already implemented in parse()
    return true;
}

void Response::parseBody(const uint8_t* data, size_t len) {
    body_.assign(data, data + len);
}

int Response::getStatusCode() const {
    return status_code_;
}

std::string Response::getStatusMessage() const {
    return status_message_;
}

std::string Response::getVersion() const {
    return version_;
}

std::string Response::getHeader(const std::string& key) const {
    auto it = headers_.find(key);
    return (it != headers_.end()) ? it->second : "";
}

bool Response::hasHeader(const std::string& key) const {
    return headers_.find(key) != headers_.end();
}

const std::map<std::string, std::string>& Response::getHeaders() const {
    return headers_;
}

const std::vector<uint8_t>& Response::getBody() const {
    return body_;
}

std::string Response::getBodyAsString() const {
    return std::string(body_.begin(), body_.end());
}

bool Response::isComplete() const {
    return is_complete_;
}

bool Response::isChunked() const {
    std::string te = getHeader("Transfer-Encoding");
    return te.find("chunked") != std::string::npos;
}

} // namespace http
} // namespace haquests
