#!/bin/bash

#for linux
PROTOC=/home/lam2003/work/test_grpc_sdk/build/linux/Release_x64/3rdparty/grpc/third_party/protobuf/protoc
GRPC_CPP_PLUGIN=/home/lam2003/work/test_grpc_sdk/build/linux/Release_x64/3rdparty/grpc/grpc_cpp_plugin
$PROTOC -I=./ --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN ./*.proto