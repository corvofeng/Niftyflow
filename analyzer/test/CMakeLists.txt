
# include 文件夹  gcc -I
include_directories(${PROJECT_SOURCE_DIR}/input
                    ${PROJECT_SOURCE_DIR}/output
                    ${PROJECT_SOURCE_DIR}/process
                    ${PROJECT_SOURCE_DIR}/common
                    ${PROJECT_BINARY_DIR}
                    ${CHECK_INCLUDE_DIRS}
                    )

# 链接库目录    gcc -L
link_directories(${PROJECT_BINARY_DIR}/input
                 ${PROJECT_BINARY_DIR}/output
                 ${PROJECT_BINARY_DIR}/process
                 ${PROJECT_BINARY_DIR}/common
                 )

# TARGET_LINK_LIBRARIES(analyzer reader trans process common)

#TARGET_LINK_LIBRARIES(analyzer ${PCAP_LIBRARY})
#TARGET_LINK_LIBRARIES(lex_rel lex)
#TARGET_LINK_LIBRARIES(yacc_rel yacc)

include(CTest)

SET(CTEST_OUTPUT_ON_FAILURE ON)

set(MY_TEST_LIST    LoggerTest;
                    PcapTest;
                    MySQLTest;
                    ProtobufTest;
                    RedisTest;
                    ExampleTest;
                    )

# 添加所有可能使用到的库
SET(LIBRARYS reader trans process common proto)

FOREACH(TEST_ITEM ${MY_TEST_LIST})
    #message(STATUS ${TEST_ITEM})
    # 设置编译输出文件
    add_executable(${TEST_ITEM} ${TEST_ITEM}.cpp)
    add_test(${TEST_ITEM}_tester ${PROJECT_BINARY_DIR}/test/${TEST_ITEM})
    TARGET_LINK_LIBRARIES(${TEST_ITEM} ${LIBRARYS})
ENDFOREACH()

#add_executable (MySQLTest MySQLTest.cpp)
#add_executable (RedisTest RedisTest.cpp)
#


