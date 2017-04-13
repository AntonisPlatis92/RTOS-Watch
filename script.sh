#!/bin/sh
clear
make clean; make
mcopy -o prexos a:
cd ..
qemu-system-i386 -fda ./prex-0.8.0.i386-pc.img
cd prex-0.9.0/



