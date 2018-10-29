cmake_minimum_required(VERSION 2.6)
PROJECT(apushserver)

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

INCLUDE_DIRECTORIES(./ ../base ../base/pb/protocol ../base/pb/ ../base/security/ ${ACL_DIR})
#LINK_DIRECTORIES(./ ../base ${PB_LIB_DIR} ../base/slog/lib ${SECURITY_LIB_DIR})
LINK_DIRECTORIES(./ ../lib ../../lib /usr/local/lib /usr/lib64)
#ADD_XXXX必须在TARGET_LINK_LIBRARIES前面，否则会报错
#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})
ADD_EXECUTABLE(apush_server ${SRC_LIST})

TARGET_LINK_LIBRARIES(apush_server base protobuf-lite slog pthread zookeeper_mt _acl_cpp _acl uuid ssl crypto curl)
 
