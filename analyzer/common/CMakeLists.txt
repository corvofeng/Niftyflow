
# 设置源码文件
set(SOURCE_FILES ./common.cpp ./log.h ./packet.h
                ./cnt_rule.h
                ./atom_counter.h
                ./conf.h ./lock.h
                ./con_queue.h
                ./trace.h)

add_library(common STATIC ${SOURCE_FILES})
