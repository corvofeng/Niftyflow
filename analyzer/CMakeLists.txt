cmake_minimum_required(VERSION 3.5)

project(Analyzer)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_COMPILER "gcc")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -DDEBUG")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Werror -DDEBUG")


include(FindPCAP.cmake)     # 需要pcap库解析pcap文件
include(FindMySQL.cmake)    # 结果存储至MySQL
include(FindRedis.cmake)    # 从控制器中读取配置, 上传预警信息

add_subdirectory(input)
add_subdirectory(process)
add_subdirectory(output)
add_subdirectory(common)
add_subdirectory(test)
