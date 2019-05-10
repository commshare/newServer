cmake_minimum_required(VERSION 2.6)
PROJECT(channel_server)

SET(ACL_CPP_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/acl/lib_acl_cpp)
SET(ACL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/acl/lib_acl)
SET(IM_PB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/pb)
SET(IM_JSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/jsoncpp)

SET(MONGODB_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mod_mongodb)

AUX_SOURCE_DIRECTORY( ${IM_PB_DIR} IM_PB_SRCS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR} CHNNSVR_SRC)

add_subdirectory(${MONGODB_LIB_DIR})

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

INCLUDE_DIRECTORIES(
	${ACL_CPP_LIB_DIR}/include
	${ACL_LIB_DIR}/include
	${IM_PB_DIR}
	${IM_JSON_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_SOURCE_DIR}/../database
	${CMAKE_CURRENT_SOURCE_DIR}
	${MONGODB_LIB_DIR}
	${REDIS_LIB_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../data_oper
)

LINK_DIRECTORIES(./ ../lib ../../lib ../data_oper /usr/local/lib ./mod_mongodb)
ADD_EXECUTABLE(channel_server ${CHNNSVR_SRC})

TARGET_LINK_LIBRARIES(channel_server data_oper im_mongo hiredis _acl_cpp _acl  protobuf-lite database base zookeeper_mt curl mysqlclient_r
						mongocxx bsoncxx mongoc-1.0 bson-1.0 slog pthread dl uuid)
