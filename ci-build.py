#!/usr/bin/env python3
from __future__ import print_function
import os
import sys
import shutil
import subprocess
import multiprocessing

SPECS_NAME = 'EduPushSDK'

def call_ignore_error(command, shell=False):
    print('call_ignore_error:', str(command))
    subprocess.call(command, shell=shell)


def call(command, shell=False):
    print('call:', str(command))
    res = subprocess.call(command, shell=shell)
    if res != 0:
        sys.exit(-1)


def makedirs(dir):
    if not os.path.isdir(dir):
        os.makedirs(dir)


platform = sys.argv[3]
output_dir = sys.argv[7]


call_ignore_error("brew install go", True)
call_ignore_error("brew upgrade go", True)
call_ignore_error("brew install cmake", True)
call_ignore_error("brew upgrade cmake", True)

call("git submodule update --init --recursive", True)
if platform.lower() == 'ios':
    call(
        "cd build/ios/ && chmod +x ./build_ios.py && ./build_ios.py", True)

makedirs(output_dir)

copycmd = 'zip -r ' + str(output_dir) + '/' + \
    str(SPECS_NAME)+'.zip ' + './out/*'
call(copycmd, True)
