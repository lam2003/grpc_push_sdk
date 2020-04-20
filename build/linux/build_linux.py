#!/usr/bin/env python

from __future__ import print_function
import sys
import os
import argparse
import shutil
import platform

filename = '../build_common.py'
if sys.version_info[0] <= 2:
    execfile(filename)
else:
    exec(compile(open(filename, "rb").read(), filename, 'exec'))
init_common(os.path.abspath('..'), 'linux')

# 获取平台相关的参数
bits, linkage = platform.architecture()
if bits == '64bit':
    ARCH = 'x64'
elif bits == '32bit':
    ARCH = 'x86'
else:
    ARCH = bits

# 生成构建文件夹
set_build_folder_name(BUILD_MODE + '_' + ARCH)

libs_path = os.path.abspath(os.path.join('../../out/lib/'))

print('Architecture:', ARCH)
print('Build mode  :', BUILD_MODE)
print('Libs path:', libs_path)


# cmake初始化
cmake_cmd = [
    'cmake',
    '-B', BUILD_DIR,
    '-DBUILD_MODE'.join(BUILD_MODE),
    '../..'
]

call(cmake_cmd)
build('grpc_cpp_plugin')
build('protoc')
build('grpc++')
build("spdlog")
build('service-mesh-cpp')


def copy_protobuf_lib():
    copy_file(BUILD_DIR + '/3rdparty/grpc/third_party/protobuf/libprotobuf.a', libs_path)

def copy_ssl_lib():
    copy_file(BUILD_DIR + '/3rdparty/grpc/third_party/boringssl/crypto/libcrypto.a', libs_path)
    copy_file(BUILD_DIR + '/3rdparty/grpc/third_party/boringssl/ssl/libssl.a', libs_path)

def copy_grpc_lib():
    copy_file(BUILD_DIR + '/3rdparty/grpc/libaddress_sorting.a', libs_path)
    copy_file(BUILD_DIR + '/3rdparty/grpc/libgpr.a', libs_path)
    copy_file(BUILD_DIR + '/3rdparty/grpc/libgrpc++.a', libs_path)
    copy_file(BUILD_DIR + '/3rdparty/grpc/libgrpc.a', libs_path)
    copy_file(BUILD_DIR + '/3rdparty/grpc/third_party/cares/cares/lib/libcares.a', libs_path)
    copy_file(BUILD_DIR + '/3rdparty/grpc/third_party/zlib/libz.a', libs_path)

def copy_service_mesh_lib():
    copy_file(BUILD_DIR + '/src/libservice-mesh-cpp.a', libs_path)

makedirs(libs_path)
copy_libs()
