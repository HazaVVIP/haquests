# HAQuests Connection Fix - Summary

## Problem Description

The HAQuests library had critical issues with HTTP and TLS connections:

1. **HTTP connections hanging after sending request**: The `simple_http_get` example would connect successfully and send the HTTP request, but then hang indefinitely waiting for a response.

2. **TLS connections failing completely**: The `tls_connection` example would fail to establish a connection to HTTPS servers.

## Root Causes

After analysis, three main issues were identified:

### 1. No Socket Receive Timeout
The raw socket implementation had no receive timeout configured, causing `recvfrom()` to block indefinitely when no packets arrived. This resulted in the application hanging when waiting for server responses.

### 2. No Overall Timeout Limit
The TCP receive logic would retry up to 100 times to filter packets for the correct connection. Combined with indefinite socket blocking, this could theoretically cause very long waits.

### 3. Kernel RST Interference
When using raw sockets to create TCP connections, the Linux kernel doesn't track these connections in its TCP stack. When response packets (SYN-ACK, data) arrive, the kernel sees them as unsolicited and sends RST (reset) packets, terminating the connection.

## Solutions Implemented

### Socket-Level Timeout (src/core/socket.cpp)
- Added 5-second receive timeout using `SO_RCVTIMEO` socket option
- Prevents indefinite blocking on `recvfrom()`
- Returns `-1` with `errno` set to `EAGAIN/EWOULDBLOCK` on timeout

### Application-Level Timeout (src/tcp/connection.cpp)
- Added 30-second overall timeout for receive operations
- Uses `std::chrono` to track elapsed time
- Ensures receive() doesn't wait forever even if retrying

### Improved Data Accumulation (src/tcp/connection.cpp)
- Modified receive() to accumulate data from multiple TCP packets
- Tries up to 10 additional receives after getting first packet
- Handles HTTP responses split across multiple packets

### Firewall Configuration (scripts/setup_firewall.sh)
- Created script to add iptables rule blocking kernel RST packets
- Rule targets source ports 10000-65535 (range used by HAQuests)
- Prevents kernel from interfering with raw socket connections

### TLS Certificate Verification (examples/tls_connection.cpp)
- Disabled certificate verification in example for easier testing
- Can be re-enabled for production use

## How to Use

### 1. Setup (One-time)

```bash
# After building, setup firewall rules (REQUIRED)
sudo ./scripts/setup_firewall.sh
```

This is **critical** - without it, connections will fail due to kernel RST packets.

### 2. Run Examples

```bash
cd build/examples

# HTTP example
sudo ./simple_http_get http://example.com

# TLS example
sudo ./tls_connection www.google.com
```

### 3. Testing

```bash
# Local functionality test (no network required)
./scripts/test_local.sh

# Full connection test (requires network)
sudo ./scripts/test_connections.sh
```

### 4. Cleanup (Optional)

```bash
# Remove firewall rules
sudo ./scripts/cleanup_firewall.sh
```

## Technical Details

### Timeout Constants

All timeout values are now defined as named constants:

- `SOCKET_RECV_TIMEOUT_SECONDS` = 5 (socket-level timeout)
- `RECEIVE_TIMEOUT_SECONDS` = 30 (overall receive timeout)
- `MAX_EXTRA_RECEIVE_ATTEMPTS` = 10 (packet accumulation attempts)
- `MAX_RECEIVE_ATTEMPTS` = 100 (packet filtering attempts)

### Port Range

HAQuests uses source ports in the range **10000-65535** for outgoing connections. This range is:
- Chosen randomly for each connection
- Used in firewall rules to identify HAQuests traffic
- High enough to avoid well-known ports
- Wide enough to support many concurrent connections

### Error Handling

The code now properly handles:
- Socket timeouts (`EAGAIN`, `EWOULDBLOCK`)
- Overall operation timeouts
- Missing/delayed packets
- Connection failures

## Files Changed

- `src/core/socket.cpp` - Added receive timeout configuration
- `src/tcp/connection.cpp` - Improved receive logic with timeouts and accumulation
- `examples/tls_connection.cpp` - Disabled certificate verification
- `scripts/setup_firewall.sh` - New script for firewall setup
- `scripts/cleanup_firewall.sh` - New script for firewall cleanup
- `scripts/test_local.sh` - New local testing script
- `scripts/test_connections.sh` - New network testing script
- `README.md` - Added firewall setup instructions

## Security Summary

**CodeQL Analysis**: No security vulnerabilities detected

The changes introduce proper timeout handling which actually improves security by preventing resource exhaustion from hanging connections.

The firewall rules are limited to the specific port range used by HAQuests and only affect outgoing RST packets, minimizing security impact.

## Known Limitations

1. **External Network Required**: The examples require actual network connectivity to external hosts.

2. **Port Range Assumption**: The firewall rules assume HAQuests uses ports 10000-65535. If this changes, firewall rules must be updated.

3. **Certificate Verification Disabled**: The TLS example has certificate verification disabled for testing. Re-enable for production use.

4. **GitHub HTTP**: GitHub.com doesn't accept HTTP connections on port 80 (redirects to HTTPS), so use HTTPS examples for testing with GitHub.

## Troubleshooting

### Connection still hangs
- Ensure firewall rules are installed: `sudo iptables -L OUTPUT -n -v | grep RST`
- Check network connectivity: `ping <target-host>`
- Verify running with sudo or CAP_NET_RAW capability

### TLS connection fails
- Try with certificate verification disabled (default in example)
- Test with different hosts (www.google.com, github.com)
- Check if port 443 is accessible

### No response received
- Some servers may not respond to raw socket connections
- Try different target servers
- Check server firewall/rate limiting

## Contributing

When modifying timeout or receive logic, ensure:
- Named constants are used (no magic numbers)
- Overall timeout prevents indefinite waiting
- Error handling is comprehensive
- Tests are updated accordingly
