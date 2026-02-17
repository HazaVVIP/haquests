# Research: TCP State Machine Implementation

## Overview

Implementing a TCP state machine adalah bagian paling kompleks dari proyek ini. TCP memiliki 11 states dan berbagai transitions yang harus ditangani dengan benar.

## TCP State Diagram

```
                              +---------+
                              |  CLOSED |
                              +---------+
                                   |
                         passive OPEN|  active OPEN
                                   v      /
                              +---------+/
                              | LISTEN  |
                              +---------+\
                    rcv SYN       |        \  snd SYN
                       |          v         v
                       |    +---------+  +---------+
                       +--->|SYN_RCVD |  |SYN_SENT |
                            +---------+  +---------+
                             |      |         |
                    rcv ACK  |      |rcv SYN  |rcv SYN/ACK
                             |      |         |
                             v      v         v
                            +---------+  +---------+
                            |ESTABLISHED| ESTABLISHED|
                            +---------+  +---------+
                                 |            |
                          CLOSE  |            | CLOSE
                                 v            v
                            +---------+  +---------+
                            |FIN_WAIT1|  |CLOSE_WAIT
                            +---------+  +---------+
```

## TCP States

### 1. CLOSED
- Initial state, no connection
- Transition: Active open → SYN_SENT

### 2. LISTEN
- Server waiting for connection
- Transition: Receive SYN → SYN_RECEIVED

### 3. SYN_SENT
- Client sent SYN, waiting for SYN-ACK
- Transition: Receive SYN-ACK → ESTABLISHED

### 4. SYN_RECEIVED
- Server received SYN, sent SYN-ACK
- Transition: Receive ACK → ESTABLISHED

### 5. ESTABLISHED
- Connection is open, data transfer
- Transition: Send FIN → FIN_WAIT_1

### 6-11. Closing States
Various states untuk graceful connection termination.

## Simplified State Machine (untuk MVP)

Untuk MVP, kita fokus pada happy path:

```cpp
enum TCPState {
    CLOSED,
    SYN_SENT,
    ESTABLISHED,
    FIN_WAIT_1,
    FIN_WAIT_2,
    TIME_WAIT,
    CLOSE_WAIT,
    LAST_ACK
};

class TCPConnection {
private:
    TCPState state;
    uint32_t seq_num;      // Our sequence number
    uint32_t ack_num;      // Their sequence number
    uint16_t window_size;
    
public:
    // State transitions
    void sendSYN();
    void receiveSYNACK();
    void sendACK();
    void sendData(const char* data, size_t len);
    void close();
};
```

## Sequence Number Management

### Initial Sequence Number (ISN)
```cpp
uint32_t generateISN() {
    // Recommendation: Use random ISN (RFC 6528)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);
    return dis(gen);
}
```

### Sequence Number Tracking
```cpp
class SequenceTracker {
private:
    uint32_t send_next;     // Next seq to send (SND.NXT)
    uint32_t send_una;      // Oldest unacknowledged (SND.UNA)
    uint32_t recv_next;     // Next seq expected (RCV.NXT)
    
public:
    void onSend(size_t data_len) {
        send_next += data_len;
    }
    
    void onAck(uint32_t ack) {
        if (ack > send_una && ack <= send_next) {
            send_una = ack;
        }
    }
    
    bool isValidSeq(uint32_t seq) {
        // Check if received seq is valid
        return seq == recv_next;
    }
};
```

## Window Management

### Send Window
```cpp
class SendWindow {
private:
    uint32_t window_size;   // Receiver's advertised window
    uint32_t congestion_window;  // Congestion control
    
public:
    size_t availableSpace() {
        return std::min(window_size, congestion_window) - inflight_bytes;
    }
    
    void updateWindow(uint16_t new_size) {
        window_size = new_size;
    }
};
```

### Receive Window
```cpp
class ReceiveWindow {
private:
    static const size_t BUFFER_SIZE = 65535;
    char buffer[BUFFER_SIZE];
    size_t used;
    
public:
    uint16_t getAdvertisedWindow() {
        return BUFFER_SIZE - used;
    }
};
```

## 3-Way Handshake Implementation

