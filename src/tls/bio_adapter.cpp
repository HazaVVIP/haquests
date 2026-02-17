#include "haquests/tls/bio_adapter.hpp"
#include "haquests/tcp/connection.hpp"

namespace haquests {
namespace tls {

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
        BIO_set_data(bio, conn);
        BIO_set_init(bio, 1);
    }
    return bio;
}

int BIOAdapter::bioWrite(BIO* bio, const char* data, int len) {
    tcp::Connection* conn = static_cast<tcp::Connection*>(BIO_get_data(bio));
    if (!conn) {
        return -1;
    }
    
    ssize_t sent = conn->send(reinterpret_cast<const uint8_t*>(data), len);
    return static_cast<int>(sent);
}

int BIOAdapter::bioRead(BIO* bio, char* data, int len) {
    tcp::Connection* conn = static_cast<tcp::Connection*>(BIO_get_data(bio));
    if (!conn) {
        return -1;
    }
    
    std::vector<uint8_t> received = conn->receive(len);
    if (received.empty()) {
        return 0;
    }
    
    std::memcpy(data, received.data(), received.size());
    return static_cast<int>(received.size());
}

int BIOAdapter::bioPuts(BIO* bio, const char* str) {
    return bioWrite(bio, str, strlen(str));
}

int BIOAdapter::bioGets(BIO* bio, char* buf, int size) {
    // Not typically used for SSL
    return -1;
}

long BIOAdapter::bioCtrl(BIO* bio, int cmd, long arg1, void* arg2) {
    switch (cmd) {
        case BIO_CTRL_FLUSH:
            return 1;
        case BIO_CTRL_EOF:
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
    
    BIO_set_data(bio, nullptr);
    BIO_set_init(bio, 0);
    return 1;
}

} // namespace tls
} // namespace haquests
