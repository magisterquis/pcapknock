/*
 * pcapknock.h
 * Function definition for pcapknock
 * By J. Stuart McMurray
 * Created 20190322
 * Last Modified 20190322
 */

#ifndef HAVE_PCAPKNOCK_H
#define HAVE_PCAPKNOCK_H

/* pcapknock is the function which starts all the action */
__attribute__((constructor)) int pcapknock(void);

#endif /* #ifndef HAVE_PCAPKNOCK_H */
