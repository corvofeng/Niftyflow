
# include 文件夹  gcc -I
include_directories(${PROJECT_SOURCE_DIR}/input ${PROJECT_SOURCE_DIR}/output )

# 链接库目录    gcc -L
link_directories(${PROJECT_BINARY_DIR}/input ${PROJECT_BINARY_DIR}/output)

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

TARGET_LINK_LIBRARIES(analyzer reader)
TARGET_LINK_LIBRARIES(analyzer trans)

#TARGET_LINK_LIBRARIES(analyzer ${PCAP_LIBRARY})
#TARGET_LINK_LIBRARIES(lex_rel lex)
#TARGET_LINK_LIBRARIES(yacc_rel yacc)

