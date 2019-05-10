#ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -std=c++11 -DDEBUG)
ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -std=c++11)
aux_source_directory(. LIB_SRCS)

#注意：前面一部分要大写，例如LIBMONGOCXX
INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/../	
	${CMAKE_CURRENT_SOURCE_DIR}/../../base	
	${CMAKE_CURRENT_SOURCE_DIR}/../../base/jsoncpp	
	${CMAKE_CURRENT_SOURCE_DIR}/../../base/pb	
	${CMAKE_CURRENT_SOURCE_DIR}/../../base/acl/lib_acl/include
	${CMAKE_CURRENT_SOURCE_DIR}/../../base/acl/lib_acl_cpp/include

)
# 生成链接库
add_library (cs_redis ${LIB_SRCS})
