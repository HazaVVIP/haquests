#include "haquests/utils/error.hpp"

namespace haquests {
namespace utils {

Error::Error(const std::string& message) 
    : std::runtime_error(message), message_(message), error_code_(0) {}

Error::Error(const std::string& message, int error_code)
    : std::runtime_error(message), message_(message), error_code_(error_code) {}

int Error::getErrorCode() const {
    return error_code_;
}

const char* Error::what() const noexcept {
    return message_.c_str();
}

SocketError::SocketError(const std::string& message)
    : Error("Socket Error: " + message) {}

ConnectionError::ConnectionError(const std::string& message)
    : Error("Connection Error: " + message) {}

TLSError::TLSError(const std::string& message)
    : Error("TLS Error: " + message) {}

HTTPError::HTTPError(const std::string& message)
    : Error("HTTP Error: " + message), status_code_(0) {}

HTTPError::HTTPError(const std::string& message, int status_code)
    : Error("HTTP Error: " + message), status_code_(status_code) {}

int HTTPError::getStatusCode() const {
    return status_code_;
}

ParseError::ParseError(const std::string& message)
    : Error("Parse Error: " + message) {}

} // namespace utils
} // namespace haquests
