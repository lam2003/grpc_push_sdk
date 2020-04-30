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

service_mesh_cpp_libs = []
grpc_libs = []
grpcpp_libs = []
gpr_libs = []
address_sorting_libs = []
cares_libs = []
crypto_libs = []
ssl_libs = []
protobuf_libs = []
z_libs = []


for arch in arch_list:
    libs_dir = os.path.abspath(
        '../../out/lib/windows/' + toolset + '_'+BUILD_MODE + '_'+arch)
    set_build_folder_name(toolset + '_'+BUILD_MODE + '_'+arch)

    print('Architecture:', arch)
    print('Build mode:', BUILD_MODE)
    print('Toolset:', toolset)

    if arch == 'x64' and vs_year < 2019:
        generator += ' Win64'

    cmake_cmd = ['cmake',
                 '-B', BUILD_DIR,
                 '-G', generator,
                 '-T', toolset,
                 '-DCMAKE_SYSTEM_VERSION=' + system_version,
                 '-DPS_BUILD_SHARED=true',
                 '-DPS_BUILD_MODE='+ BUILD_MODE,
                 '-config=' + BUILD_MODE
                 ]
    if vs_year >= 2019:
        cmake_cmd.append('-A')

        if arch == 'x64':
            cmake_cmd.append('x64')
        else:
            cmake_cmd.append('Win32')

    cmake_cmd.append('../..')
    call(cmake_cmd)
    build('libprotobuf')
    build('grpc++')
    build('push_sdk')
