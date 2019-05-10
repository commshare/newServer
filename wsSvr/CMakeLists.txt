cmake_minimum_required(VERSION 2.6)
PROJECT(wsSvr)


SET(ACL_CPP_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/acl/lib_acl_cpp)
SET(ACL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/acl/lib_acl)
SET(IM_PB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/pb)

SET(COMMON_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_common)
SET(MONGODB_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_mongodb)
SET(MYSQL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_mysql)
SET(REDIS_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_redis)


AUX_SOURCE_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR} WSSERVICE_SRCS)


#SET(LIBRARY_OUTPUT_PATH ./)
add_subdirectory(${REDIS_LIB_DIR})
add_subdirectory(${MONGODB_LIB_DIR})
add_subdirectory(${MYSQL_LIB_DIR})
add_subdirectory(${COMMON_LIB_DIR})

ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -std=c++11 -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -D_USE_FAST_MACRO -DLINUX2)
#ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -Wno-literal-suffix -Wno-unused-parameter -Wno-ignored-qualifiers -std=c++11  -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -D_USE_FAST_MACRO -DLINUX2)


#注意：前面一部分要大写，例如LIBMONGOCXX
INCLUDE_DIRECTORIES(
	${ACL_CPP_LIB_DIR}/include
	${ACL_LIB_DIR}/include
	${IM_PB_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_SOURCE_DIR}/../base/jsoncpp
	${CMAKE_CURRENT_SOURCE_DIR}/../database
	${CMAKE_CURRENT_SOURCE_DIR}
	${MONGODB_LIB_DIR}
	${MYSQL_LIB_DIR}
	${REDIS_LIB_DIR}
	${COMMON_LIB_DIR}
)

LINK_DIRECTORIES(. ./../lib ./../../lib /usr/local/lib ./mod_redis)

#ADD_XXXX指示要生成什么库或者执行文件，必须在TARGET_LINK_LIBRARIES前面，否则会报错
#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})
#ADD_LIBRARY(cs_redis ${REDIS_LIB_SRCS})
ADD_EXECUTABLE(ws_server ${WSSERVICE_SRCS})

#TARGET_LINK_LIBRARIES指示要生成的库或者执行文件依赖其他什么库
TARGET_LINK_LIBRARIES(ws_server cs_redis hiredis ws_mongo ws_mysql ws_comm _acl_cpp _acl  protobuf-lite database base  zookeeper_mt curl
					boost_system boost_chrono mongocxx bsoncxx mongoc-1.0 bson-1.0 slog pthread dl uuid)
