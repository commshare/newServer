ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -std=c++11)

aux_source_directory(. LIB_SRCS)

#注意：前面一部分要大写，例如LIBMONGOCXX
INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/../../base	
	${CMAKE_CURRENT_SOURCE_DIR}/../../base/acl/lib_acl/include
	${CMAKE_CURRENT_SOURCE_DIR}/../../base/acl/lib_acl_cpp/include
	${CMAKE_CURRENT_SOURCE_DIR}/../mod_common
)
# 生成链接库
add_library (im_redis ${LIB_SRCS})