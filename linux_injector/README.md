PcapKnock Injector
==================
This is an injector for pcapknock.  It uses gdb and `memfd_create(2)` to shove
pcap into systemd.  It works on Linux.

For legal use only

Building
--------
Compile with
```bash
cc -DDOUBLEFORK -O2 -Wall --pedantic -o injectpk *.c -static
```

Omitting `-DDOUBLEFORK` will cause it to not fork and be sneaky-like, but may
be useful for debugging.

Running
-------
As root:
```
./pcapknock
```

It'd probably be good to put in an init script or something.
