/*
 * pcapknock.h
 * Function definition for pcapknock
 * By J. Stuart McMurray
 * Created 20190322
 * Last Modified 20190325
 */

#ifndef HAVE_PCAPKNOCK_H
#define HAVE_PCAPKNOCK_H

/* pcapknock is the function which starts all the action */
int pcapknock(void);

/* start_capture spawns a thread to start the capturing */
int start_capture(void);

#endif /* #ifndef HAVE_PCAPKNOCK_H */
