#include "haquests/tcp/connection.hpp"
#include "haquests/tcp/state_machine.hpp"
#include "haquests/core/packet.hpp"
#include "haquests/core/socket.hpp"
#include "haquests/core/types.hpp"
#include "haquests/utils/error.hpp"
#include "haquests/utils/network.hpp"
#include <random>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <chrono>
#include <thread>
#include <cstring>

namespace haquests {
namespace tcp {

class Connection::Impl {
public:
    Impl() : seq_num_(0), ack_num_(0), src_port_(0), dst_port_(0) {
        // Generate random source port
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint16_t> dis(10000, 65535);
        src_port_ = dis(gen);
    }
    
    bool connect(const std::string& host, uint16_t port) {
        dst_port_ = port;
        
        // Resolve host using getaddrinfo (more modern and robust)
        struct addrinfo hints, *result;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;  // IPv4
        hints.ai_socktype = SOCK_STREAM;
        
        int ret = getaddrinfo(host.c_str(), nullptr, &hints, &result);
        if (ret != 0) {
            return false;
        }
        
        // Get the first result
        struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(result->ai_addr);
        dst_ip_ = std::string(inet_ntoa(addr->sin_addr));
        freeaddrinfo(result);
        
        // Get local IP address that would be used to reach destination
        src_ip_ = utils::getLocalIPAddress(dst_ip_);
        if (src_ip_.empty()) {
            return false;
        }
        
        // Open raw socket
        if (!socket_.open()) {
            return false;
        }
        
        // Perform 3-way handshake
        return performHandshake();
    }
    
    ssize_t send(const uint8_t* data, size_t len) {
        if (state_machine_.getState() != core::TCPState::ESTABLISHED) {
            throw utils::ConnectionError("Not connected");
        }
        
        // Build packet
        core::Packet packet;
        
        uint32_t src_ip_int, dst_ip_int;
        utils::ipStringToUint32(src_ip_, src_ip_int);
        utils::ipStringToUint32(dst_ip_, dst_ip_int);
        
        packet.buildIPHeader(src_ip_int, dst_ip_int,
                            sizeof(core::IPHeader) + sizeof(core::TCPHeader) + len);
        packet.setData(data, len);
        packet.buildTCPHeader(src_port_, dst_port_, seq_num_, ack_num_,
                             core::TCP_FLAG_PSH | core::TCP_FLAG_ACK);
        
        // Serialize and send
        uint8_t buffer[65535];
        size_t packet_size = packet.serialize(buffer, sizeof(buffer));
        
        ssize_t sent = socket_.send(buffer, packet_size, dst_ip_, dst_port_);
        if (sent > 0) {
            seq_num_ += len;
        }
        
        return sent;
    }
    
    std::vector<uint8_t> receive(size_t max_len) {
        // Accumulate data from multiple packets if needed
        std::vector<uint8_t> accumulated_data;
        int attempts = 0;
        
        // Set overall timeout for receiving (30 seconds total)
        const auto timeout_duration = std::chrono::seconds(30);
        auto start_time = std::chrono::steady_clock::now();
        
        // Keep trying to receive packets until we get data or timeout
        while (attempts < MAX_RECEIVE_ATTEMPTS && accumulated_data.empty()) {
            // Check if we've exceeded overall timeout
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            if (elapsed >= timeout_duration) {
                break;
            }
            
            uint8_t buffer[65535];
            ssize_t received = socket_.receive(buffer, sizeof(buffer));
            
            if (received > 0) {
                // Parse packet and extract TCP payload data
                std::vector<uint8_t> data = parsePacketData(buffer, received);
                if (!data.empty()) {
                    accumulated_data.insert(accumulated_data.end(), data.begin(), data.end());
                    
                    // If we have enough data or hit max_len, return
                    if (accumulated_data.size() >= max_len) {
                        accumulated_data.resize(max_len);
                        return accumulated_data;
                    }
                    
                    // Try to receive more packets with data for our connection
                    // This allows accumulating data from multiple packets
                    int extra_attempts = 0;
                    while (extra_attempts < 10 && accumulated_data.size() < max_len) {
                        // Check timeout again
                        elapsed = std::chrono::steady_clock::now() - start_time;
                        if (elapsed >= timeout_duration) {
                            break;
                        }
                        
                        received = socket_.receive(buffer, sizeof(buffer));
                        if (received > 0) {
                            data = parsePacketData(buffer, received);
                            if (!data.empty()) {
                                accumulated_data.insert(accumulated_data.end(), data.begin(), data.end());
                            }
                        } else {
                            // Timeout or error - return what we have
                            break;
                        }
                        extra_attempts++;
                    }
                    
                    return accumulated_data;
                }
                // If empty, it wasn't for our connection, keep trying
            } else if (received < 0) {
                // Timeout or error - if we have any data, return it
                if (!accumulated_data.empty()) {
                    return accumulated_data;
                }
                // Otherwise continue trying (will be limited by overall timeout)
            }
            
            attempts++;
        }
        
        // Return whatever data we accumulated (might be empty)
        return accumulated_data;
    }
    
