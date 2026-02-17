#include "haquests/tls/connection.hpp"
#include "haquests/tls/bio_adapter.hpp"
#include "haquests/utils/error.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace haquests {
namespace tls {

class Connection::Impl {
public:
    Impl() : ssl_ctx_(nullptr), ssl_(nullptr), verify_cert_(true) {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
    }
    
    ~Impl() {
        if (ssl_) {
            SSL_free(ssl_);
        }
        if (ssl_ctx_) {
            SSL_CTX_free(ssl_ctx_);
        }
    }
    
    bool connect(const std::string& host, uint16_t port) {
        // Create TCP connection first
        if (!tcp_conn_.connect(host, port)) {
            return false;
        }
        
        // Create SSL context
        ssl_ctx_ = SSL_CTX_new(TLS_client_method());
        if (!ssl_ctx_) {
            return false;
        }
        
        // Set verification mode
        if (verify_cert_) {
            SSL_CTX_set_verify(ssl_ctx_, SSL_VERIFY_PEER, nullptr);
            SSL_CTX_set_default_verify_paths(ssl_ctx_);
        } else {
            SSL_CTX_set_verify(ssl_ctx_, SSL_VERIFY_NONE, nullptr);
        }
        
        // Create SSL structure
        ssl_ = SSL_new(ssl_ctx_);
        if (!ssl_) {
            return false;
        }
        
        // Create custom BIO
        BIO* bio = BIOAdapter::createBIO(&tcp_conn_);
        SSL_set_bio(ssl_, bio, bio);
        
        // Set SNI
        SSL_set_tlsext_host_name(ssl_, host.c_str());
        
        // Perform handshake
        if (SSL_connect(ssl_) != 1) {
            return false;
        }
        
        return true;
    }
    
    ssize_t send(const uint8_t* data, size_t len) {
        if (!ssl_) {
            throw utils::TLSError("Not connected");
        }
        
        return SSL_write(ssl_, data, len);
    }
    
    std::vector<uint8_t> receive(size_t max_len) {
        if (!ssl_) {
            throw utils::TLSError("Not connected");
        }
        
        std::vector<uint8_t> buffer(max_len);
        int received = SSL_read(ssl_, buffer.data(), max_len);
        
        if (received > 0) {
            buffer.resize(received);
            return buffer;
        }
        
        return std::vector<uint8_t>();
    }
    
    void close() {
        if (ssl_) {
            SSL_shutdown(ssl_);
        }
        tcp_conn_.close();
    }
    
    bool isConnected() const {
        return tcp_conn_.isConnected() && ssl_ != nullptr;
    }
    
    std::string getTLSVersion() const {
        if (!ssl_) return "";
        return SSL_get_version(ssl_);
    }
    
    std::string getCipherSuite() const {
        if (!ssl_) return "";
        const char* cipher = SSL_get_cipher_name(ssl_);
        return cipher ? cipher : "";
    }
    
    void setVerifyCertificate(bool verify) {
        verify_cert_ = verify;
    }
    
    void setCAFile(const std::string& ca_file) {
        ca_file_ = ca_file;
    }
    
    void setCAPath(const std::string& ca_path) {
        ca_path_ = ca_path;
    }

private:
    tcp::Connection tcp_conn_;
    SSL_CTX* ssl_ctx_;
    SSL* ssl_;
    bool verify_cert_;
    std::string ca_file_;
    std::string ca_path_;
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

bool Connection::isConnected() const {
    return impl_->isConnected();
}

std::string Connection::getTLSVersion() const {
    return impl_->getTLSVersion();
}

std::string Connection::getCipherSuite() const {
    return impl_->getCipherSuite();
}

void Connection::setVerifyCertificate(bool verify) {
    impl_->setVerifyCertificate(verify);
}

void Connection::setCAFile(const std::string& ca_file) {
    impl_->setCAFile(ca_file);
}

void Connection::setCAPath(const std::string& ca_path) {
    impl_->setCAPath(ca_path);
}

} // namespace tls
} // namespace haquests
