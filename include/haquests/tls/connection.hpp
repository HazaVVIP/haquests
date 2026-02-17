#pragma once

#include "haquests/tcp/connection.hpp"
#include <memory>
#include <string>
#include <vector>

namespace haquests {
namespace tls {

class Connection {
public:
    Connection();
    ~Connection();

    // Connect with TLS
    bool connect(const std::string& host, uint16_t port);
    
    // Send data over TLS
    ssize_t send(const uint8_t* data, size_t len);
    
    // Receive data over TLS
    std::vector<uint8_t> receive(size_t max_len = 4096);
    
    // Close TLS connection
    void close();
    
    // Check if connected
    bool isConnected() const;
    
    // Get TLS version
    std::string getTLSVersion() const;
    
    // Get cipher suite
    std::string getCipherSuite() const;
    
    // Enable/disable certificate verification
    void setVerifyCertificate(bool verify);
    
    // Set custom CA certificate
    void setCAFile(const std::string& ca_file);
    void setCAPath(const std::string& ca_path);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace tls
} // namespace haquests
