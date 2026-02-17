#pragma once

#include "request.hpp"
#include <string>

namespace haquests {
namespace http {

// HTTP Request Smuggling techniques
enum class SmugglingType {
    CL_TE,    // Content-Length vs Transfer-Encoding
    TE_CL,    // Transfer-Encoding vs Content-Length
    TE_TE     // Dual Transfer-Encoding
};

class Smuggling {
public:
    // Create CL.TE smuggling request
    static Request createCLTE(const std::string& url,
                              const std::string& smuggled_request);
    
    // Create TE.CL smuggling request
    static Request createTECL(const std::string& url,
                              const std::string& smuggled_request);
    
    // Create TE.TE smuggling request
    static Request createTETE(const std::string& url,
                              const std::string& smuggled_request);
    
    // Build malformed request with custom headers
    static Request buildMalformed(const std::string& url,
                                  const std::string& content,
                                  SmugglingType type);
    
    // Test if endpoint is vulnerable
    static bool detectVulnerability(const std::string& url,
                                    SmugglingType type);
};

} // namespace http
} // namespace haquests
