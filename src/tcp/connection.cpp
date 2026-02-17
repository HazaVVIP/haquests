#include "haquests/tcp/connection.hpp"
#include "haquests/tcp/state_machine.hpp"
#include "haquests/core/packet.hpp"
#include "haquests/core/socket.hpp"
#include "haquests/core/types.hpp"
#include "haquests/utils/error.hpp"
#include <random>
#include <arpa/inet.h>
#include <netdb.h>

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
        
        // Resolve host
        struct hostent* he = gethostbyname(host.c_str());
        if (!he) {
            return false;
        }
        
        dst_ip_ = std::string(inet_ntoa(*reinterpret_cast<struct in_addr*>(he->h_addr)));
        
        // Get local IP (simplified - use first available)
        src_ip_ = "127.0.0.1";  // TODO: Get actual local IP
        
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
        inet_pton(AF_INET, src_ip_.c_str(), &src_ip_int);
        inet_pton(AF_INET, dst_ip_.c_str(), &dst_ip_int);
        
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
        uint8_t buffer[65535];
        ssize_t received = socket_.receive(buffer, sizeof(buffer));
        
        if (received > 0) {
            // Parse packet and extract data
            // Simplified: return all data
            return std::vector<uint8_t>(buffer, buffer + received);
        }
        
        return std::vector<uint8_t>();
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
        
        // Wait for SYN-ACK (simplified - no timeout handling)
        // TODO: Implement proper SYN-ACK reception
        
        // Send ACK
        // state_machine_.onReceiveSYNACK();
        // sendACK();
        
        // For now, mark as established (simplified)
        state_machine_.onEstablished();
        return true;
    }
    
    bool sendSYN() {
        core::Packet packet;
        
        uint32_t src_ip_int, dst_ip_int;
        inet_pton(AF_INET, src_ip_.c_str(), &src_ip_int);
        inet_pton(AF_INET, dst_ip_.c_str(), &dst_ip_int);
        
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
        inet_pton(AF_INET, src_ip_.c_str(), &src_ip_int);
        inet_pton(AF_INET, dst_ip_.c_str(), &dst_ip_int);
        
        packet.buildIPHeader(src_ip_int, dst_ip_int,
                            sizeof(core::IPHeader) + sizeof(core::TCPHeader));
        packet.buildTCPHeader(src_port_, dst_port_, seq_num_, ack_num_,
                             core::TCP_FLAG_FIN | core::TCP_FLAG_ACK);
        
        uint8_t buffer[65535];
        size_t packet_size = packet.serialize(buffer, sizeof(buffer));
        
        socket_.send(buffer, packet_size, dst_ip_, dst_port_);
        state_machine_.onSendFIN();
    }
    
    StateMachine state_machine_;
    core::RawSocket socket_;
    uint32_t seq_num_;
    uint32_t ack_num_;
    uint16_t src_port_;
    uint16_t dst_port_;
    std::string src_ip_;
    std::string dst_ip_;
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
