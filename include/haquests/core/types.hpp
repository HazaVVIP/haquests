#pragma once

#include <cstdint>

namespace haquests {
namespace core {

// IP version
constexpr uint8_t IP_VERSION_4 = 4;

// TCP flags
constexpr uint8_t TCP_FLAG_FIN = 0x01;
constexpr uint8_t TCP_FLAG_SYN = 0x02;
constexpr uint8_t TCP_FLAG_RST = 0x04;
constexpr uint8_t TCP_FLAG_PSH = 0x08;
constexpr uint8_t TCP_FLAG_ACK = 0x10;
constexpr uint8_t TCP_FLAG_URG = 0x20;

// IP protocol numbers
constexpr uint8_t IPPROTO_ICMP = 1;
constexpr uint8_t IPPROTO_TCP_NUM = 6;
constexpr uint8_t IPPROTO_UDP = 17;

// Default values
constexpr uint8_t DEFAULT_TTL = 64;
constexpr uint16_t DEFAULT_WINDOW_SIZE = 65535;
constexpr size_t MTU = 1500;
constexpr size_t IP_HEADER_SIZE = 20;
constexpr size_t TCP_HEADER_SIZE = 20;
constexpr size_t MAX_PACKET_SIZE = 65535;

// TCP states
enum class TCPState {
    CLOSED,
    LISTEN,
    SYN_SENT,
    SYN_RECEIVED,
    ESTABLISHED,
    FIN_WAIT_1,
    FIN_WAIT_2,
    CLOSE_WAIT,
    CLOSING,
    LAST_ACK,
    TIME_WAIT
};

} // namespace core
} // namespace haquests
