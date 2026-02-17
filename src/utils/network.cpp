#include "haquests/utils/network.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

namespace haquests {
namespace utils {

std::string getLocalIPAddress(const std::string& dest_ip) {
    // Create a UDP socket to determine which interface would be used
    // to reach the destination (without actually sending data)
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return "";
    }

    struct sockaddr_in dest_addr;
    std::memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(80); // Port doesn't matter for this purpose
    
    if (inet_pton(AF_INET, dest_ip.c_str(), &dest_addr.sin_addr) != 1) {
        close(sock);
        return "";
    }

    // Connect to determine the local interface
    if (connect(sock, reinterpret_cast<struct sockaddr*>(&dest_addr), 
                sizeof(dest_addr)) < 0) {
        close(sock);
        return "";
    }

    // Get the local address that would be used
    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    if (getsockname(sock, reinterpret_cast<struct sockaddr*>(&local_addr), 
                     &addr_len) < 0) {
        close(sock);
        return "";
    }

    close(sock);

    char ip_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &local_addr.sin_addr, ip_str, sizeof(ip_str)) == nullptr) {
        return "";
    }

    return std::string(ip_str);
}

bool ipStringToUint32(const std::string& ip_str, uint32_t& ip_out) {
    struct in_addr addr;
    if (inet_pton(AF_INET, ip_str.c_str(), &addr) != 1) {
        return false;
    }
    ip_out = addr.s_addr;
    return true;
}

std::string uint32ToIpString(uint32_t ip) {
    struct in_addr addr;
    addr.s_addr = ip;
    char ip_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &addr, ip_str, sizeof(ip_str)) == nullptr) {
        return "";
    }
    return std::string(ip_str);
}

} // namespace utils
} // namespace haquests
