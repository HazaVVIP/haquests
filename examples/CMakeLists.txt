# Add example executables
add_executable(simple_http_get simple_http_get.cpp)
target_link_libraries(simple_http_get haquests)

add_executable(tls_connection tls_connection.cpp)
target_link_libraries(tls_connection haquests)

add_executable(smuggling_clte smuggling_clte.cpp)
target_link_libraries(smuggling_clte haquests)

# Install examples (optional)
install(TARGETS simple_http_get tls_connection smuggling_clte
        RUNTIME DESTINATION bin/examples)
