
include_directories(${PROJECT_SOURCE_DIR}/common
                    ${DPDK_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/libs)

MESSAGE(STATUS "GET DPDK include  ${DPDK_INCLUDE_DIR}")

# 设置源码文件
set(SOURCE_FILES ./reader.cpp ./reader.h)
set(DPDK_SOURCE_FILES ./dpdk_adapter.h ./dpdk_adapter.cpp)

add_library(reader)
add_library(pcaps STATIC ${SOURCE_FILES})
add_library(dpdks STATIC ${DPDK_SOURCE_FILES})

TARGET_LINK_LIBRARIES(reader pcaps dpdks)

TARGET_LINK_LIBRARIES(pcaps ${PCAP_LIBRARY})
TARGET_LINK_LIBRARIES(dpdks ${DPDK_LIBRARIES} pthread)