    void close() {
        if (state_machine_.getState() == core::TCPState::ESTABLISHED) {
            sendFIN();
        }
        socket_.close();
        state_machine_.onClose();
    }
    
    core::TCPState getState() const {
        return state_machine_.getState();
    }
    
    bool isConnected() const {
        return state_machine_.getState() == core::TCPState::ESTABLISHED;
    }

private:
    bool performHandshake() {
        // Generate ISN
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);
        seq_num_ = dis(gen);
        
        // Send SYN
        if (!sendSYN()) {
            return false;
        }
        
        state_machine_.onSendSYN();
        
        // Wait for SYN-ACK with timeout
        if (!waitForSYNACK()) {
            return false;
        }
        
        // Send ACK to complete handshake
        if (!sendACK()) {
            return false;
        }
        
        state_machine_.onEstablished();
        return true;
    }
    
    bool waitForSYNACK() {
        const int timeout_seconds = 5;
        
        auto start_time = std::chrono::steady_clock::now();
        
        while (true) {
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                current_time - start_time).count();
            
            if (elapsed >= timeout_seconds) {
                return false; // Timeout
            }
            
            // Set up select to wait for incoming packets with timeout
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(socket_.getFd(), &read_fds);
            
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 100000; // 100ms timeout
            
            int ready = select(socket_.getFd() + 1, &read_fds, nullptr, nullptr, &tv);
            
            if (ready > 0 && FD_ISSET(socket_.getFd(), &read_fds)) {
                uint8_t buffer[65535];
                ssize_t received = socket_.receive(buffer, sizeof(buffer));
                
                if (received > 0) {
                    // Parse the received packet
                    if (parseSYNACK(buffer, received)) {
                        return true; // Successfully received SYN-ACK
                    }
                }
            }
            // select() already provides timing, no additional sleep needed
        }
        
        return false; // Failed to receive SYN-ACK
    }
    
    bool parseSYNACK(const uint8_t* buffer, size_t len) {
        // Minimum packet size check
        if (len < sizeof(core::IPHeader) + sizeof(core::TCPHeader)) {
            return false;
        }
        
        const core::IPHeader* ip_hdr = reinterpret_cast<const core::IPHeader*>(buffer);
        const core::TCPHeader* tcp_hdr = reinterpret_cast<const core::TCPHeader*>(
            buffer + sizeof(core::IPHeader));
        
        // Verify this is a packet for us
        uint32_t src_ip_int, dst_ip_int;
        utils::ipStringToUint32(src_ip_, src_ip_int);
        utils::ipStringToUint32(dst_ip_, dst_ip_int);
        
        // Check if packet is from destination to us
        if (ip_hdr->src_addr != dst_ip_int || ip_hdr->dst_addr != src_ip_int) {
            return false;
        }
        
        // Check ports
        if (ntohs(tcp_hdr->src_port) != dst_port_ || 
            ntohs(tcp_hdr->dst_port) != src_port_) {
            return false;
        }
        
        // Check if SYN and ACK flags are set
        if ((tcp_hdr->flags & (core::TCP_FLAG_SYN | core::TCP_FLAG_ACK)) != 
            (core::TCP_FLAG_SYN | core::TCP_FLAG_ACK)) {
            return false;
        }
        
        // Verify ACK number matches our SEQ + 1
        uint32_t expected_ack = seq_num_ + 1;
        if (ntohl(tcp_hdr->ack_num) != expected_ack) {
            return false;
        }
        
        // Update our sequence and acknowledgment numbers
        seq_num_ = expected_ack; // Move our SEQ forward
        ack_num_ = ntohl(tcp_hdr->seq_num) + 1; // ACK their SEQ + 1
        
        state_machine_.onReceiveSYNACK();
        return true;
    }
    
    bool sendACK() {
        core::Packet packet;
        
        uint32_t src_ip_int, dst_ip_int;
        utils::ipStringToUint32(src_ip_, src_ip_int);
        utils::ipStringToUint32(dst_ip_, dst_ip_int);
        
        packet.buildIPHeader(src_ip_int, dst_ip_int,
                            sizeof(core::IPHeader) + sizeof(core::TCPHeader));
        packet.buildTCPHeader(src_port_, dst_port_, seq_num_, ack_num_, 
                             core::TCP_FLAG_ACK);
        
        uint8_t buffer[65535];
        size_t packet_size = packet.serialize(buffer, sizeof(buffer));
        
        return socket_.send(buffer, packet_size, dst_ip_, dst_port_) > 0;
    }
    
    bool sendSYN() {
        core::Packet packet;
        
        uint32_t src_ip_int, dst_ip_int;
        utils::ipStringToUint32(src_ip_, src_ip_int);
        utils::ipStringToUint32(dst_ip_, dst_ip_int);
        
        packet.buildIPHeader(src_ip_int, dst_ip_int,
                            sizeof(core::IPHeader) + sizeof(core::TCPHeader));
        packet.buildTCPHeader(src_port_, dst_port_, seq_num_, 0, core::TCP_FLAG_SYN);
        
        uint8_t buffer[65535];
        size_t packet_size = packet.serialize(buffer, sizeof(buffer));
        
        return socket_.send(buffer, packet_size, dst_ip_, dst_port_) > 0;
    }
    
