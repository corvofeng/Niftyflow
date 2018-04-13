
INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/input
        ${PROJECT_SOURCE_DIR}/output
        ${PROJECT_SOURCE_DIR}/libs
        ${PROJECT_SOURCE_DIR}/common
        )

# 设置源码文件
set(SOURCE_FILES ./on_process.cpp ./on_process.h
            ./watcher.h ./watcher.cpp
            ./ever_main.h ./ever_main.cpp)

add_library(process STATIC ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(process pthread reader trans)
