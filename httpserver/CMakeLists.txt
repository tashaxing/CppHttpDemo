
add_executable(http_server http_server.cpp http_server.h main.cpp)
target_link_libraries(http_server mongoose)
