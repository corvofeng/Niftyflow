
# include 文件夹  gcc -I
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/input
                    ${PROJECT_SOURCE_DIR}/output
                    ${PROJECT_SOURCE_DIR}/process
                    ${PROJECT_SOURCE_DIR}/common
                    ${PROJECT_SOURCE_DIR}/libs
                    ${PROJECT_BINARY_DIR}
                    ${DPDK_INCLUDE_DIR}
                    ${CHECK_INCLUDE_DIRS}
)

# 链接库目录    gcc -L
LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/input
                 ${PROJECT_BINARY_DIR}/output
                 ${PROJECT_BINARY_DIR}/process
                 ${PROJECT_BINARY_DIR}/common
                 ${PROJECT_BINARY_DIR}/libs
                 $ENV{DPDK_DIR}/lib
)

INCLUDE(CTest)

SET(CTEST_OUTPUT_ON_FAILURE ON)

SET(MY_TEST_LIST
#  测试名称    ; 输入文件       ; 使用的类库
  "LoggerTest \; LoggerTest.cpp \; common"
  "PcapTest   \; PcapTest.cpp   \; pcaps"
  "DPDKTest   \; DPDKTest.cpp   \; dpdks"
  "RedisTest  \; RedisTest.cpp  \; trans"
  "ConfTest   \; ConfTest.cpp   \; common"
  "MySQLTest  \; MySQLTest.cpp  \; trans"
  "ExampleTest\; ExampleTest.cpp\; "
  )

# For DPDK
ADD_DEFINITIONS(-mssse3 -D__STDC_LIMIT_MACROS)

FOREACH(TEST_ITEM ${MY_TEST_LIST})
    LIST(GET TEST_ITEM 0 TEST_NAME)
    STRING(STRIP ${TEST_NAME} TEST_NAME)

    LIST(GET TEST_ITEM 1 TEST_FILE)
    STRING(STRIP ${TEST_FILE} TEST_FILE)

    LIST(GET TEST_ITEM 2 TEST_LIB)
    STRING(STRIP ${TEST_LIB} TEST_LIB)

    MESSAGE(STATUS "name: '${TEST_NAME}' file: '${TEST_FILE}' lib: '${TEST_LIB}'")

    # 设置编译输出文件
    ADD_EXECUTABLE(${TEST_NAME} ./${TEST_FILE})
    ADD_TEST(${TEST_NAME}_tester ${PROJECT_BINARY_DIR}/test/${TEST_NAME})
    TARGET_LINK_LIBRARIES(${TEST_NAME} ${TEST_LIB})
ENDFOREACH()

