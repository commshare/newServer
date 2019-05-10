cmake_minimum_required(VERSION 2.6)
PROJECT(Msg)

SET(ACL_CPP_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/acl/lib_acl_cpp)
SET(ACL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/acl/lib_acl)
SET(IM_PB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/pb)

SET(COMMON_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_common)
SET(MONGODB_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_mongodb)
SET(MYSQL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_mysql)
SET(REDIS_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_redis)


AUX_SOURCE_DIRECTORY( ${IM_PB_DIR} IM_PB_SRCS)
AUX_SOURCE_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR} MSGSVR_SRCS)

#AUX_SOURCE_DIRECTORY( ${COMMON_LIB_DIR} COMMON_LIB_SRCS)
#AUX_SOURCE_DIRECTORY( ${MONGODB_LIB_DIR} MONGODB_LIB_SRCS)
#AUX_SOURCE_DIRECTORY( ${MYSQL_LIB_DIR} MYSQL_LIB_SRCS)
#AUX_SOURCE_DIRECTORY( ${REDIS_LIB_DIR} REDIS_LIB_SRCS)


#SET(LIBRARY_OUTPUT_PATH ./)
add_subdirectory(${COMMON_LIB_DIR})
add_subdirectory(${MONGODB_LIB_DIR})
add_subdirectory(${MYSQL_LIB_DIR})
add_subdirectory(${REDIS_LIB_DIR})

ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -std=c++11  -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -D_USE_FAST_MACRO -DLINUX2)
#ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -Wno-literal-suffix -Wno-unused-parameter -Wno-ignored-qualifiers -std=c++11  -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -D_USE_FAST_MACRO -DLINUX2)


#注意：前面一部分要大写，例如LIBMONGOCXX
INCLUDE_DIRECTORIES(
	${ACL_CPP_LIB_DIR}/include
	${ACL_LIB_DIR}/include
	${IM_PB_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_SOURCE_DIR}/../database
	${CMAKE_CURRENT_SOURCE_DIR}
	${MONGODB_LIB_DIR}
	${MYSQL_LIB_DIR}
	${REDIS_LIB_DIR}
	${COMMON_LIB_DIR}
)

LINK_DIRECTORIES(. ./../lib ./../../lib /usr/local/lib)

#ADD_XXXX指示要生成什么库或者执行文件，必须在TARGET_LINK_LIBRARIES前面，否则会报错
#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})
#ADD_LIBRARY(im_pb ${IM_PB_SRCS})
#ADD_LIBRARY(im_comm ${COMMON_LIB_SRCS})
#ADD_LIBRARY(im_mongo ${MONGODB_LIB_SRCS})
#ADD_LIBRARY(im_mysql ${MYSQL_LIB_SRCS})
#ADD_LIBRARY(im_redis ${REDIS_LIB_SRCS})
ADD_EXECUTABLE(msg_server ${MSGSVR_SRCS})

#ADD_EXECUTABLE(msgServerDbg ${MSGSVR_SRCS})

#TARGET_LINK_LIBRARIES指示要生成的库或者执行文件依赖其他什么库
TARGET_LINK_LIBRARIES(msg_server im_mongo im_mysql im_redis im_comm _acl_cpp _acl  protobuf-lite database base  zookeeper_mt curl mysqlclient_r
						mongocxx bsoncxx mongoc-1.0 bson-1.0 slog pthread dl uuid)
#TARGET_LINK_LIBRARIES(msgServerDbg im_mongo im_mysql im_redis im_comm _acl_cpp _acl  im_pb protobuf-lite database base zookeeper_mt curlmongocxx bsoncxx slog pthread dl uuid)						
						
#install (TARGETS msgServer DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../../deployment/msg)
