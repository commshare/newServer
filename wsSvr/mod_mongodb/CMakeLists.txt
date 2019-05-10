ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -std=c++11)

aux_source_directory(. LIB_MONGO_SRCS)

#find_package(libmongocxx REQUIRED)
#find_package(libbsoncxx REQUIRED)

#注意：前面一部分要大写，例如LIBMONGOCXX
INCLUDE_DIRECTORIES(
#	${LIBMONGOCXX_INCLUDE_DIRS}
#	${LIBBSONCXX_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/../mod_common
	${CMAKE_CURRENT_SOURCE_DIR}/../../base
	${CMAKE_CURRENT_SOURCE_DIR}/../../database
	${CMAKE_CURRENT_SOURCE_DIR}/../../base/pb
)
# 生成链接库
add_library (ws_mongo ${LIB_MONGO_SRCS})
