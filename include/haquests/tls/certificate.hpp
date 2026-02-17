#pragma once

#include <string>
#include <vector>
#include <openssl/x509.h>

namespace haquests {
namespace tls {

class Certificate {
public:
    Certificate();
    explicit Certificate(X509* cert);
    ~Certificate();
    
    // Load from file
    bool loadFromFile(const std::string& filename);
    
    // Load from memory
    bool loadFromMemory(const uint8_t* data, size_t len);
    
    // Get subject
    std::string getSubject() const;
    
    // Get issuer
    std::string getIssuer() const;
    
    // Get common name
    std::string getCommonName() const;
    
    // Get subject alternative names
    std::vector<std::string> getSubjectAltNames() const;
    
    // Verify certificate
    bool verify() const;
    
    // Check if expired
    bool isExpired() const;
    
    // Get not before date
    std::string getNotBefore() const;
    
    // Get not after date
    std::string getNotAfter() const;
    
    // Get fingerprint
    std::string getFingerprint() const;

private:
    X509* cert_;
    bool owned_;
};

} // namespace tls
} // namespace haquests
