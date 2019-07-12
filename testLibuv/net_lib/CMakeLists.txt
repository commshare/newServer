cmake_minimum_required(VERSION 2.6)
PROJECT(netlib)

AUX_SOURCE_DIRECTORY(./ SRC_LIST1)
#AUX_SOURCE_DIRECTORY(./pb SRC_LIST2)
#AUX_SOURCE_DIRECTORY(./jsoncpp SRC_LIST3)
#AUX_SOURCE_DIRECTORY(./tinyxml2 SRC_LIST4)
#

#if(APPLE)
#    SET(PB_LIB_DIR ./pb/lib/mac)
#elseif(UNIX)
#    SET(PB_LIB_DIR ./pb/lib/linux)
#endif()
#
SET(LIBRARY_OUTPUT_PATH ./)

ADD_DEFINITIONS( -g -W -Wall -Wno-unused-parameter -Wno-deprecated -D_REENTRANT 
-D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO -DAC_HAS_WARNING -DAC_HAS_ERROR 
-DAC_HAS_CRITICAL -DTIXML_USE_STL -DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11)

INCLUDE_DIRECTORIES(./ /usr/local/include)
LINK_DIRECTORIES(./ ../ ../../../Server/src/lib)

ADD_LIBRARY(netlib SHARED ${SRC_LIST1}) 

TARGET_LINK_LIBRARIES(netlib pthread slog crypto )
