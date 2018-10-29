#!/bin/sh

svn up


cd protocol/
./create.sh
./sync.sh

cd ../base/
rm -rf CMakeFiles/ CMakeCache.txt Makefile cmake_install.cmake
cmake .;make clean; make 
cp libbase.so ../lib
cp libbase.so ../../deployment/lib

cd ../database/
rm -rf CMakeFiles/ CMakeCache.txt Makefile cmake_install.cmake
cmake .;make clean; make
cp libdatabase.so ../lib 
cp libdatabase.so ../../deployment/lib

cd ../lib/
sh createLink.sh
sh sync.sh

cd ../msg/
rm -rf CMakeFiles/ CMakeCache.txt Makefile cmake_install.cmake
cmake .;make clean; make
cp msg_server ../../deployment/msg

cd ../cm/
rm -rf CMakeFiles/ CMakeCache.txt Makefile cmake_install.cmake
cmake .;make clean; make
cp cm_server ../../deployment/cm

cd ../group/
rm -rf CMakeFiles/ CMakeCache.txt Makefile cmake_install.cmake
cmake .;make clean; make
cp group_server ../../deployment/group

cd ../apush/
rm -rf CMakeFiles/ CMakeCache.txt Makefile cmake_install.cmake
cmake .;make clean; make
cp apush_server ../../deployment/push/apush

cd ../ipush/
rm -rf CMakeFiles/ CMakeCache.txt Makefile cmake_install.cmake
cmake .;make clean; make
cp ipush_server ../../deployment/push/ipush

cd ../customerService/
rm -rf CMakeFiles/ CMakeCache.txt Makefile cmake_install.cmake
cmake .;make clean; make
cp cust_server ../../deployment/custSvr



