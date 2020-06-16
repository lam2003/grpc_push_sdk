#!/usr/bin/env python
from __future__ import print_function
import sys
import os
import argparse
import shutil

filename = '../build_common.py'
if sys.version_info[0] <= 2:
    execfile(filename)
else:
    exec(compile(open(filename, "rb").read(), filename, 'exec'))
init_common(os.path.abspath('..'), 'windows')


system_version = '10.0'
generator = 'Visual Studio 16 2019'
vs_year = 2019
toolset = 'v142'

arch_list = ['x86', 'x64']
build_mode = ['Debug', 'Release']

for arch in arch_list:
    for mode in build_mode:
        libs_dir = os.path.abspath(
            '../../out/lib/windows/' + mode + '_'+arch)
        set_build_folder_name(mode + '_'+arch)

        print('Architecture:', arch)
        print('Build mode:', mode)
        print('Toolset:', toolset)

        if arch == 'x64' and vs_year < 2019:
            generator += ' Win64'

        cmake_cmd = ['cmake',
                     '-B', BUILD_DIR,
                     '-G', generator,
                     '-T', toolset,
                     '-DCMAKE_SYSTEM_VERSION=' + system_version,
                     '-DPS_BUILD_SHARED=true',
                     '-DPS_BUILD_MODE=' + mode,
                     '-DEVENT__DISABLE_OPENSSL=true',
                     '-DEVENT__DISABLE_TESTS=true',
                     '-config=' + mode
                     ]
        if vs_year >= 2019:
            cmake_cmd.append('-A')

            if arch == 'x64':
                cmake_cmd.append('x64')
            else:
                cmake_cmd.append('Win32')

        cmake_cmd.append('../..')
        call(cmake_cmd)

        global BUILD_MODE
        BUILD_MODE = mode
        build('libprotobuf')
        build('grpc++')
        build('event_static')
        build('push_sdk')

        if mode == 'Debug':
            libs_postfix = 'd'
        else:
            libs_postfix = ''

        def copy_push_sdk_lib():
            copy_file(BUILD_DIR + '/src/'+mode+'/push_sdk' +
                      libs_postfix+'.dll', libs_dir)
            copy_file(BUILD_DIR + '/src/'+mode+'/push_sdk' +
                      libs_postfix+'.lib', libs_dir)

        def copy_protobuf_lib():
            copy_file(BUILD_DIR + '/3rdparty/grpc/third_party/protobuf/' +
                      mode + '/libprotobuf'+libs_postfix+'.lib', libs_dir)

        def copy_ssl_lib():
            copy_file(BUILD_DIR + '/3rdparty/grpc/third_party/boringssl/ssl/' +
                      mode + '/ssl'+libs_postfix+'.lib', libs_dir)
            copy_file(BUILD_DIR + '/3rdparty/grpc/third_party/boringssl/crypto/' +
                      mode + '/crypto'+libs_postfix+'.lib', libs_dir)

        def copy_grpc_lib():
            copy_file(BUILD_DIR + '/3rdparty/grpc/' + mode +
                      '/grpc' + libs_postfix + '.lib', libs_dir)
            copy_file(BUILD_DIR + '/3rdparty/grpc/' + mode +
                      '/grpc++' + libs_postfix + '.lib', libs_dir)
            copy_file(BUILD_DIR + '/3rdparty/grpc/' + mode +
                      '/gpr' + libs_postfix + '.lib', libs_dir)
            copy_file(BUILD_DIR + '/3rdparty/grpc/' + mode +
                      '/address_sorting' + libs_postfix + '.lib', libs_dir)
            copy_file(BUILD_DIR + '/3rdparty/grpc/third_party/cares/cares/lib/'+mode+'/cares' +
                      libs_postfix + '.lib', libs_dir)
            copy_file(BUILD_DIR + '/3rdparty/grpc/third_party/zlib/'+mode+'/zlibstatic' +
                      libs_postfix + '.lib', libs_dir)

        def copy_libevent_lib():
            copy_file(BUILD_DIR + '/3rdparty/libevent/lib/'+mode+'/event' +
                      libs_postfix + '.lib', libs_dir)

        makedirs(libs_dir)
        copy_libs()
