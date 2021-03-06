cmake_minimum_required(VERSION 2.6)
PROJECT(offlineMsgTest)

#SET(ACL_CPP_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/acl/lib_acl_cpp)
#SET(ACL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/acl/lib_acl)


SET(REDISDB_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../mod_redis)


AUX_SOURCE_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR} REDISTEST_SRCS)

#SET(LIBRARY_OUTPUT_PATH ./)

ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -Wno-literal-suffix -Wno-unused-parameter -Wno-ignored-qualifiers -std=c++11  -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -D_USE_FAST_MACRO -DLINUX2)

#注意：前面一部分要大写，例如LIBREDISCXX
INCLUDE_DIRECTORIES(
	${ACL_CPP_LIB_DIR}/include
	${ACL_LIB_DIR}/include
	${LIBREDISCXX_INCLUDE_DIRS}
	${LIBBSONCXX_INCLUDE_DIRS}
	${IM_PB_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../database
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_SOURCE_DIR}
	${REDISDB_LIB_DIR}
	${COMMON_LIB_DIR}
)

LINK_DIRECTORIES(
   ${LIBREDISCXX_LIBRARY_DIRS}
   ${LIBBSONCXX_LIBRARY_DIRS}   
   ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../build
)

#ADD_XXXX指示要生成什么库或者执行文件，必须在TARGET_LINK_LIBRARIES前面，否则会报错
#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})
ADD_EXECUTABLE(redisTest ${REDISTEST_SRCS})

#TARGET_LINK_LIBRARIES指示要生成的库或者执行文件依赖其他什么库
TARGET_LINK_LIBRARIES(redisTest im_redis _acl_cpp _acl protobuf-lite database base zookeeper_mt curl
						${LIBREDISCXX_LIBRARIES} ${LIBBSONCXX_LIBRARIES} slog pthread dl uuid)
 