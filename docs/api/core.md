# Core API Documentation

## Overview

The Core module provides low-level packet manipulation and raw socket functionality.

## Classes

### RawSocket

Raw socket wrapper for sending/receiving packets.

```cpp
#include <haquests/core/socket.hpp>

haquests::core::RawSocket socket;
socket.open();
socket.send(data, len, "192.168.1.1", 80);
```

### Packet

Packet builder for IP and TCP headers.

```cpp
#include <haquests/core/packet.hpp>

haquests::core::Packet packet;
packet.buildIPHeader(src_ip, dst_ip, total_len);
packet.buildTCPHeader(src_port, dst_port, seq, ack, flags);
```

## Functions

### checksum()

Calculate IP/TCP checksum.

```cpp
uint16_t checksum(const uint16_t* buffer, size_t size);
```

### tcpChecksum()

Calculate TCP checksum with pseudo-header.

```cpp
uint16_t tcpChecksum(uint32_t src_ip, uint32_t dst_ip,
                     const uint8_t* tcp_segment, size_t tcp_len);
```

## Constants

- `TCP_FLAG_SYN`: SYN flag (0x02)
- `TCP_FLAG_ACK`: ACK flag (0x10)
- `TCP_FLAG_FIN`: FIN flag (0x01)
- `DEFAULT_TTL`: Default TTL value (64)
- `DEFAULT_WINDOW_SIZE`: Default TCP window (65535)
