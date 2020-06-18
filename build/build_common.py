#!/usr/bin/env python

from __future__ import print_function
import os
import sys
import shutil
import subprocess
import multiprocessing

BUILD_DIR = ''
TARGET_PLATFORM = ''  # mac, ios, android, windows, linux


def init_common(build_common_path, target_platform):
    if build_common_path.find(' ') >= 0:
        print('Error: space foud in path:', build_common_path)
        print('please remove spaces from path and try again')
        sys.exit(-1)

    global TARGET_PLATFORM
    TARGET_PLATFORM = target_platform

    src_include = os.path.realpath(
        os.path.join(build_common_path, '..', 'include'))
    src_third_party = os.path.realpath(
        os.path.join(build_common_path, '..', '3rdparty'))
    dest_include = os.path.realpath(os.path.join(
        build_common_path, '..', 'out', 'include'))

    makedirs(dest_include)
    copy_file(src_include+'/push_sdk.h', dest_include)


def call(command, shell=False):
    print('calling:', str(command))
    res = subprocess.call(command, shell=shell)
    if res != 0:
        sys.exit(-1)


def set_build_folder_name(name):
    global BUILD_DIR
    BUILD_DIR = os.path.abspath(name)
    makedirs(BUILD_DIR)


def build(target):
    print()
    print('building ' + target + '...')
    call(['cmake',
          '--build', BUILD_DIR,
          '--target', target,
          '--config', BUILD_MODE#,
        #   '--parallel', str(multiprocessing.cpu_count())
          ], shell=False)


def makedirs(dir):
    if not os.path.isdir(dir):
        os.makedirs(dir)

def bool2cmake(bVal):
    if bVal:
        return 'ON'
    else:
        return 'OFF'


def copy_file(src, dest):
    shutil.copy(src, dest)
    print('copied', os.path.basename(src))


def copy_folder(src, dest, replace=True):
    folder_name = os.path.basename(src)
    dest_full_path = os.path.join(dest, folder_name)
    if replace:
        if os.path.exists(dest_full_path):
            shutil.rmtree(dest_full_path)

    shutil.copytree(src, dest_full_path)
    print('copied', os.path.basename(src))


def copy_libs():
    print
    print('copying to release folder...')
    copy_protobuf_lib()
    copy_ssl_lib()
    copy_grpc_lib()
    copy_libevent_lib()
    copy_jsoncpp_lib()
    copy_push_sdk_lib()

# for ios
def create_universal_lib(libs,release_libs_dir):
    if len(libs) == 0:
        return

    name = os.path.basename(libs[0])
    print('creating universal library', name + ' ...')
    lipo_commands = ['lipo', '-create']
    for lib in libs:
        lipo_commands.append(lib)
    lipo_commands.append('-output')
    lipo_commands.append(release_libs_dir + '/' + name)
    call(lipo_commands)
