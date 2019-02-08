/*
 * main.c
 * Stub code to turn pcapknock into a program
 * By J. Stuart McMurray
 * Created 20180308
 * Last Modified 20180310
 */

#ifdef DOUBLEFORK
#include <sys/wait.h>
#include <fcntl.h>
#include <err.h>
#endif /* #ifdef DOUBLEFORK */

#include <pthread.h>
#include <unistd.h>

#include "pcapknock.h"

int
main(int argc, char **argv) {
        int ret;

        /* Double-fork if we ought */
#ifdef DOUBLEFORK
        pid_t pid;
        int i, fd;

        /* Close all output if we're making a dissociated child */
        for (i = 0; i <= CLMAXFD; ++i)
                close(i);
        if (-1 != (fd = open("/dev/null", O_RDWR))) {
                dup2(STDIN_FILENO, fd);
                dup2(STDOUT_FILENO, fd);
                dup2(STDERR_FILENO, fd);
                if (fd != STDIN_FILENO && fd != STDOUT_FILENO &&
                                fd != STDERR_FILENO)
                        close(fd);
        }

        /* First of two forks */
        switch (pid = fork()) {
                case 0: /* Child */
                        break;
                case -1: /* Error */
                        return 1;
                default: /* Parent */
                        /* Wait on the child and we're done */
                        waitpid(pid, NULL, 0);
                        /* Go back to processing packets */
                        return 0;
        }

        /* In middle child */

        /* Disassociate from parent, and ignore child death */
        if (-1 == setsid())
                return 6;
        if (SIG_ERR == signal(SIGCHLD, SIG_IGN))
                return 7;

        /* Fork the real process */
        switch (pid = fork()) {
                case 0: /* Child */
                        break;
                case -1: /* Error */
                        return 8; /* Shouldn't work */
                default: /* Parent */
                        /* Die, let the child do the work */
                        return 0;
        }

#endif /* #ifdef DOUBLEFORK */

        /* Fire off the listener */
        if (0 != (ret = pcapknock()))
                return ret;
        /* Main's done, keep listener going */
        pthread_exit(NULL);
}
