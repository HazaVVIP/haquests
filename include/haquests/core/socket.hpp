#pragma once

#include <string>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>

namespace haquests {
namespace core {

class RawSocket {
public:
    RawSocket();
    ~RawSocket();

    // Disable copy
    RawSocket(const RawSocket&) = delete;
    RawSocket& operator=(const RawSocket&) = delete;

    // Open raw socket
    bool open();
    
    // Close socket
    void close();
    
    // Send packet
    ssize_t send(const uint8_t* data, size_t len, 
                 const std::string& dst_ip, uint16_t dst_port);
    
    // Receive packet
    ssize_t receive(uint8_t* buffer, size_t buffer_size);
    
    // Check if socket is open
    bool isOpen() const;
    
    // Get socket file descriptor
    int getFd() const;
    
    // Check if we have required capabilities
    static bool hasCapabilities();

private:
    int sockfd_;
    bool is_open_;
};

} // namespace core
} // namespace haquests
