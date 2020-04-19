#!/usr/bin/env python

from __future__ import print_function
import sys
import subprocess

 #导入编译配置
filename = '../build_config.py'
if sys.version_info[0] <= 2:
    execfile(filename)
else:
    exec(compile(open(filename, "rb").read(), filename, 'exec'))

def call(command):
    res = subprocess.call(command, shell=False)
    if res != 0:
        sys.exit(-1)

call(['python', 'build_linux.py'])