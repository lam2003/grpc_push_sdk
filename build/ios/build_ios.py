#!/usr/bin/env python3

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
build_mode = ['Debug', 'Release']

deployment_target = '10.9'
generator = 'Unix Makefiles'

for mode in build_mode:
    make_universal_list = []
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
    event_libs = []
    event_core_libs = []
    jsoncpp_libs = []

    cmake_toolchain_path = ''
    for arch in arch_list:
        if arch == 'x86_64':
            cmake_toolchain_path = os.path.abspath(
                '../../cmake/ios.simulator.toolchain.cmake')
        else:
            cmake_toolchain_path = os.path.abspath(
                '../../cmake/ios.toolchain.cmake')

        libs_dir = os.path.abspath('../../out/lib/ios/' + mode)
        set_build_folder_name(mode+'_'+arch)

        print('Architecture:', arch)
        print('Build mode:', mode)

        # generate projects
        cmake_cmd = ['cmake',
                     '-B',
                     BUILD_DIR,
                     '-DAPPLE_IOS=YES',
                     '-DCMAKE_TOOLCHAIN_FILE='+cmake_toolchain_path,
                     '-DCMAKE_OSX_DEPLOYMENT_TARGET=' + deployment_target,
                     '-DCMAKE_OSX_ARCHITECTURES=' + arch,
                     '-Dprotobuf_BUILD_PROTOC_BINARIES=OFF',
                     '-DgRPC_BUILD_CODEGEN=OFF',
                     '-DCARES_INSTALL=OFF',
                     '-DENABLE_BITCODE=FALSE',
                     '-DENABLE_ARC=TRUE',
                     '-DPS_BUILD_MODE=' + mode,
                     '-DEVENT__DISABLE_OPENSSL=true',
                     '-DEVENT__DISABLE_TESTS=true',
                     '-G' + generator,
                     '../..'
                     ]
        if arch == 'x86_64':
            cmake_cmd.append('-DCMAKE_OSX_SYSROOT=iphonesimulator')

        call(cmake_cmd)

        global BUILD_MODE
        BUILD_MODE = mode
        build('libprotobuf')
        build('grpc++')
        build('event_static')
        build('event_core_static')
        build('jsoncpp_lib_static')
        build('push_sdk')

        service_mesh_cpp_libs     .append(
            BUILD_DIR + '/src/libpush_sdk.a')
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
        event_libs          .append(
            BUILD_DIR + '/3rdparty/libevent/lib/libevent.a'
        )
        event_core_libs     .append(
            BUILD_DIR + '/3rdparty/libevent/lib/libevent_core.a'
        )
        jsoncpp_libs        .append(
            BUILD_DIR + '/3rdparty/jsoncpp/src/lib_json/libjsoncpp.a'
        )

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

    def copy_libevent_lib():
        make_universal_list.append(event_libs)
        make_universal_list.append(event_core_libs)

    def copy_jsoncpp_lib():
        make_universal_list.append(jsoncpp_libs)

    def copy_push_sdk_lib():
        make_universal_list.append(service_mesh_cpp_libs)

    makedirs(libs_dir)
    copy_libs()
    for libs_list in make_universal_list:
        create_universal_lib(libs_list, libs_dir)
