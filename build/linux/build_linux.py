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

bits, linkage = platform.architecture()
if bits == '64bit':
    arch = 'x64'
elif bits == '32bit':
    arch = 'x86'
else:
    arch = bits

set_build_folder_name(BUILD_MODE + '_' + arch)

libs_path = os.path.abspath(os.path.join(
    '../../out/lib/linux/'+BUILD_MODE + '_'+arch))

print('Architecture:', arch)
print('Build mode:', BUILD_MODE)
print('Libs path:', libs_path)

cmake_cmd = [
    'cmake',
    '-B', BUILD_DIR,
    '-DBUILD_MODE=' + BUILD_MODE,
    '../..'
]

call(cmake_cmd)
build('libprotobuf')
build('grpc++')
build('push_sdk')


def copy_protobuf_lib():
    protobuf_lib = BUILD_DIR + '/3rdparty/grpc/third_party/protobuf/libprotobuf.a'
    if not os.path.exists(protobuf_lib):
        protobuf_lib = BUILD_DIR + '/3rdparty/grpc/third_party/protobuf/cmake/libprotobuf.a'
    copy_file(protobuf_lib, libs_path)


def copy_ssl_lib():
    copy_file(
        BUILD_DIR + '/3rdparty/grpc/third_party/boringssl/crypto/libcrypto.a', libs_path)
    copy_file(
        BUILD_DIR + '/3rdparty/grpc/third_party/boringssl/ssl/libssl.a', libs_path)


def copy_grpc_lib():
    copy_file(BUILD_DIR + '/3rdparty/grpc/libaddress_sorting.a', libs_path)
    copy_file(BUILD_DIR + '/3rdparty/grpc/libgpr.a', libs_path)
    copy_file(BUILD_DIR + '/3rdparty/grpc/libgrpc++.a', libs_path)
    copy_file(BUILD_DIR + '/3rdparty/grpc/libgrpc.a', libs_path)
    copy_file(
        BUILD_DIR + '/3rdparty/grpc/third_party/cares/cares/lib/libcares.a', libs_path)
    copy_file(BUILD_DIR + '/3rdparty/grpc/third_party/zlib/libz.a', libs_path)


def copy_service_mesh_lib():
    copy_file(BUILD_DIR + '/src/libpush_sdk.a', libs_path)


makedirs(libs_path)
copy_libs()
