#!/bin/sh

# make menuconfig
# ERROR: please fix package/nodog/Makefile

TOPDIR=$PWD make -C package/nodog/ DUMP=1
