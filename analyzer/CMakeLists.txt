cmake_minimum_required(VERSION 3.5)

project(Analyzer)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Werror ")


include(cmake/FindPCAP.cmake)     # 需要pcap库解析pcap文件
include(cmake/FindMySQL.cmake)    # 结果存储至MySQL
include(cmake/FindRedis.cmake)    # 从控制器中读取配置, 上传预警信息
include(cmake/FindLibEvent.cmake)

add_subdirectory(input)
add_subdirectory(process)
add_subdirectory(output)
add_subdirectory(common)
add_subdirectory(libs)
add_subdirectory(conf)

enable_testing()
add_subdirectory(test)


# include 文件夹  gcc -I
include_directories(
                    ${PROJECT_SOURCE_DIR}/input
                    ${PROJECT_SOURCE_DIR}/output
                    ${PROJECT_SOURCE_DIR}/process
                    ${PROJECT_SOURCE_DIR}/common
                    ${PROJECT_SOURCE_DIR}/libs
                    ${PROJECT_BINARY_DIR}   # for protobuf
                    )


# 设置源码文件
set(SOURCE_FILES ./main.cpp)

# 设置编译输出文件
add_executable(analyzer ${SOURCE_FILES})

# 设置编译输出文件
# add_executable(lex_rel ./lex_rel.cpp)

# 设置编译输出文件
# add_executable(yacc_rel ./yacc_rel.cpp)

# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

TARGET_LINK_LIBRARIES(analyzer reader trans process common cJSON event)

