#pragma once

#include <string>
#include <exception>
#include <stdexcept>

namespace haquests {
namespace utils {

class Error : public std::runtime_error {
public:
    explicit Error(const std::string& message);
    Error(const std::string& message, int error_code);
    
    int getErrorCode() const;
    const char* what() const noexcept override;

private:
    std::string message_;
    int error_code_;
};

// Specific error types
class SocketError : public Error {
public:
    explicit SocketError(const std::string& message);
};

class ConnectionError : public Error {
public:
    explicit ConnectionError(const std::string& message);
};

class TLSError : public Error {
public:
    explicit TLSError(const std::string& message);
};

class HTTPError : public Error {
public:
    explicit HTTPError(const std::string& message);
    HTTPError(const std::string& message, int status_code);
    
    int getStatusCode() const;

private:
    int status_code_;
};

class ParseError : public Error {
public:
    explicit ParseError(const std::string& message);
};

} // namespace utils
} // namespace haquests
