#!/bin/sh


function buildProject
{
	make clean;
	rm -rf CMakeFiles/ CMakeCache.txt Makefile cmake_install.cmake
	cmake .;make
}


cd lib
sh createLink.sh

cd ../protocol/
sh create.sh
sh sync.sh

cd ../base/
buildProject
cp libbase.so ../lib
cp libbase.so ../deployment/lib

cd ../commongrpc
buildProject
cp libcommongrpc.so ../lib
cp libcommongrpc.so ../deployment/lib

cd ../database/
buildProject
cp libdatabase.so ../lib 
cp libdatabase.so ../deployment/lib

cd ../data_oper/
buildProject

cd ../cm/
buildProject
cp cm_server ../deployment/cm

cd ../login
buildProject
cp login_server ../deployment/login

cd ../msg/
buildProject
cp msg_server ../deployment/msg

cd ../group/
buildProject
cp group_server ../deployment/group

cd ../apush/
buildProject
cp apush_server ../deployment/push/apush

cd ../ipush/
buildProject
cp ipush_server ../deployment/push/ipush

cd ../customerService/
buildProject
cp cust_server ../deployment/custSvr

cd ../notifyserver
buildProject
cp notify_server ../deployment/notify

cd ../channel
buildProject
cp channel_server ../deployment/channel

cd ../destopGw
buildProject
cp desktop_server ../deployment/desktop_server


