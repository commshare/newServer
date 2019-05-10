cmake_minimum_required(VERSION 2.6)
PROJECT(login_server)

AUX_SOURCE_DIRECTORY(./ SRC_LIST)

#SET(LIBRARY_OUTPUT_PATH ./)
SET(EXECUTABLE_OUTPUT_PATH  ./)
if(APPLE)
    SET(PB_LIB_DIR ../base/pb/lib/mac)
    SET(SECURITY_LIB_DIR ../base/security/lib/mac)
elseif(UNIX)
    SET(PB_LIB_DIR ../base/pb/lib/linux)
    SET(ACL_DIR	   ../base/acl/lib_acl_cpp/include)
    SET(SECURITY_LIB_DIR ../base/security/lib/linux)
endif()


ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11)

INCLUDE_DIRECTORIES(./ ../base ../base/pb/protocol ../base/pb/ ../base/security/ ../data_oper ${ACL_DIR})
#LINK_DIRECTORIES(./ ../base ${PB_LIB_DIR} ../base/slog/lib ${SECURITY_LIB_DIR})
LINK_DIRECTORIES(./ ../lib ../data_oper ../../lib /usr/local/lib)
#ADD_XXXX必须在TARGET_LINK_LIBRARIES前面，否则会报错
#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})
ADD_EXECUTABLE(login_server ${SRC_LIST})

TARGET_LINK_LIBRARIES(login_server data_oper hiredis  base protobuf-lite slog pthread zookeeper_mt _acl_cpp _acl uuid curl)
 
