# 指定cmake的最小版本
cmake_minimum_required(VERSION 3.10)# 声明要求的cmake最低版本

# 设置项目名称
project(CppHttpDemo)# 声明cmake工程名字,还可以增加版本号码

set(CMAKE_CXX_STANDARD 14)# 指定C++标准

# 设置编译类型
add_executable(CppHttpDemo httpserver/main.cpp httpserver/http_server.cpp httpserver/http_server.h httpclient/http_client.cpp httpclient/http_client.h common/mongoose.c common/mongoose.h )

# print project_name
message(${PROJECT_SOURCE_DIR})
