/*
 * debug.c
 * Debug-printing functions
 * Created 20190322
 * Last Modified 20190322
 */

#ifdef DEBUG
#include <err.h>
#include <stdarg.h>
void dbg(const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vwarn(fmt, args);
}
void dbgx(const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vwarnx(fmt, args);
}
#else /* ! #ifdef DEBUG */
void dbg(const char *fmt, ...) {return;}
void dbgx(const char *fmt, ...) {return;}
#endif /* #ifdef DEBUG */
