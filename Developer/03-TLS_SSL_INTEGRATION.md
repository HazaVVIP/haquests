# Research: TLS/SSL Integration dengan Raw Socket

## Overview Challenge

Mengintegrasikan TLS dengan raw socket adalah challenge terbesar dalam proyek ini karena:
1. OpenSSL/BoringSSL expect standard socket file descriptors
2. Kita perlu bridge antara raw packet handling dan TLS state machine
3. TLS handshake sangat complex dan error-prone

## OpenSSL BIO (Basic I/O) Abstraction

OpenSSL menggunakan BIO abstraction untuk I/O operations. Kita bisa membuat custom BIO yang interface dengan raw socket kita.

### BIO Architecture

```
┌─────────────┐
│   OpenSSL   │
│  TLS Stack  │
└─────┬───────┘
      │
┌─────▼───────┐
│  Custom BIO │  <-- We implement this
└─────┬───────┘
      │
┌─────▼───────┐
│  Raw Socket │
│   Handler   │
└─────────────┘
```

## Custom BIO Implementation

### BIO Method Structure
```cpp
static BIO_METHOD* create_raw_socket_bio_method() {
    BIO_METHOD* method = BIO_meth_new(BIO_TYPE_SOURCE_SINK, "raw_socket");
    
    BIO_meth_set_write(method, raw_socket_bio_write);
    BIO_meth_set_read(method, raw_socket_bio_read);
    BIO_meth_set_puts(method, raw_socket_bio_puts);
    BIO_meth_set_gets(method, raw_socket_bio_gets);
    BIO_meth_set_ctrl(method, raw_socket_bio_ctrl);
    BIO_meth_set_create(method, raw_socket_bio_create);
    BIO_meth_set_destroy(method, raw_socket_bio_destroy);
    
    return method;
}
```

### Custom BIO Callbacks
```cpp
// Write callback - called when SSL wants to send data
static int raw_socket_bio_write(BIO* bio, const char* data, int len) {
    RawSocketConnection* conn = (RawSocketConnection*)BIO_get_data(bio);
    
    // Send via our raw TCP implementation
    try {
        conn->send(data, len);
        return len;
    } catch (const std::exception& e) {
        return -1;
    }
}

// Read callback - called when SSL wants to receive data
static int raw_socket_bio_read(BIO* bio, char* buffer, int len) {
    RawSocketConnection* conn = (RawSocketConnection*)BIO_get_data(bio);
    
    // Receive via our raw TCP implementation
    try {
        size_t received = conn->receive(buffer, len);
        return received;
    } catch (const std::exception& e) {
        return -1;
    }
}

// Control callback - various control operations
static long raw_socket_bio_ctrl(BIO* bio, int cmd, long num, void* ptr) {
    switch (cmd) {
        case BIO_CTRL_FLUSH:
            return 1;  // Success
        case BIO_CTRL_PUSH:
        case BIO_CTRL_POP:
            return 0;
        default:
            return 0;
    }
}
```

## TLS Connection Setup

### Client-Side TLS Handshake
```cpp
class TLSRawConnection {
private:
    RawSocketConnection* raw_conn;
    SSL_CTX* ssl_ctx;
    SSL* ssl;
    
public:
    TLSRawConnection(const string& host, uint16_t port) {
        // 1. Establish TCP connection via raw socket
        raw_conn = new RawSocketConnection(host, port);
        raw_conn->connect();
        
        // 2. Initialize SSL context
        ssl_ctx = SSL_CTX_new(TLS_client_method());
        SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, nullptr);
        SSL_CTX_set_default_verify_paths(ssl_ctx);
        
        // 3. Create SSL object
        ssl = SSL_new(ssl_ctx);
        
        // 4. Create custom BIO and attach to SSL
        BIO* bio = BIO_new(create_raw_socket_bio_method());
        BIO_set_data(bio, raw_conn);
        SSL_set_bio(ssl, bio, bio);
        
        // 5. Set SNI (Server Name Indication)
        SSL_set_tlsext_host_name(ssl, host.c_str());
        
        // 6. Perform TLS handshake
        if (SSL_connect(ssl) != 1) {
            throw std::runtime_error("TLS handshake failed");
        }
    }
    
    void send(const char* data, size_t len) {
        int written = SSL_write(ssl, data, len);
        if (written <= 0) {
            int err = SSL_get_error(ssl, written);
            throw std::runtime_error("SSL_write failed: " + std::to_string(err));
        }
    }
    
    vector<char> receive(size_t max_len) {
        vector<char> buffer(max_len);
        int read = SSL_read(ssl, buffer.data(), max_len);
        if (read <= 0) {
            int err = SSL_get_error(ssl, read);
            throw std::runtime_error("SSL_read failed: " + std::to_string(err));
        }
        buffer.resize(read);
        return buffer;
    }
};
```

## TLS Handshake Flow

