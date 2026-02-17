#pragma once

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <memory>

namespace haquests {
namespace tcp {
    class Connection;
}

namespace tls {

// Custom BIO method for raw socket integration
class BIOAdapter {
public:
    static BIO_METHOD* getMethod();
    
    // Create BIO for TCP connection
    static BIO* createBIO(tcp::Connection* conn);
    
private:
    // BIO callbacks
    static int bioWrite(BIO* bio, const char* data, int len);
    static int bioRead(BIO* bio, char* data, int len);
    static int bioPuts(BIO* bio, const char* str);
    static int bioGets(BIO* bio, char* buf, int size);
    static long bioCtrl(BIO* bio, int cmd, long arg1, void* arg2);
    static int bioCreate(BIO* bio);
    static int bioDestroy(BIO* bio);
    
    static BIO_METHOD* method_;
    static int bio_index_;
};

} // namespace tls
} // namespace haquests
