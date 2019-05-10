#!/bin/sh
CPP_DIR=../base/pb
DST_DIR=./gen

GRPC_DIR=../commongrpc

#C++
cp $DST_DIR/cpp/im.* $CPP_DIR/
cp $DST_DIR/grpc/* $GRPC_DIR/ 

rm -rf ./gen
