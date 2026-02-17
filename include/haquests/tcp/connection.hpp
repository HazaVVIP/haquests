#pragma once

#include "haquests/core/types.hpp"
#include "haquests/core/packet.hpp"
#include "haquests/core/socket.hpp"
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace haquests {
namespace tcp {

// TCP Connection implementation using raw sockets
// 
// Note: This class is NOT thread-safe. Do not call methods on the same
// Connection object concurrently from multiple threads.
class Connection {
public:
    Connection();
    ~Connection();

    // Connect to remote host
    bool connect(const std::string& host, uint16_t port);
    
    // Send data
    ssize_t send(const uint8_t* data, size_t len);
    
    // Receive data
    std::vector<uint8_t> receive(size_t max_len = 4096);
    
    // Close connection
    void close();
    
    // Get current state
    core::TCPState getState() const;
    
    // Check if connected
    bool isConnected() const;
    
    // Set receive callback
    using ReceiveCallback = std::function<void(const uint8_t*, size_t)>;
    void setReceiveCallback(ReceiveCallback callback);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace tcp
} // namespace haquests