    void sendFIN() {
        core::Packet packet;
        
        uint32_t src_ip_int, dst_ip_int;
        utils::ipStringToUint32(src_ip_, src_ip_int);
        utils::ipStringToUint32(dst_ip_, dst_ip_int);
        
        packet.buildIPHeader(src_ip_int, dst_ip_int,
                            sizeof(core::IPHeader) + sizeof(core::TCPHeader));
        packet.buildTCPHeader(src_port_, dst_port_, seq_num_, ack_num_,
                             core::TCP_FLAG_FIN | core::TCP_FLAG_ACK);
        
        uint8_t buffer[65535];
        size_t packet_size = packet.serialize(buffer, sizeof(buffer));
        
        socket_.send(buffer, packet_size, dst_ip_, dst_port_);
        state_machine_.onSendFIN();
    }
    
    std::vector<uint8_t> parsePacketData(const uint8_t* buffer, size_t len) {
        // Minimum packet size check
        if (len < sizeof(core::IPHeader) + sizeof(core::TCPHeader)) {
            return std::vector<uint8_t>();
        }
        
        const core::IPHeader* ip_hdr = reinterpret_cast<const core::IPHeader*>(buffer);
        
        // Calculate IP header length (IHL is in 32-bit words)
        size_t ip_header_len = (ip_hdr->version_ihl & 0x0F) * 4;
        
        // Check if we have enough data for both headers
        if (len < ip_header_len + sizeof(core::TCPHeader)) {
            return std::vector<uint8_t>();
        }
        
        const core::TCPHeader* tcp_hdr = reinterpret_cast<const core::TCPHeader*>(
            buffer + ip_header_len);
        
        // Verify this packet is for our connection
        uint32_t src_ip_int, dst_ip_int;
        utils::ipStringToUint32(src_ip_, src_ip_int);
        utils::ipStringToUint32(dst_ip_, dst_ip_int);
        
        // Check if packet is from destination to us
        if (ip_hdr->src_addr != dst_ip_int || ip_hdr->dst_addr != src_ip_int) {
            return std::vector<uint8_t>();
        }
        
        // Check ports
        if (ntohs(tcp_hdr->src_port) != dst_port_ || 
            ntohs(tcp_hdr->dst_port) != src_port_) {
            return std::vector<uint8_t>();
        }
        
        // Calculate TCP header length (data_offset is in 32-bit words)
        size_t tcp_header_len = (tcp_hdr->data_offset >> 4) * 4;
        
        // Calculate payload offset and length
        size_t payload_offset = ip_header_len + tcp_header_len;
        
        if (payload_offset >= len) {
            // No payload data
            return std::vector<uint8_t>();
        }
        
        // Extract payload data
        size_t payload_len = len - payload_offset;
        const uint8_t* payload = buffer + payload_offset;
        
        // Update acknowledgment number for received data
        // This indicates we've received data up to (seq_num + payload_len)
        // TCP retransmissions will have the same seq_num, so this correctly
        // sets ack_num_ to the same value, which is the expected behavior
        if (payload_len > 0) {
            ack_num_ = ntohl(tcp_hdr->seq_num) + payload_len;
        }
        
        return std::vector<uint8_t>(payload, payload + payload_len);
    }
    
    StateMachine state_machine_;
    core::RawSocket socket_;
    uint32_t seq_num_;
    uint32_t ack_num_;
    uint16_t src_port_;
    uint16_t dst_port_;
    std::string src_ip_;
    std::string dst_ip_;
    
    // Maximum number of receive attempts before giving up
    // This prevents infinite loops when filtering packets
    // 
    // When using raw sockets, we receive ALL TCP packets on the interface,
    // not just packets for our connection. We need to filter them and retry.
    // In a busy network, we might receive many packets before finding ours.
    // 
    // The value of 100 was chosen as a reasonable balance:
    // - High enough to handle busy networks with many concurrent connections
    // - Low enough to prevent excessive blocking (with typical socket timeouts)
    // - Can be adjusted based on performance requirements
    static constexpr int MAX_RECEIVE_ATTEMPTS = 100;
};

Connection::Connection() : impl_(new Impl()) {}
Connection::~Connection() = default;

bool Connection::connect(const std::string& host, uint16_t port) {
    return impl_->connect(host, port);
}

ssize_t Connection::send(const uint8_t* data, size_t len) {
    return impl_->send(data, len);
}

std::vector<uint8_t> Connection::receive(size_t max_len) {
    return impl_->receive(max_len);
}

void Connection::close() {
    impl_->close();
}

core::TCPState Connection::getState() const {
    return impl_->getState();
}

bool Connection::isConnected() const {
    return impl_->isConnected();
}

void Connection::setReceiveCallback(ReceiveCallback callback) {
    // TODO: Implement callback mechanism
}

} // namespace tcp
} // namespace haquests
