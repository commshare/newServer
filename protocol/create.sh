#!/bin/sh

SRC_DIR=./
DST_DIR=./gen

#C++
mkdir -p ./gen/cpp

protoc --proto_path=$SRC_DIR/app --proto_path=$SRC_DIR/inner --cpp_out=$DST_DIR/cpp/ -I$SRC_DIR/app/ $SRC_DIR/app/*.proto -I$SRC_DIR/inner/ $SRC_DIR/inner/*.proto

mkdir -p $DST_DIR/grpc

protoc -I=$SRC_DIR/grpc --cpp_out=$DST_DIR/grpc/ $SRC_DIR/grpc/*.proto
protoc -I=$SRC_DIR/grpc --grpc_out=$DST_DIR/grpc/ --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $SRC_DIR/grpc/*.proto

#JAVA
#mkdir -p $DST_DIR/java
#protoc -I=$SRC_DIR --java_out=$DST_DIR/java/ $SRC_DIR/*.proto

#PYTHON
mkdir -p $DST_DIR/python
#protoc -I=$SRC_DIR --python_out=$DST_DIR/python/ $SRC_DIR/*.proto
protoc --proto_path=$SRC_DIR/app --proto_path=$SRC_DIR/inner --python_out=$DST_DIR/cpp/ -I$SRC_DIR/app/ $SRC_DIR/app/*.proto -I$SRC_DIR/inner/ $SRC_DIR/inner/*.proto