### Normal TLS 1.3 Handshake
```
Client                                           Server

ClientHello          -------->
                                             ServerHello
                                    {EncryptedExtensions}
                                   {CertificateRequest*}
                                          {Certificate*}
                                    {CertificateVerify*}
                                              {Finished}
                     <--------       [Application Data*]
{Certificate*}
{CertificateVerify*}
{Finished}           -------->
[Application Data]   <------->       [Application Data]
```

Setiap message di-handle oleh OpenSSL, tapi transport via custom BIO kita.

## Certificate Verification

### Basic Verification
```cpp
void setupCertificateVerification(SSL_CTX* ctx) {
    // Load system CA certificates
    SSL_CTX_set_default_verify_paths(ctx);
    
    // Set verification mode
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);
}

int verify_callback(int preverify_ok, X509_STORE_CTX* ctx) {
    if (!preverify_ok) {
        int err = X509_STORE_CTX_get_error(ctx);
        const char* err_str = X509_verify_cert_error_string(err);
        fprintf(stderr, "Certificate verification failed: %s\n", err_str);
    }
    return preverify_ok;
}
```

### Custom Verification (untuk testing)
```cpp
// Option to disable verification (DANGEROUS - only for testing!)
void disableVerification(SSL_CTX* ctx) {
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);
}

// Custom verification logic
int custom_verify(int preverify_ok, X509_STORE_CTX* ctx) {
    // Custom certificate pinning
    X509* cert = X509_STORE_CTX_get_current_cert(ctx);
    
    // Check certificate fingerprint
    unsigned char fingerprint[EVP_MAX_MD_SIZE];
    unsigned int fingerprint_len;
    X509_digest(cert, EVP_sha256(), fingerprint, &fingerprint_len);
    
    // Compare with expected fingerprint
    // ...
    
    return 1;  // Accept
}
```

## ALPN (Application-Layer Protocol Negotiation)

### Setting ALPN for HTTP/1.1 or HTTP/2
```cpp
void setALPN(SSL* ssl, bool prefer_h2) {
    if (prefer_h2) {
        // Prefer HTTP/2
        unsigned char alpn[] = "\x02h2\x08http/1.1";
        SSL_set_alpn_protos(ssl, alpn, sizeof(alpn) - 1);
    } else {
        // HTTP/1.1 only
        unsigned char alpn[] = "\x08http/1.1";
        SSL_set_alpn_protos(ssl, alpn, sizeof(alpn) - 1);
    }
}

// Check negotiated protocol
void checkNegotiatedProtocol(SSL* ssl) {
    const unsigned char* alpn_data;
    unsigned int alpn_len;
    SSL_get0_alpn_selected(ssl, &alpn_data, &alpn_len);
    
    if (alpn_len > 0) {
        string protocol(reinterpret_cast<const char*>(alpn_data), alpn_len);
        cout << "Negotiated protocol: " << protocol << endl;
    }
}
```

## Session Resumption

### Session Caching
```cpp
class TLSSessionCache {
private:
    map<string, SSL_SESSION*> cache;
    
public:
    void saveSession(const string& host, SSL* ssl) {
        SSL_SESSION* session = SSL_get1_session(ssl);
        if (session) {
            cache[host] = session;
        }
    }
    
    bool restoreSession(const string& host, SSL* ssl) {
        auto it = cache.find(host);
        if (it != cache.end()) {
            SSL_set_session(ssl, it->second);
            return true;
        }
        return false;
    }
};
```

## Error Handling

### SSL Error Codes
```cpp
void handleSSLError(SSL* ssl, int ret) {
    int err = SSL_get_error(ssl, ret);
    
    switch (err) {
        case SSL_ERROR_NONE:
            // Success
            break;
            
        case SSL_ERROR_WANT_READ:
            // Need more data - retry read
            // This should not happen with blocking I/O
            break;
            
        case SSL_ERROR_WANT_WRITE:
            // Need to write - retry
            break;
            
        case SSL_ERROR_SYSCALL:
            // System call error
            perror("SSL syscall error");
            break;
            
        case SSL_ERROR_SSL:
            // SSL protocol error
            ERR_print_errors_fp(stderr);
            break;
            
        default:
            fprintf(stderr, "Unknown SSL error: %d\n", err);
    }
}
```

## Buffering Strategy

### Read Buffer
Raw socket receive adalah asynchronous dan bisa return partial data. TLS needs complete records.

```cpp
class TLSReadBuffer {
private:
    vector<char> buffer;
    size_t offset;
    
public:
    int fillBuffer(RawSocketConnection* conn, size_t min_bytes) {
        while (buffer.size() - offset < min_bytes) {
            char temp[4096];
            size_t received = conn->receive(temp, sizeof(temp));
            if (received == 0) {
                return 0;  // Connection closed
            }
            buffer.insert(buffer.end(), temp, temp + received);
        }
        return buffer.size() - offset;
    }
    
    int read(char* out, size_t len) {
        size_t available = buffer.size() - offset;
        size_t to_read = std::min(len, available);
        
        memcpy(out, buffer.data() + offset, to_read);
        offset += to_read;
        
        // Compact buffer if offset is large
        if (offset > 8192 && offset > buffer.size() / 2) {
            buffer.erase(buffer.begin(), buffer.begin() + offset);
            offset = 0;
        }
        
        return to_read;
    }
};
```

