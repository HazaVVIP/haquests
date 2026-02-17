#pragma once

#include <string>
#include <cstdint>

namespace haquests {
namespace utils {

/**
 * Get the local IP address of the network interface that would be used
 * to connect to the specified destination IP address.
 * 
 * @param dest_ip Destination IP address to connect to
 * @return Local IP address as string, or empty string on error
 */
std::string getLocalIPAddress(const std::string& dest_ip);

/**
 * Convert IP address string to uint32_t in network byte order
 * 
 * @param ip_str IP address as string (e.g., "192.168.1.1")
 * @param ip_out Output parameter for the IP address
 * @return true if conversion successful, false otherwise
 */
bool ipStringToUint32(const std::string& ip_str, uint32_t& ip_out);

/**
 * Convert uint32_t IP address in network byte order to string
 * 
 * @param ip IP address as uint32_t
 * @return IP address as string
 */
std::string uint32ToIpString(uint32_t ip);

} // namespace utils
} // namespace haquests
