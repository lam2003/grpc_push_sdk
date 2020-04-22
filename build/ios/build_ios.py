#!/usr/bin/env python

from __future__ import print_function
import os
import sys

filename = '../build_common.py'
if sys.version_info[0] <= 2:
    execfile(filename)
else:
    exec(compile(open(filename, "rb").read(), filename, 'exec'))
init_common(os.path.abspath('..'), 'ios')

arch_list = ['arm64',
             'armv7',
             'armv7s',
             'x86_64'
             ]

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

deployment_target = '8.0'
generator = 'Unix Makefiles'

for arch in arch_list:
    libs_dir = os.path.abspath('../../out/lib/ios/'+BUILD_MODE + '_'+arch)
    set_build_folder_name(BUILD_MODE + '_' + arch)

    # generate projects
    cmake_cmd = ['cmake',
                 '-B',
                 BUILD_DIR,
                 '-DCMAKE_SYSTEM_NAME=iOS',
                 '-DAPPLE_IOS=YES',
                 '-DCMAKE_OSX_DEPLOYMENT_TARGET=' + deployment_target,
                 '-DCMAKE_OSX_ARCHITECTURES=' + arch,
                 '-Dprotobuf_BUILD_PROTOC_BINARIES=OFF',
                 '-DgRPC_BUILD_CODEGEN=OFF',
                 '-DCARES_INSTALL=OFF',
                 '-DENABLE_BITCODE=FALSE',
                 '-DENABLE_ARC=TRUE',
                 '-G' + generator,
                 '../..'
                 ]
    if arch == 'x86_64':
        cmake_cmd.append('-DCMAKE_OSX_SYSROOT=iphonesimulator')

    call(cmake_cmd)

    build('service-mesh-cpp')

    service_mesh_cpp_libs     .append(
        BUILD_DIR + '/src/libservice-mesh-cpp.a')
    grpc_libs           .append(
        BUILD_DIR + '/3rdparty/grpc/libgrpc.a')
    grpcpp_libs         .append(
        BUILD_DIR + '/3rdparty/grpc/libgrpc++.a')
    gpr_libs            .append(
        BUILD_DIR + '/3rdparty/grpc/libgpr.a')
    address_sorting_libs.append(
        BUILD_DIR + '/3rdparty/grpc/libaddress_sorting.a')
    cares_libs          .append(
        BUILD_DIR + '/3rdparty/grpc/third_party/cares/cares/lib/libcares.a')
    crypto_libs         .append(
        BUILD_DIR + '/3rdparty/grpc/third_party/boringssl/crypto/libcrypto.a')
    ssl_libs            .append(
        BUILD_DIR + '/3rdparty/grpc/third_party/boringssl/ssl/libssl.a')
    z_libs              .append(
        BUILD_DIR + '/3rdparty/grpc/third_party/zlib/libz.a')
    protobuf_lib = BUILD_DIR + \
        '/3rdparty/grpc/third_party/protobuf/libprotobuf.a'
    if not os.path.exists(protobuf_lib):
        protobuf_lib = BUILD_DIR + \
            '/3rdparty/grpc/third_party/protobuf/cmake/libprotobuf.a'
        protobuf_libs.append(protobuf_lib)

    make_universal_list = []

    def copy_protobuf_lib():
        make_universal_list.append(protobuf_libs)

    def copy_ssl_lib():
        make_universal_list.append(ssl_libs)
        make_universal_list.append(crypto_libs)

    def copy_grpc_lib():
        make_universal_list.append(address_sorting_libs)
        make_universal_list.append(gpr_libs)
        make_universal_list.append(grpcpp_libs)
        make_universal_list.append(grpc_libs)
        make_universal_list.append(cares_libs)
        make_universal_list.append(z_libs)

    def copy_service_mesh_lib():
        make_universal_list.append(service_mesh_cpp_libs)

    makedirs(libs_dir)
    copy_libs()

    for libs_list in make_universal_list:
        create_universal_lib(libs_list)
