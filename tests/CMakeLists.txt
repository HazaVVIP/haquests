# Note: GoogleTest should be added as a submodule
# For now, we'll create a placeholder test file

# Check if GoogleTest is available
find_package(GTest)

if(GTEST_FOUND)
    # Unit tests
    add_executable(test_checksum unit/test_checksum.cpp)
    target_link_libraries(test_checksum haquests GTest::GTest GTest::Main)
    add_test(NAME ChecksumTest COMMAND test_checksum)
    
    add_executable(test_http_parser unit/test_http_parser.cpp)
    target_link_libraries(test_http_parser haquests GTest::GTest GTest::Main)
    add_test(NAME HTTPParserTest COMMAND test_http_parser)
else()
    message(WARNING "GoogleTest not found. Tests will not be built.")
endif()
