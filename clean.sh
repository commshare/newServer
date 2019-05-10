#!/bin/sh


function cleanbuild
{
	make clean;
	rm -rf CMakeFiles/ CMakeCache.txt Makefile cmake_install.cmake

}

cd base/
cleanbuild

cd ../database/
cleanbuild

cd ../cm/
cleanbuild

cd ../login
cleanbuild

cd ../msg/
cleanbuild

cd mod_mongodb
cleanbuild

cd ../mod_common
cleanbuild

cd ../mod_mysql
cleanbuild

cd ../mod_redis
cleanbuild

cd ../../group/
cleanbuild

cd mod_mongodb
cleanbuild

cd ../mod_common
cleanbuild

cd ../mod_mysql
cleanbuild

cd ../mod_redis
cleanbuild

cd ../../apush/
cleanbuild

cd ../ipush/
cleanbuild

cd ../customerService/
cleanbuild

cd mod_redis
cleanbuild

cd ../../protocol
rm gen -rf

cd ../lib
rm libmongoc-1.0.so.0 libmongoc-1.0.so libbson-1.0.so.0 libbson-1.0.so libmysqlclient.so.18 libmysqlclient.so libmysqlclient_r.so libprotobuf-lite.so.10 libprotobuf-lite.so liblog4cxx.so.10 liblog4cxx.so libmongocxx.so libbsoncxx.so libzookeeper_mt.so.2 libzookeeper_mt.so libnghttp2.so
