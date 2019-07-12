cmake_minimum_required(VERSION 2.6)
PROJECT(BaseU)

AUX_SOURCE_DIRECTORY(./ SRC_LIST1)
AUX_SOURCE_DIRECTORY(./pb SRC_LIST2)

add_subdirectory(net_lib)

if(APPLE)
    SET(PB_LIB_DIR ./pb/lib/mac)
elseif(UNIX)
    SET(PB_LIB_DIR ./pb/lib/linux)
endif()

SET(LIBRARY_OUTPUT_PATH ./)
#SET(EXECUTABLE_OUTPUT_PATH  ./bin)

ADD_DEFINITIONS( -g -W -Wall -Wno-unused-parameter -Wno-deprecated -D_REENTRANT 
-D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO -DAC_HAS_WARNING -DAC_HAS_ERROR 
-DAC_HAS_CRITICAL -DTIXML_USE_STL -DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11)

INCLUDE_DIRECTORIES(./ ../ ./jsoncpp ./pb ./pb/protocol /usr/local/include)
LINK_DIRECTORIES(./ ${PB_LIB_DIR} ./slog/lib ../lib)

ADD_LIBRARY(baseU SHARED ${SRC_LIST1} ${SRC_LIST2}} 
#ADD_XXXX必须在TARGET_LINK_LIBRARIES前面，否则会报错

TARGET_LINK_LIBRARIES(baseU pthread slog crypto uuid protobuf protobuf-lite uv)
