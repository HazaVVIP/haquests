#pragma once

#include <string>
#include <map>
#include <vector>

namespace haquests {
namespace http {

class Response {
public:
    Response();
    
    // Parse response from raw data
    bool parse(const uint8_t* data, size_t len);
    bool parse(const std::string& raw);
    
    // Getters
    int getStatusCode() const;
    std::string getStatusMessage() const;
    std::string getVersion() const;
    
    // Headers
    std::string getHeader(const std::string& key) const;
    bool hasHeader(const std::string& key) const;
    const std::map<std::string, std::string>& getHeaders() const;
    
    // Body
    const std::vector<uint8_t>& getBody() const;
    std::string getBodyAsString() const;
    
    // Check if complete
    bool isComplete() const;
    
    // Check if chunked
    bool isChunked() const;

private:
    bool parseStatusLine(const std::string& line);
    bool parseHeaders(const std::string& header_section);
    void parseBody(const uint8_t* data, size_t len);
    
    std::string version_;
    int status_code_;
    std::string status_message_;
    std::map<std::string, std::string> headers_;
    std::vector<uint8_t> body_;
    bool is_complete_;
};

} // namespace http
} // namespace haquests
