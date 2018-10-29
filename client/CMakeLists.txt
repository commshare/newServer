cmake_minimum_required(VERSION 2.6)
PROJECT(client)

AUX_SOURCE_DIRECTORY(./ SRC_LIST)

#SET(LIBRARY_OUTPUT_PATH ./)
SET(EXECUTABLE_OUTPUT_PATH  ./)
if(APPLE)
    SET(PB_LIB_DIR ../base-cli/pb/lib/mac)
elseif(UNIX)
    SET(PB_LIB_DIR ../base-cli/pb/lib/linux)
endif()

ADD_DEFINITIONS( -g -W -Wall -Wno-unused-parameter -Wno-deprecated -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11)

INCLUDE_DIRECTORIES(./ ../base-cli ../base-cli/jsoncpp/ ../base-cli/pb/)
LINK_DIRECTORIES(./ ../lib ../base-cli ../../base-cli ${PB_LIB_DIR} ../base-cli/slog/lib ../../base-cli/slog/lib /usr/local/lib)

#ADD_XXXX必须在TARGET_LINK_LIBRARIES前面，否则会报错
#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})
ADD_EXECUTABLE(client ${SRC_LIST})
ADD_EXECUTABLE(client2 ${SRC_LIST})

TARGET_LINK_LIBRARIES(client base-cli protobuf-lite slog pthread uuid) 
TARGET_LINK_LIBRARIES(client2 base-cli protobuf-lite slog pthread uuid) 
