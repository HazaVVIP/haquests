#include "haquests/core/socket.hpp"
#include "haquests/utils/error.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/capability.h>
#include <cstring>

namespace haquests {
namespace core {

RawSocket::RawSocket() : sockfd_(-1), is_open_(false) {}

RawSocket::~RawSocket() {
    close();
}

bool RawSocket::open() {
    if (is_open_) {
        return true;
    }
    
    // Create raw socket
    sockfd_ = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd_ < 0) {
        return false;
    }
    
    // Set IP_HDRINCL to tell kernel we provide IP header
    int one = 1;
    if (setsockopt(sockfd_, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        ::close(sockfd_);
        sockfd_ = -1;
        return false;
    }
    
    is_open_ = true;
    return true;
}

void RawSocket::close() {
    if (is_open_ && sockfd_ >= 0) {
        ::close(sockfd_);
        sockfd_ = -1;
        is_open_ = false;
    }
}

ssize_t RawSocket::send(const uint8_t* data, size_t len,
                        const std::string& dst_ip, uint16_t dst_port) {
    if (!is_open_) {
        throw utils::SocketError("Socket not open");
    }
    
    struct sockaddr_in dest;
    std::memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(dst_port);
    
    if (inet_pton(AF_INET, dst_ip.c_str(), &dest.sin_addr) != 1) {
        throw utils::SocketError("Invalid destination IP");
    }
    
    return sendto(sockfd_, data, len, 0,
                  reinterpret_cast<struct sockaddr*>(&dest), sizeof(dest));
}

ssize_t RawSocket::receive(uint8_t* buffer, size_t buffer_size) {
    if (!is_open_) {
        throw utils::SocketError("Socket not open");
    }
    
    struct sockaddr_in src;
    socklen_t src_len = sizeof(src);
    
    return recvfrom(sockfd_, buffer, buffer_size, 0,
                    reinterpret_cast<struct sockaddr*>(&src), &src_len);
}

bool RawSocket::isOpen() const {
    return is_open_;
}

int RawSocket::getFd() const {
    return sockfd_;
}

bool RawSocket::hasCapabilities() {
    // Try to create a raw socket to check capabilities
    int test_sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (test_sock >= 0) {
        ::close(test_sock);
        return true;
    }
    return false;
}

} // namespace core
} // namespace haquests
