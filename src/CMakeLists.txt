# Collect all source files
file(GLOB_RECURSE HAQUESTS_SOURCES
    core/*.cpp
    tcp/*.cpp
    tls/*.cpp
    http/*.cpp
    utils/*.cpp
)

# Create library
add_library(haquests SHARED ${HAQUESTS_SOURCES})

# Link dependencies
target_link_libraries(haquests
    OpenSSL::SSL
    OpenSSL::Crypto
    ${PCAP_LIBRARIES}
    pthread
)

# Set library properties
set_target_properties(haquests PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    PUBLIC_HEADER ${PROJECT_SOURCE_DIR}/include/haquests/haquests.hpp
)

# Install library
install(TARGETS haquests
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/haquests)