### Client Side
```cpp
void TCPClient::connect(const string& host, uint16_t port) {
    // 1. Send SYN
    state = SYN_SENT;
    seq_num = generateISN();
    sendPacket(SYN_FLAG, seq_num, 0, nullptr, 0);
    
    // 2. Wait for SYN-ACK
    TCPPacket response = receivePacket();
    if (response.flags & (SYN_FLAG | ACK_FLAG)) {
        ack_num = response.seq + 1;
        seq_num++;
        
        // 3. Send ACK
        state = ESTABLISHED;
        sendPacket(ACK_FLAG, seq_num, ack_num, nullptr, 0);
    }
}
```

### Server Side (if needed)
```cpp
void TCPServer::listen(uint16_t port) {
    state = LISTEN;
    
    while (true) {
        TCPPacket syn = receivePacket();
        if (syn.flags & SYN_FLAG) {
            // 1. Received SYN
            state = SYN_RECEIVED;
            ack_num = syn.seq + 1;
            seq_num = generateISN();
            
            // 2. Send SYN-ACK
            sendPacket(SYN_FLAG | ACK_FLAG, seq_num, ack_num, nullptr, 0);
            
            // 3. Wait for ACK
            TCPPacket ack = receivePacket();
            if (ack.flags & ACK_FLAG) {
                state = ESTABLISHED;
                seq_num++;
            }
        }
    }
}
```

## Data Transfer

### Sending Data
```cpp
void TCPConnection::send(const char* data, size_t len) {
    if (state != ESTABLISHED) {
        throw std::runtime_error("Not connected");
    }
    
    // Fragment if necessary
    size_t offset = 0;
    while (offset < len) {
        size_t chunk = std::min(len - offset, MTU - IP_TCP_HEADER_SIZE);
        
        sendPacket(PSH_FLAG | ACK_FLAG, 
                   seq_num, 
                   ack_num, 
                   data + offset, 
                   chunk);
        
        seq_num += chunk;
        offset += chunk;
        
        // Wait for ACK (simplified - no timeout/retransmission yet)
        TCPPacket ack = receivePacket();
        if (!(ack.flags & ACK_FLAG) || ack.ack != seq_num) {
            // Handle retransmission
        }
    }
}
```

### Receiving Data
```cpp
vector<char> TCPConnection::receive() {
    vector<char> data;
    
    while (true) {
        TCPPacket packet = receivePacket();
        
        // Check sequence number
        if (packet.seq != ack_num) {
            // Out of order - handle reordering or drop
            continue;
        }
        
        // Add data
        data.insert(data.end(), 
                    packet.data, 
                    packet.data + packet.data_len);
        
        // Update ACK
        ack_num += packet.data_len;
        sendPacket(ACK_FLAG, seq_num, ack_num, nullptr, 0);
        
        // Check for FIN or PSH
        if (packet.flags & FIN_FLAG) {
            break;
        }
    }
    
    return data;
}
```

## Retransmission Timer

### Simplified Timeout
```cpp
class RetransmissionTimer {
private:
    std::chrono::milliseconds rto;  // Retransmission timeout
    
public:
    RetransmissionTimer() : rto(1000) {}  // Start with 1 second
    
    void sendWithRetry(function<void()> sendFunc, 
                       function<bool()> checkAck) {
        int attempts = 0;
        while (attempts < MAX_RETRIES) {
            sendFunc();
            
            auto start = chrono::steady_clock::now();
            while (chrono::steady_clock::now() - start < rto) {
                if (checkAck()) {
                    return;  // ACK received
                }
                this_thread::sleep_for(chrono::milliseconds(10));
            }
            
            attempts++;
            rto *= 2;  // Exponential backoff
        }
        throw std::runtime_error("Retransmission failed");
    }
};
```

### Advanced: RTT Estimation (Jacobson/Karels Algorithm)
```cpp
class RTTEstimator {
private:
    double srtt;    // Smoothed RTT
    double rttvar;  // RTT variance
    
public:
    void updateRTT(double measured_rtt) {
        if (srtt == 0) {
            srtt = measured_rtt;
            rttvar = measured_rtt / 2;
        } else {
            double alpha = 0.125;
            double beta = 0.25;
            rttvar = (1 - beta) * rttvar + beta * abs(srtt - measured_rtt);
            srtt = (1 - alpha) * srtt + alpha * measured_rtt;
        }
    }
    
    double getRTO() {
        return srtt + 4 * rttvar;
    }
};
```

