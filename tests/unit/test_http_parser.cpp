#include "haquests/http/request.hpp"
#include <gtest/gtest.h>

using namespace haquests::http;

TEST(HTTPRequestTest, BasicGET) {
    Request req = Request::GET("/test");
    req.setHeader("Host", "example.com");
    
    std::string result = req.build();
    
    EXPECT_NE(result.find("GET /test HTTP/1.1"), std::string::npos);
    EXPECT_NE(result.find("Host: example.com"), std::string::npos);
}

TEST(HTTPRequestTest, POST) {
    Request req = Request::POST("/api", "test data");
    req.setHeader("Host", "example.com");
    
    std::string result = req.build();
    
    EXPECT_NE(result.find("POST /api HTTP/1.1"), std::string::npos);
    EXPECT_NE(result.find("Content-Length:"), std::string::npos);
    EXPECT_NE(result.find("test data"), std::string::npos);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
