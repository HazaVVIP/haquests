#include "haquests/haquests.hpp"
#include <iostream>

using namespace haquests;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <url>" << std::endl;
        return 1;
    }
    
    std::string url = argv[1];
    
    try {
        // Parse URL
        std::string host = "www.google.com";
        uint16_t port = 443;
        std::string path = "/";
        
        if (url.find("https://") == 0) {
            url = url.substr(8);
        }
        
        size_t slash_pos = url.find('/');
        std::string host_port;
        if (slash_pos != std::string::npos) {
            host_port = url.substr(0, slash_pos);
            path = url.substr(slash_pos);
        } else {
            host_port = url;
        }
        
        // Parse host and port
        size_t colon_pos = host_port.find(':');
        if (colon_pos != std::string::npos) {
            host = host_port.substr(0, colon_pos);
            port = static_cast<uint16_t>(std::stoi(host_port.substr(colon_pos + 1)));
        } else {
            host = host_port;
        }
        
        std::cout << "Connecting to " << host << ":" << port << " (TLS)" << std::endl;
        
        // Create TLS connection
        tls::Connection conn;
        // Disable certificate verification for testing purposes
        // In production, you should verify certificates!
        conn.setVerifyCertificate(false);
        if (!conn.connect(host, port)) {
            std::cerr << "Failed to connect" << std::endl;
            return 1;
        }
        
        std::cout << "Connected with " << conn.getTLSVersion() 
                  << " using " << conn.getCipherSuite() << std::endl;
        
        // Build HTTP request
        http::Request request = http::Request::GET(path);
        request.setHeader("Host", host);
        request.setHeader("Connection", "close");
        
        std::string req_str = request.build();
        
        // Send request
        conn.send(reinterpret_cast<const uint8_t*>(req_str.data()), req_str.size());
        
        std::cout << "Request sent, waiting for response..." << std::endl;
        
        // Receive response
        std::vector<uint8_t> response = conn.receive(4096);
        
        if (!response.empty()) {
            std::cout << "Received " << response.size() << " bytes\n";
            std::cout.write(reinterpret_cast<const char*>(response.data()), response.size());
            std::cout << std::endl;
        }
        
        conn.close();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