## Connection Termination

### Active Close (Client)
```cpp
void TCPConnection::close() {
    // 1. Send FIN
    state = FIN_WAIT_1;
    sendPacket(FIN_FLAG | ACK_FLAG, seq_num, ack_num, nullptr, 0);
    seq_num++;
    
    // 2. Wait for ACK
    TCPPacket ack = receivePacket();
    if (ack.flags & ACK_FLAG) {
        state = FIN_WAIT_2;
    }
    
    // 3. Wait for FIN
    TCPPacket fin = receivePacket();
    if (fin.flags & FIN_FLAG) {
        ack_num++;
        
        // 4. Send final ACK
        sendPacket(ACK_FLAG, seq_num, ack_num, nullptr, 0);
        state = TIME_WAIT;
        
        // 5. Wait 2*MSL then close
        this_thread::sleep_for(chrono::seconds(60));
        state = CLOSED;
    }
}
```

## Congestion Control (Optional for MVP)

### Slow Start
```cpp
class CongestionControl {
private:
    size_t cwnd;        // Congestion window
    size_t ssthresh;    // Slow start threshold
    
public:
    void onAck(size_t acked_bytes) {
        if (cwnd < ssthresh) {
            // Slow start: exponential growth
            cwnd += acked_bytes;
        } else {
            // Congestion avoidance: linear growth
            cwnd += (acked_bytes * acked_bytes) / cwnd;
        }
    }
    
    void onTimeout() {
        ssthresh = cwnd / 2;
        cwnd = INITIAL_CWND;
    }
};
```

## State Machine Testing

### Test Cases
1. **Normal Connection**: SYN → SYN-ACK → ACK
2. **Simultaneous Open**: Both sides send SYN
3. **Connection Refused**: Send RST instead of SYN-ACK
4. **Data Transfer**: Send data, verify ACKs
5. **Retransmission**: Drop ACK, verify retransmit
6. **Graceful Close**: FIN handshake
7. **Abrupt Close**: RST

### Test Framework
```cpp
class TCPStateMachineTester {
public:
    void testHandshake() {
        TCPConnection conn;
        assert(conn.getState() == CLOSED);
        
        conn.sendSYN();
        assert(conn.getState() == SYN_SENT);
        
        conn.receiveSYNACK();
        assert(conn.getState() == ESTABLISHED);
    }
    
    void testDataTransfer() {
        // Setup connection
        // Send data
        // Verify sequence numbers
        // Verify data integrity
    }
};
```

## Minimal Implementation Strategy

### Phase 1: Happy Path Only
- Implement only CLOSED → SYN_SENT → ESTABLISHED
- No retransmissions
- No congestion control
- Fixed window size

### Phase 2: Basic Reliability
- Add retransmission timer
- Handle out-of-order packets (buffer or drop)
- Implement proper close

### Phase 3: Full State Machine
- All states
- Congestion control
- Flow control
- Advanced features

## Common Issues dan Solutions

### Issue 1: Sequence Number Wrap-Around
```cpp
bool seqLessThan(uint32_t a, uint32_t b) {
    return (int32_t)(a - b) < 0;  // Handles wrap-around
}
```

### Issue 2: Duplicate ACKs
```cpp
void handleDuplicateACK(uint32_t ack) {
    dup_ack_count++;
    if (dup_ack_count == 3) {
        // Fast retransmit
        retransmit(ack);
    }
}
```

### Issue 3: Keep-Alive
```cpp
void sendKeepAlive() {
    // Send packet with seq = SND.NXT - 1
    sendPacket(ACK_FLAG, seq_num - 1, ack_num, nullptr, 0);
}
```

## References

1. **RFC 793**: TCP Protocol Specification
2. **RFC 6298**: Computing TCP's Retransmission Timer
3. **RFC 5681**: TCP Congestion Control
4. **TCP/IP Illustrated Vol. 1**: Stevens
5. **Linux TCP Implementation**: Source code study

## Next Steps

1. Implement basic state enum dan transitions
2. Create sequence number tracker
3. Implement 3-way handshake (client side)
4. Test dengan wireshark
5. Add data transfer capability
