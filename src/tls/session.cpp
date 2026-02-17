#include "haquests/tls/session.hpp"
#include <fstream>
#include <cstring>

namespace haquests {
namespace tls {

Session::Session() : session_(nullptr) {}

Session::~Session() {
    if (session_) {
        SSL_SESSION_free(session_);
    }
}

std::vector<uint8_t> Session::save() const {
    if (!session_) {
        return std::vector<uint8_t>();
    }
    
    unsigned char* data = nullptr;
    int len = i2d_SSL_SESSION(session_, &data);
    
    if (len <= 0) {
        return std::vector<uint8_t>();
    }
    
    std::vector<uint8_t> result(data, data + len);
    OPENSSL_free(data);
    
    return result;
}

bool Session::load(const uint8_t* data, size_t len) {
    if (session_) {
        SSL_SESSION_free(session_);
        session_ = nullptr;
    }
    
    const unsigned char* p = data;
    session_ = d2i_SSL_SESSION(nullptr, &p, len);
    
    return session_ != nullptr;
}

bool Session::saveToFile(const std::string& filename) const {
    std::vector<uint8_t> data = save();
    if (data.empty()) {
        return false;
    }
    
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return file.good();
}

bool Session::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }
    
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        return false;
    }
    
    return load(data.data(), data.size());
}

std::vector<uint8_t> Session::getSessionId() const {
    if (!session_) {
        return std::vector<uint8_t>();
    }
    
    unsigned int len = 0;
    const unsigned char* id = SSL_SESSION_get_id(session_, &len);
    
    return std::vector<uint8_t>(id, id + len);
}

bool Session::isValid() const {
    return session_ != nullptr;
}

SSL_SESSION* Session::getSession() const {
    return session_;
}

void Session::setSession(SSL_SESSION* session) {
    if (session_) {
        SSL_SESSION_free(session_);
    }
    session_ = session;
}

} // namespace tls
} // namespace haquests
