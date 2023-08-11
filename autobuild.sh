#!/bin/bash
set -e
rm -rf `pwd`/build/*   #删除所有build中的文件
cd `pwd`/build &&
    cmake .. &&
    make
cd ..
cp -r `pwd`/src/include/ `pwd`/lib  #静态库