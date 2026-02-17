#pragma once

#include <string>
#include <map>
#include <vector>
#include <cstdint>

namespace haquests {
namespace http {

class Request {
public:
    Request();
    Request(const std::string& method, const std::string& path);
    
    // Set method and path
    void setMethod(const std::string& method);
    void setPath(const std::string& path);
    void setVersion(const std::string& version);
    
    // Headers
    void addHeader(const std::string& key, const std::string& value);
    void setHeader(const std::string& key, const std::string& value);
    std::string getHeader(const std::string& key) const;
    bool hasHeader(const std::string& key) const;
    void removeHeader(const std::string& key);
    
    // Body
    void setBody(const std::string& body);
    void setBody(const uint8_t* data, size_t len);
    
    // Build complete request
    std::string build() const;
    std::vector<uint8_t> buildRaw() const;
    
    // Common request types
    static Request GET(const std::string& url);
    static Request POST(const std::string& url, const std::string& body);
    static Request PUT(const std::string& url, const std::string& body);
    static Request DELETE(const std::string& url);

private:
    std::string method_;
    std::string path_;
    std::string version_;
    std::map<std::string, std::string> headers_;
    std::vector<uint8_t> body_;
};

} // namespace http
} // namespace haquests
