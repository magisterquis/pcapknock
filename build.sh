#!/bin/sh
#
# build.sh
# Builder for pcapknock
# By J. Stuart McMurray
# Created 20180310
# Last Modified 20180310

set -e

CFLAGS="-O2 -Wall --pedantic"
LFLAGS="-lpcap -lpthread"
LIBNAME="pcapknock.so"
BINNAME="pcapknock"

# User-settable DEFINES
if [ -z $DEFINES ]; then
        DEFINES=""
fi 

# Get device from environment
if [ -n "$DEVICE" ]; then
        DEFINES="$DEFINES -DDEVICE=\"$DEVICE\""
fi

echo "C Flags:      \"$CFLAGS\""
echo "Defines:      \"$DEFINES\""
echo "Linker Flags: \"$LFLAGS\""

# Compile an injectable library
echo -n "Building library..."
cc $DEFINES -DCONSTRUCTOR $CFLAGS -fPIC -o $LIBNAME pcapknock.c $LFLAGS -shared
echo $LIBNAME

# Compile standalone binaries
echo -n "Building static binary..."
cc -DERROUT $DEFINES $CFLAGS -o $BINNAME.static pcapknock.c main.c $LFLAGS -static
echo $BINNAME.static
echo -n "Building dynamic binary..."
cc -DERROUT $DEFINES $CFLAGS -o $BINNAME.dynamic pcapknock.c main.c $LFLAGS
echo $BINNAME.dynamic
