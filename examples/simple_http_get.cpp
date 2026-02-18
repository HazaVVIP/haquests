#include "haquests/haquests.hpp"
#include <iostream>
#include <cstring>

using namespace haquests;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <url>" << std::endl;
        return 1;
    }
    
    std::string url = argv[1];
    
    // Check capabilities
    if (!core::RawSocket::hasCapabilities()) {
        std::cerr << "Error: CAP_NET_RAW capability required" << std::endl;
        std::cerr << "Run with sudo or set capabilities: sudo setcap cap_net_raw=eip " << argv[0] << std::endl;
        return 1;
    }
    
    try {
        // Parse URL (simplified - assume http://host:port/path format)
        std::string host = "example.com";
        uint16_t port = 80;
        std::string path = "/";
        
        // Extract host and path from URL
        if (url.find("http://") == 0) {
            url = url.substr(7);
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
        
        std::cout << "Connecting to " << host << ":" << port << std::endl;
        
        // Create TCP connection
        tcp::Connection conn;
        if (!conn.connect(host, port)) {
            std::cerr << "Failed to connect" << std::endl;
            return 1;
        }
        
        std::cout << "Connected successfully" << std::endl;
        
        // Build HTTP request
        http::Request request = http::Request::GET(path);
        request.setHeader("Host", host);
        request.setHeader("Connection", "close");
        
        std::string req_str = request.build();
        std::cout << "Sending request:\n" << req_str << std::endl;
        
        // Send request
        conn.send(reinterpret_cast<const uint8_t*>(req_str.data()), req_str.size());
        
        std::cout << "Request sent, waiting for response..." << std::endl;
        
        // Receive response (simplified)
        std::vector<uint8_t> response = conn.receive(4096);
        
        if (!response.empty()) {
            std::cout << "Received " << response.size() << " bytes" << std::endl;
            std::cout << "\nResponse:\n";
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
