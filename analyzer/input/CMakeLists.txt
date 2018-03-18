
include_directories(${PROJECT_SOURCE_DIR}/common)

# 设置源码文件
set(SOURCE_FILES ./reader.cpp ./reader.h)

add_library(reader ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(reader ${PCAP_LIBRARY})

