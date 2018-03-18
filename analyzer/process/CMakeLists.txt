
# 设置源码文件
set(SOURCE_FILES ./on_process.cpp ./on_process.h)

add_library(process ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(process pthread)
