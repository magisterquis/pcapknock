/*
 * doublefork.h
 * Fork and dissociate
 * By J. Stuart McMurray
 * Created 20190322
 * Last Modified 20190322
 */

#ifndef HAVE_DOUBLEFORK_H
#define HAVE_DOUBLEFORK_H

/* doublefork forks twice, effectively creating a new process not very
 * associated with the parent.  The child will ignore SIGCHLD.  The parent
 * will get a return value of 1 and the child a return value of 0.  Any other
 * value indicates an error. */
int doublefork(void);

#endif /* #ifndef HAVE_DOUBLEFORK_H */