## Non-Blocking I/O Consideration

### BIO dengan Non-Blocking Mode
```cpp
static int raw_socket_bio_read_nonblocking(BIO* bio, char* buffer, int len) {
    RawSocketConnection* conn = (RawSocketConnection*)BIO_get_data(bio);
    
    // Try to read without blocking
    ssize_t received = conn->tryReceive(buffer, len);
    
    if (received < 0) {
        // Would block
        BIO_set_retry_read(bio);
        return -1;
    }
    
    return received;
}
```

## Testing Strategy

### Test Cases
1. **Basic TLS Connection**: Connect to google.com:443
2. **Certificate Verification**: Test with valid and invalid certs
3. **SNI**: Verify SNI is sent correctly
4. **Session Resumption**: Connect twice, verify session reuse
5. **ALPN**: Verify protocol negotiation
6. **Error Handling**: Test with malformed data

### Example Test
```cpp
void testGoogleTLS() {
    try {
        TLSRawConnection conn("www.google.com", 443);
        
        // Send HTTP request
        string request = "GET / HTTP/1.1\r\n"
                        "Host: www.google.com\r\n"
                        "Connection: close\r\n"
                        "\r\n";
        conn.send(request.c_str(), request.length());
        
        // Read response
        auto response = conn.receive(4096);
        cout << string(response.begin(), response.end()) << endl;
        
    } catch (const exception& e) {
        cerr << "Test failed: " << e.what() << endl;
    }
}
```

## Alternative: BoringSSL

BoringSSL (Google's fork) mungkin lebih mudah untuk integrate:
```cpp
// BoringSSL has cleaner API for custom transports
bssl::UniquePtr<SSL_CTX> ctx(SSL_CTX_new(TLS_method()));
bssl::UniquePtr<SSL> ssl(SSL_new(ctx.get()));

// Set custom I/O callbacks
SSL_set_custom_verify(ssl.get(), SSL_VERIFY_PEER, verify_callback);
```

## Performance Considerations

### Zero-Copy TLS
- Investigate `SSL_write_ex` for better performance
- Use scatter-gather I/O where possible
- Minimize copies between BIO and raw socket

### Connection Pooling
```cpp
class TLSConnectionPool {
private:
    map<string, vector<TLSRawConnection*>> pool;
    
public:
    TLSRawConnection* getConnection(const string& host, uint16_t port) {
        string key = host + ":" + to_string(port);
        
        if (!pool[key].empty()) {
            auto conn = pool[key].back();
            pool[key].pop_back();
            return conn;
        }
        
        return new TLSRawConnection(host, port);
    }
    
    void releaseConnection(const string& host, uint16_t port, TLSRawConnection* conn) {
        string key = host + ":" + to_string(port);
        pool[key].push_back(conn);
    }
};
```

## Security Considerations

### Key Points
1. **Always verify certificates** in production
2. **Use strong cipher suites**: Disable weak algorithms
3. **Keep OpenSSL updated**: Security patches are critical
4. **Protect session keys**: Don't log sensitive data
5. **Implement proper timeout**: Prevent resource exhaustion

### Cipher Suite Selection
```cpp
void setSecureCiphers(SSL_CTX* ctx) {
    // Use only strong ciphers
    SSL_CTX_set_cipher_list(ctx, 
        "ECDHE-ECDSA-AES128-GCM-SHA256:"
        "ECDHE-RSA-AES128-GCM-SHA256:"
        "ECDHE-ECDSA-AES256-GCM-SHA384:"
        "ECDHE-RSA-AES256-GCM-SHA384");
    
    // Disable old protocols
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
}
```

## Implementation Roadmap

### Phase 1: Basic BIO
- [ ] Implement custom BIO methods
- [ ] Test with simple SSL connection
- [ ] Verify data flow

### Phase 2: Full Integration
- [ ] Integrate with TCP state machine
- [ ] Add buffering layer
- [ ] Handle all error cases

### Phase 3: Advanced Features
- [ ] Session resumption
- [ ] ALPN support
- [ ] Performance optimization

## References

1. **OpenSSL Documentation**: BIO programming
2. **OpenSSL Cookbook**: Practical examples
3. **RFC 8446**: TLS 1.3 specification
4. **BoringSSL Examples**: Custom transport implementation

## Next Steps

1. Prototype basic BIO implementation
2. Test with loopback connection
3. Integrate with TCP layer
4. Test with real HTTPS servers
