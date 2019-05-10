ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -std=c++11)
aux_source_directory(. DIR_LIB_SRCS)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/../../base/pb
	${CMAKE_CURRENT_SOURCE_DIR}/../../base
)
# 生成链接库
add_library (ws_comm ${DIR_LIB_SRCS})
