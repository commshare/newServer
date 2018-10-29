cmake_minimum_required(VERSION 2.6)
PROJECT(ipush_server)


SET(IM_PB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/pb)


AUX_SOURCE_DIRECTORY( ${IM_PB_DIR} IM_PB_SRCS)
AUX_SOURCE_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR} IPUSH_SRCS)

#SET(LIBRARY_OUTPUT_PATH ./)

ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated  -std=c++11  -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -D_USE_FAST_MACRO -DLINUX2)

#注意：前面一部分要大写，例如LIBMONGOCXX
INCLUDE_DIRECTORIES(
	${IM_PB_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_SOURCE_DIR}/../base/jsoncpp
	${CMAKE_CURRENT_SOURCE_DIR}
)

LINK_DIRECTORIES(. ./../lib ./../../lib /usr/local/lib)

#ADD_XXXX指示要生成什么库或者执行文件，必须在TARGET_LINK_LIBRARIES前面，否则会报错
#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})
ADD_EXECUTABLE(ipush_server ${IPUSH_SRCS})

#TARGET_LINK_LIBRARIES指示要生成的库或者执行文件依赖其他什么库
TARGET_LINK_LIBRARIES(ipush_server protobuf-lite base  zookeeper_mt slog pthread dl uuid nghttp2 crypto ssl dl curl)
					
						
