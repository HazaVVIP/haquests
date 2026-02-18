#include "haquests/tls/bio_adapter.hpp"
#include "haquests/tcp/connection.hpp"
#include <unistd.h>
#include <cstring>
#include <vector>

namespace haquests {
namespace tls {

// Buffer structure to hold unread data
struct BIOBuffer {
    tcp::Connection* conn;
    std::vector<uint8_t> buffer;
    size_t offset;
    
    BIOBuffer(tcp::Connection* c) : conn(c), offset(0) {}
};

BIO_METHOD* BIOAdapter::method_ = nullptr;
int BIOAdapter::bio_index_ = -1;

BIO_METHOD* BIOAdapter::getMethod() {
    if (!method_) {
        method_ = BIO_meth_new(BIO_TYPE_SOURCE_SINK, "TCP Connection BIO");
        if (method_) {
            BIO_meth_set_write(method_, bioWrite);
            BIO_meth_set_read(method_, bioRead);
            BIO_meth_set_puts(method_, bioPuts);
            BIO_meth_set_gets(method_, bioGets);
            BIO_meth_set_ctrl(method_, bioCtrl);
            BIO_meth_set_create(method_, bioCreate);
            BIO_meth_set_destroy(method_, bioDestroy);
        }
    }
    return method_;
}

BIO* BIOAdapter::createBIO(tcp::Connection* conn) {
    BIO* bio = BIO_new(getMethod());
    if (bio) {
        BIOBuffer* buf = new BIOBuffer(conn);
        BIO_set_data(bio, buf);
        BIO_set_init(bio, 1);
    }
    return bio;
}

int BIOAdapter::bioWrite(BIO* bio, const char* data, int len) {
    BIOBuffer* buf = static_cast<BIOBuffer*>(BIO_get_data(bio));
    if (!buf || !buf->conn) {
        return -1;
    }
    
    ssize_t sent = buf->conn->send(reinterpret_cast<const uint8_t*>(data), len);
    return static_cast<int>(sent);
}

int BIOAdapter::bioRead(BIO* bio, char* data, int len) {
    BIOBuffer* buf = static_cast<BIOBuffer*>(BIO_get_data(bio));
    if (!buf || !buf->conn) {
        return -1;
    }
    
    // Check if we have buffered data from a previous read
    if (buf->offset < buf->buffer.size()) {
        // Return buffered data
        size_t available = buf->buffer.size() - buf->offset;
        size_t to_copy = std::min(available, static_cast<size_t>(len));
        std::memcpy(data, buf->buffer.data() + buf->offset, to_copy);
        buf->offset += to_copy;
        
        // Clear buffer if fully consumed
        if (buf->offset >= buf->buffer.size()) {
            buf->buffer.clear();
            buf->offset = 0;
        }
        
        return static_cast<int>(to_copy);
    }
    
    // No buffered data - receive new data from TCP connection
    std::vector<uint8_t> received = buf->conn->receive(len);
    
    if (received.empty()) {
        // No data received - tell OpenSSL to retry
        BIO_set_retry_read(bio);
        return -1;
    }
    
    // If we received more than requested, buffer the extra for next read
    if (received.size() <= static_cast<size_t>(len)) {
        // Received data fits in request - return it all
        std::memcpy(data, received.data(), received.size());
        return static_cast<int>(received.size());
    } else {
        // Received more than requested - return what was asked and buffer the rest
        std::memcpy(data, received.data(), len);
        buf->buffer.assign(received.begin() + len, received.end());
        buf->offset = 0;
        return len;
    }
}

int BIOAdapter::bioPuts(BIO* bio, const char* str) {
    return bioWrite(bio, str, strlen(str));
}

int BIOAdapter::bioGets(BIO* bio, char* buf, int size) {
    (void)bio;   // Suppress unused parameter warning
    (void)buf;   // Suppress unused parameter warning
    (void)size;  // Suppress unused parameter warning
    // Not typically used for SSL
    return -1;
}

long BIOAdapter::bioCtrl(BIO* bio, int cmd, long arg1, void* arg2) {
    (void)bio;     // Suppress unused parameter warning
    (void)arg1;    // Suppress unused parameter warning
    (void)arg2;    // Suppress unused parameter warning
    
    switch (cmd) {
        case BIO_CTRL_FLUSH:
            return 1;
        case BIO_CTRL_EOF:
            return 0;
        case BIO_CTRL_WPENDING:
            return 0;
        case BIO_CTRL_PENDING:
            return 0;
        case BIO_CTRL_PUSH:
        case BIO_CTRL_POP:
            return 0;
        default:
            return 0;
    }
}

int BIOAdapter::bioCreate(BIO* bio) {
    BIO_set_shutdown(bio, 0);
    BIO_set_init(bio, 0);
    BIO_set_data(bio, nullptr);
    return 1;
}

int BIOAdapter::bioDestroy(BIO* bio) {
    if (!bio) {
        return 0;
    }
    
    BIOBuffer* buf = static_cast<BIOBuffer*>(BIO_get_data(bio));
    delete buf;
    
    BIO_set_data(bio, nullptr);
    BIO_set_init(bio, 0);
    return 1;
}

} // namespace tls
} // namespace haquests
