#pragma once

#include <openssl/ssl.h>
#include <string>
#include <vector>

namespace haquests {
namespace tls {

class Session {
public:
    Session();
    ~Session();
    
    // Save session to buffer
    std::vector<uint8_t> save() const;
    
    // Load session from buffer
    bool load(const uint8_t* data, size_t len);
    
    // Save to file
    bool saveToFile(const std::string& filename) const;
    
    // Load from file
    bool loadFromFile(const std::string& filename);
    
    // Get session ID
    std::vector<uint8_t> getSessionId() const;
    
    // Check if valid
    bool isValid() const;
    
    // Get SSL_SESSION pointer
    SSL_SESSION* getSession() const;
    
    // Set SSL_SESSION pointer
    void setSession(SSL_SESSION* session);

private:
    SSL_SESSION* session_;
};

} // namespace tls
} // namespace haquests
