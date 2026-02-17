#include "haquests/http/headers.hpp"
#include <algorithm>
#include <sstream>

namespace haquests {
namespace http {

Headers::Headers() {}

void Headers::add(const std::string& key, const std::string& value) {
    headers_.insert({normalizeKey(key), value});
}

void Headers::set(const std::string& key, const std::string& value) {
    std::string norm_key = normalizeKey(key);
    headers_.erase(norm_key);
    headers_.insert({norm_key, value});
}

std::string Headers::get(const std::string& key) const {
    auto it = headers_.find(normalizeKey(key));
    return (it != headers_.end()) ? it->second : "";
}

std::vector<std::string> Headers::getAll(const std::string& key) const {
    std::vector<std::string> result;
    std::string norm_key = normalizeKey(key);
    
    auto range = headers_.equal_range(norm_key);
    for (auto it = range.first; it != range.second; ++it) {
        result.push_back(it->second);
    }
    
    return result;
}

bool Headers::has(const std::string& key) const {
    return headers_.find(normalizeKey(key)) != headers_.end();
}

void Headers::remove(const std::string& key) {
    headers_.erase(normalizeKey(key));
}

bool Headers::parse(const std::string& header_section) {
    std::istringstream iss(header_section);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.empty() || line == "\r") {
            continue;
        }
        
        // Remove trailing \r
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            
            // Trim whitespace from value
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            add(key, value);
        }
    }
    
    return true;
}

std::string Headers::build() const {
    std::ostringstream oss;
    
    for (const auto& header : headers_) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    return oss.str();
}

const std::multimap<std::string, std::string>& Headers::getAll() const {
    return headers_;
}

std::string Headers::normalizeKey(const std::string& key) const {
    // Keep original case for now (HTTP headers are case-insensitive but we preserve case)
    return key;
}

} // namespace http
} // namespace haquests
