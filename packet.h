/*
 * packet.h
 * Packet-processing
 * By J. Stuart McMurray
 * Created 20190322
 * Last Modified 20190322
 */

#ifndef HAVE_PACKET_H
#define HAVE_PACKET_H

/* handle_packet inspects the packet passed in by pcap_loop for a command and
 * handles it if found. */
void handle_packet(u_char *u, const struct pcap_pkthdr *hdr, const u_char *pkt);

/* These hold the size of the command/callback flags */
extern size_t cblen;
extern size_t cmdlen;

#endif /* #ifndef HAVE_PACKET_H */
