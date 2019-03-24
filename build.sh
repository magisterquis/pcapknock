#!/bin/sh
#
# build.sh
# Build pcapknock for a variety of situations
# By J. Stuart McMurray
# Created 20190324
# Last Modified 20190325

set -e

# Options to pass to the C compiler
COPTS="-Os -Wall --pedantic"
OUTDIR="./built/$(uname -s)"

# Make sure we're in the same directory as the sources
if [ ! -r pcapknock.c ]; then
	echo "Source file pcapknock.c not found.  Sure you're in the right place?" >&2
	exit 1
fi

# Make sure we have an output directory
mkdir -p $OUTDIR

# Linux
if [ "Linux" = "$(uname -s)" ]; then
	PCAPDIR=./libpcap-1.9.0/built #Vendored libpcap
        INJDIR=./linux-injector

	# Build a standalone binary for debugging
	cc $COPTS -DDEBUG       -I$PCAPDIR/include -o "$OUTDIR/pcapknock.standalone.debug"  *.c $PCAPDIR/lib/libpcap.a -lpthread
	# Build a standalone non-daemonizing binary
	cc $COPTS               -I$PCAPDIR/include -o "$OUTDIR/pcapknock.standalone"        *.c $PCAPDIR/lib/libpcap.a -lpthread
	# Build a standalone daemonizing binary
	cc $COPTS -DDAEMON      -I$PCAPDIR/include -o "$OUTDIR/pcapknock.standalone.daemon" *.c $PCAPDIR/lib/libpcap.a -lpthread
	# Build an injectable library
	cc $COPTS -DCONSTRUCTOR -I$PCAPDIR/include -o "$OUTDIR/pcapknock.so"                *.c $PCAPDIR/lib/libpcap.a -lpthread -fPIC -shared
        # Build an injector
        (cd $OUTDIR; xxd -i pcapknock.so) > linux-injector/pcapknock.so.c
        cc -DDEBUG $COPTS -I$INJDIR -o $OUTDIR/pcapknock.pid1-injector $INJDIR/*.c -static
elif [ "OpenBSD" = "$(uname -s)" ] || [ "FreeBSD" = "$(uname -s)" ]; then
	# Build a standalone binary for debugging
	cc $COPTS -DDEBUG       -o "$OUTDIR/pcapknock.standalone.debug"  *.c -lpcap -lpthread
	# Build a standalone non-daemonizing binary
	cc $COPTS               -o "$OUTDIR/pcapknock.standalone"        *.c -lpcap -lpthread
	# Build a standalone daemonizing binary
	cc $COPTS -DDAEMON      -o "$OUTDIR/pcapknock.standalone.daemon" *.c -lpcap -lpthread
	# Build an injectable library
	cc $COPTS -DCONSTRUCTOR -o "$OUTDIR/pcapknock.so"                *.c -lpcap -lpthread -fPIC -shared
fi

echo "Compiled files are in $OUTDIR:"
ls -lart $OUTDIR
