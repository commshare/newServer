cmake_minimum_required(VERSION 2.6)
PROJECT(groupserver)

SET(ACL_CPP_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/acl/lib_acl_cpp)
SET(ACL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/acl/lib_acl)
SET(IM_PB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/pb)

SET(COMMON_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_common)
SET(MONGODB_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_mongodb)
SET(MYSQL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_mysql)
SET(REDIS_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_redis)

AUX_SOURCE_DIRECTORY( ${IM_PB_DIR} IM_PB_SRCS)
AUX_SOURCE_DIRECTORY(./ SRC_LIST)

#SET(LIBRARY_OUTPUT_PATH ./)
add_subdirectory(${COMMON_LIB_DIR})
add_subdirectory(${MONGODB_LIB_DIR})
add_subdirectory(${MYSQL_LIB_DIR})
add_subdirectory(${REDIS_LIB_DIR})

SET(EXECUTABLE_OUTPUT_PATH  ./)
if(APPLE)
    SET(PB_LIB_DIR ../base/pb/lib/mac)
    SET(SECURITY_LIB_DIR ../base/security/lib/mac)
elseif(UNIX)
    SET(PB_LIB_DIR ../base/pb/lib/linux)
    SET(ACL_DIR	   ../base/acl/lib_acl_cpp/include)
    SET(SECURITY_LIB_DIR ../base/security/lib/linux)
endif()


ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -Wno-unused-parameter -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11)

#INCLUDE_DIRECTORIES(./ ../base ../base/pb/protocol ../base/pb/ ../base/security/ ${ACL_DIR}  ${MONGODB_LIB_DIR} ${MYSQL_LIB_DIR} ${REDIS_LIB_DIR} ${COMMON_LIB_DIR})
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

LINK_DIRECTORIES(./ ../lib ../../lib /usr/local/lib)
#ADD_LIBRARY(im_pb ${IM_PB_SRCS})
ADD_EXECUTABLE(group_server ${SRC_LIST})

#TARGET_LINK_LIBRARIES(group_server im_mongo im_mysql im_redis im_comm  base protobuf-lite slog pthread zookeeper_mt _acl_cpp _acl uuid curl)
TARGET_LINK_LIBRARIES(group_server im_mongo im_mysql im_redis im_comm _acl_cpp _acl  protobuf-lite database base zookeeper_mt curl mysqlclient_r
						mongocxx bsoncxx mongoc-1.0 bson-1.0 slog pthread dl uuid)
