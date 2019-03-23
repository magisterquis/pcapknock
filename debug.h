/*
 * debug.h
 * Debug-printing functions
 * Created 20190322
 * Last Modified 20190322
 */

#ifndef HAVE_DEBUG_H_JSM
#define HAVE_DEBUG_H_JSM

/* dbg is like warn but only works if DEBUG is defined. */
void dbg(const char *fmt, ...);
/* dbgx is like warnx but only works if DEBUG is defined. */
void dbgx(const char *fmt, ...);

#endif /* #define HAVE_DEBUG_H_JSM */
