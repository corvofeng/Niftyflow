
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/common ${PROJECT_SOURCE_DIR}/libs)
INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIR} ${HIREDIS_INCLUDE_DIR})

# 设置源码文件
SET(SOURCE_FILES ./trans.cpp ./trans.h)


ADD_LIBRARY(trans STATIC ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(trans ${MYSQL_LIB} ${HIREDIS_LIBRARIES} cJSON)

