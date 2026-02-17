#include "haquests/tls/certificate.hpp"
#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace haquests {
namespace tls {

Certificate::Certificate() : cert_(nullptr), owned_(false) {}

Certificate::Certificate(X509* cert) : cert_(cert), owned_(false) {}

Certificate::~Certificate() {
    if (owned_ && cert_) {
        X509_free(cert_);
    }
}

bool Certificate::loadFromFile(const std::string& filename) {
    FILE* fp = fopen(filename.c_str(), "r");
    if (!fp) {
        return false;
    }
    
    cert_ = PEM_read_X509(fp, nullptr, nullptr, nullptr);
    fclose(fp);
    
    if (cert_) {
        owned_ = true;
        return true;
    }
    
    return false;
}

bool Certificate::loadFromMemory(const uint8_t* data, size_t len) {
    BIO* bio = BIO_new_mem_buf(data, len);
    if (!bio) {
        return false;
    }
    
    cert_ = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    
    if (cert_) {
        owned_ = true;
        return true;
    }
    
    return false;
}

std::string Certificate::getSubject() const {
    if (!cert_) return "";
    
    char* subj = X509_NAME_oneline(X509_get_subject_name(cert_), nullptr, 0);
    std::string result = subj ? subj : "";
    OPENSSL_free(subj);
    
    return result;
}

std::string Certificate::getIssuer() const {
    if (!cert_) return "";
    
    char* issuer = X509_NAME_oneline(X509_get_issuer_name(cert_), nullptr, 0);
    std::string result = issuer ? issuer : "";
    OPENSSL_free(issuer);
    
    return result;
}

std::string Certificate::getCommonName() const {
    if (!cert_) return "";
    
    X509_NAME* subject = X509_get_subject_name(cert_);
    int pos = X509_NAME_get_index_by_NID(subject, NID_commonName, -1);
    
    if (pos >= 0) {
        X509_NAME_ENTRY* entry = X509_NAME_get_entry(subject, pos);
        ASN1_STRING* data = X509_NAME_ENTRY_get_data(entry);
        
        const unsigned char* str = ASN1_STRING_get0_data(data);
        return std::string(reinterpret_cast<const char*>(str));
    }
    
    return "";
}

std::vector<std::string> Certificate::getSubjectAltNames() const {
    std::vector<std::string> names;
    
    if (!cert_) return names;
    
    STACK_OF(GENERAL_NAME)* san_names = static_cast<STACK_OF(GENERAL_NAME)*>(
        X509_get_ext_d2i(cert_, NID_subject_alt_name, nullptr, nullptr));
    
    if (san_names) {
        int san_count = sk_GENERAL_NAME_num(san_names);
        for (int i = 0; i < san_count; i++) {
            GENERAL_NAME* name = sk_GENERAL_NAME_value(san_names, i);
            if (name->type == GEN_DNS) {
                const unsigned char* dns = ASN1_STRING_get0_data(name->d.dNSName);
                names.push_back(std::string(reinterpret_cast<const char*>(dns)));
            }
        }
        sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
    }
    
    return names;
}

bool Certificate::verify() const {
    // Simplified verification
    return cert_ != nullptr;
}

bool Certificate::isExpired() const {
    if (!cert_) return true;
    
    ASN1_TIME* not_after = X509_get_notAfter(cert_);
    int result = X509_cmp_current_time(not_after);
    
    return result < 0;
}

std::string Certificate::getNotBefore() const {
    if (!cert_) return "";
    
    ASN1_TIME* not_before = X509_get_notBefore(cert_);
    BIO* bio = BIO_new(BIO_s_mem());
    ASN1_TIME_print(bio, not_before);
    
    char buffer[256];
    int len = BIO_read(bio, buffer, sizeof(buffer) - 1);
    buffer[len] = '\0';
    BIO_free(bio);
    
    return std::string(buffer);
}

std::string Certificate::getNotAfter() const {
    if (!cert_) return "";
    
    ASN1_TIME* not_after = X509_get_notAfter(cert_);
    BIO* bio = BIO_new(BIO_s_mem());
    ASN1_TIME_print(bio, not_after);
    
    char buffer[256];
    int len = BIO_read(bio, buffer, sizeof(buffer) - 1);
    buffer[len] = '\0';
    BIO_free(bio);
    
    return std::string(buffer);
}

std::string Certificate::getFingerprint() const {
    if (!cert_) return "";
    
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    
    if (!X509_digest(cert_, EVP_sha256(), md, &md_len)) {
        return "";
    }
    
    std::ostringstream oss;
    for (unsigned int i = 0; i < md_len; i++) {
        if (i > 0) oss << ":";
        oss << std::hex << std::setw(2) << std::setfill('0') 
            << static_cast<int>(md[i]);
    }
    
    return oss.str();
}

} // namespace tls
} // namespace haquests
