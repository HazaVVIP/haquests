#include "haquests/http/request.hpp"
#include <sstream>

namespace haquests {
namespace http {

Request::Request() : method_("GET"), path_("/"), version_("HTTP/1.1") {}

Request::Request(const std::string& method, const std::string& path)
    : method_(method), path_(path), version_("HTTP/1.1") {}

void Request::setMethod(const std::string& method) {
    method_ = method;
}

void Request::setPath(const std::string& path) {
    path_ = path;
}

void Request::setVersion(const std::string& version) {
    version_ = version;
}

void Request::addHeader(const std::string& key, const std::string& value) {
    headers_[key] = value;
}

void Request::setHeader(const std::string& key, const std::string& value) {
    headers_[key] = value;
}

std::string Request::getHeader(const std::string& key) const {
    auto it = headers_.find(key);
    return (it != headers_.end()) ? it->second : "";
}

bool Request::hasHeader(const std::string& key) const {
    return headers_.find(key) != headers_.end();
}

void Request::removeHeader(const std::string& key) {
    headers_.erase(key);
}

void Request::setBody(const std::string& body) {
    body_.assign(body.begin(), body.end());
    setHeader("Content-Length", std::to_string(body_.size()));
}

void Request::setBody(const uint8_t* data, size_t len) {
    body_.assign(data, data + len);
    setHeader("Content-Length", std::to_string(body_.size()));
}

std::string Request::build() const {
    std::ostringstream oss;
    
    // Request line
    oss << method_ << " " << path_ << " " << version_ << "\r\n";
    
    // Headers
    for (const auto& header : headers_) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    // Empty line
    oss << "\r\n";
    
    // Body
    if (!body_.empty()) {
        oss.write(reinterpret_cast<const char*>(body_.data()), body_.size());
    }
    
    return oss.str();
}

std::vector<uint8_t> Request::buildRaw() const {
    std::string str = build();
    return std::vector<uint8_t>(str.begin(), str.end());
}

Request Request::GET(const std::string& url) {
    Request req("GET", url);
    req.setHeader("User-Agent", "HAQuests/0.1");
    return req;
}

Request Request::POST(const std::string& url, const std::string& body) {
    Request req("POST", url);
    req.setHeader("User-Agent", "HAQuests/0.1");
    req.setBody(body);
    return req;
}

Request Request::PUT(const std::string& url, const std::string& body) {
    Request req("PUT", url);
    req.setHeader("User-Agent", "HAQuests/0.1");
    req.setBody(body);
    return req;
}

Request Request::DELETE(const std::string& url) {
    Request req("DELETE", url);
    req.setHeader("User-Agent", "HAQuests/0.1");
    return req;
}

} // namespace http
} // namespace haquests
