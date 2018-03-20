
# 设置源码文件
set(SOURCE_FILES ./common.cpp ./log.h ./packet.h
                ./conf.h ./lock.h
                ./trace.h)

add_library(common ${SOURCE_FILES})
