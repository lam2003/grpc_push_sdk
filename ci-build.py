#!/usr/bin/env python3
from __future__ import print_function
import os
import sys
import shutil
import subprocess
import multiprocessing


def call(command, shell=False):
    print('calling:', str(command))
    res = subprocess.call(command, shell=shell)
    if res != 0:
        sys.exit(-1)


platform = sys.argv[3]

call("git submodule update --init --recursive", True)

if platform.lower() == 'ios':
    call("brew install go", True)
    call("cd build/ios/ && chmod +x ./build_ios.py && ./build_ios.py", True)
