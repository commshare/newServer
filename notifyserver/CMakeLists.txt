cmake_minimum_required(VERSION 2.6)
PROJECT(offlineMsg)
#SET(CMAKE_CXX_COMPILER "/usr/local/gcc/bin/g++")
#SET(CMAKE_C_COMPILER "/usr/local/gcc/bin/gcc")
SET(ACL_CPP_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/acl/lib_acl_cpp)
SET(ACL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/acl/lib_acl)
SET(IM_PB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/pb) 
SET(GRPC_PB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../commongrpc)

AUX_SOURCE_DIRECTORY( ${IM_PB_DIR} IM_PB_SRCS)
AUX_SOURCE_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR} MSGSVR_SRCS)
SET(grpcnotifyservice_src ${CMAKE_CURRENT_SOURCE_DIR}/grpcnotifyservice.cpp)
SET(imappframe_src ${CMAKE_CURRENT_SOURCE_DIR}/imappframe.cpp           )
SET(imframe_src ${CMAKE_CURRENT_SOURCE_DIR}/imframe.cpp              )
SET(pdusender_src ${CMAKE_CURRENT_SOURCE_DIR}/pdusender.cpp )
SET(notifysendack_src ${CMAKE_CURRENT_SOURCE_DIR}/notifySendAck.cpp )

ADD_DEFINITIONS( -g -W -Wall -Wno-deprecated -std=c++11  -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -D_USE_FAST_MACRO -DLINUX2)


INCLUDE_DIRECTORIES(
	${ACL_CPP_LIB_DIR}/include
	${ACL_LIB_DIR}/include
	${IM_PB_DIR}	
	${GRPC_PB_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_SOURCE_DIR}
	${MONGODB_LIB_DIR}
	${REDIS_LIB_DIR}
	${COMMON_LIB_DIR}
)

LINK_DIRECTORIES(. ./../lib /usr/local/lib)

foreach(_target  notify_server 
	notify_client)
	ADD_EXECUTABLE(${_target}   
	${basehandle_src} 
        ${grpcnotifyservice_src}
        ${imappframe_src} 
        ${imframe_src} 
        ${offLineMsgHandle_src}
		${pdusender_src} 
		${notifysendack_src}
	"${_target}.cc")
#if(${_target} STREQUAL "notify_server")
	TARGET_LINK_LIBRARIES(${_target}  _acl_cpp _acl 
	       	protobuf-lite base commongrpc zookeeper_mt curl slog pthread dl uuid grpc++ grpc)	
	#   else()
	#TARGET_LINK_LIBRARIES(${_target}  protobuf grpc++ grpc)
	#    endif()
endforeach()

