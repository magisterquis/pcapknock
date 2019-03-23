/*
 * doublefork.c
 * Fork and dissociate
 * By J. Stuart McMurray
 * Created 20190322
 * Last Modified 20190322
 */

#include <sys/wait.h>

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"

/* doublefork forks twice, effectively creating a new process not very
 * associated with the parent.  The child will ignore SIGCHLD.  The parent
 * will get a return value of 1 and the child a return value of 0.  Any other
 * value indicates an error. */
int
doublefork(void)
{
        pid_t ret;
        int status;
        int i;

        /* Fork away from the main process */
        switch (ret = fork()) {
                case 0: /* Middle child */
                        break; /* Continues below */
                case -1: /* Error */
                        dbg("fork");
                        return 8;
                default: /* Parent */
                        /* Wait for child to finish, go about life */
                        if (-1 == waitpid(ret, &status, 0)) {
                                dbg("waitpid");
                                return 7;
                        }
                        return 1;
        }

        /* Between forks, disassociate */
        if (-1 == setsid()) {
                dbg("setsid");
                return 8;
        }
        if (SIG_ERR == signal(SIGCHLD, SIG_IGN)) {
                dbg("signal");
                return 9;
        }
        for (i = 0; i <= FD_SETSIZE; i++) {
                close(i);
        }
        /* Fork again, to lose parentage */
        switch (ret = fork()) {
                case 0: /* Ultimate child */
                        return 0; /* Success */
                case -1: /* Error, in middle */
                        dbg("fork");
                        exit(10);
                default: /* Middle */
                        exit(0);
        }
}
