#!/usr/bin/env python3

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

# build_mode = ["Debug", "Release"]
# linux只编译Debug
build_mode = ["Debug"]

for mode in build_mode:
    set_build_folder_name(mode + '_' + arch)

    libs_dir = os.path.abspath(os.path.join(
        '../../out/lib/linux/'+mode + '_'+arch))

    print('Architecture:', arch)
    print('Build mode:', mode)

    cmake_cmd = [
        'cmake',
        '-B', BUILD_DIR,
        '-DPS_BUILD_MODE=' + mode,
        '-DEVENT__DISABLE_OPENSSL=true',
        '-DEVENT__DISABLE_TESTS=true',
        '../..'
    ]

    call(cmake_cmd)

    global BUILD_MODE
    BUILD_MODE = mode
    build('libprotobuf')
    build('grpc++')
    build("event_static")
    build('event_core_static')
    build('jsoncpp_lib_static')
    build('push_sdk')

    def copy_protobuf_lib():
        copy_file(
            BUILD_DIR + '/3rdparty/grpc/third_party/protobuf/libprotobuf.a', libs_dir)

    def copy_ssl_lib():
        copy_file(
            BUILD_DIR + '/3rdparty/grpc/third_party/boringssl/crypto/libcrypto.a', libs_dir)
        copy_file(
            BUILD_DIR + '/3rdparty/grpc/third_party/boringssl/ssl/libssl.a', libs_dir)

    def copy_grpc_lib():
        copy_file(BUILD_DIR + '/3rdparty/grpc/libaddress_sorting.a', libs_dir)
        copy_file(BUILD_DIR + '/3rdparty/grpc/libgpr.a', libs_dir)
        copy_file(BUILD_DIR + '/3rdparty/grpc/libgrpc++.a', libs_dir)
        copy_file(BUILD_DIR + '/3rdparty/grpc/libgrpc.a', libs_dir)
        copy_file(
            BUILD_DIR + '/3rdparty/grpc/third_party/cares/cares/lib/libcares.a', libs_dir)
        copy_file(BUILD_DIR + '/3rdparty/grpc/third_party/zlib/libz.a', libs_dir)

    def copy_libevent_lib():
        copy_file(BUILD_DIR + '/3rdparty/libevent/lib/libevent.a', libs_dir)
        copy_file(BUILD_DIR + '/3rdparty/libevent/lib/libevent_core.a', libs_dir)

    def copy_jsoncpp_lib():
        copy_file(BUILD_DIR + '/3rdparty/jsoncpp/src/lib_json/libjsoncpp.a', libs_dir)

    def copy_push_sdk_lib():
        copy_file(BUILD_DIR + '/src/libpush_sdk.a', libs_dir)

    makedirs(libs_dir)
    copy_libs()
