#include "haquests/core/checksum.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace haquests::core;

TEST(ChecksumTest, BasicChecksum) {
    std::vector<uint16_t> data = {0x0001, 0x0002, 0x0003, 0x0004};
    uint16_t result = checksum(data.data(), data.size() * sizeof(uint16_t));
    
    EXPECT_NE(result, 0);
}

TEST(ChecksumTest, ZeroData) {
    std::vector<uint16_t> data = {0x0000, 0x0000, 0x0000};
    uint16_t result = checksum(data.data(), data.size() * sizeof(uint16_t));
    
    EXPECT_EQ(result, 0xFFFF);
}

TEST(ChecksumTest, TCPChecksum) {
    uint32_t src_ip = 0x7F000001;  // 127.0.0.1
    uint32_t dst_ip = 0x7F000001;
    
    uint8_t tcp_segment[] = {0x00, 0x50, 0x1F, 0x90};  // Sample TCP data
    uint16_t result = tcpChecksum(src_ip, dst_ip, tcp_segment, sizeof(tcp_segment));
    
    EXPECT_NE(result, 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
