#include "log.h"
#include <assert.h>
#include "packet.h"

void PARSE_PKT_print(PARSE_PKT* p) {


}

/*
 * dissect/print packet
 */
void
got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{

    static int count = 1;                   /* packet counter */

    /* declare pointers to packet headers */
    const struct sniff_ethernet *ethernet;  /* The ethernet header [1] */
    const struct sniff_ip *ip;              /* The IP header */
    const struct sniff_tcp *tcp;            /* The TCP header */
    const struct sniff_std_gre *gre;        /* The GRE header */

    const char *payload;                    /* Packet payload */

    int size_ip;
    int size_tcp;
    int size_payload;

    printf("\nPacket number %d:\n", count);
    count++;

    /* define ethernet header */
    ethernet = (struct sniff_ethernet*)(packet);

    /* define/compute ip header offset */
    ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
    size_ip = IP_HL(ip) * 4;
    if (size_ip < 20) {
        LOG_D(FMT("   * Invalid IP header length: %u bytes\n", size_ip));
        return;
    }

    /* print source and destination IP addresses */
    LOG_D(FMT("       From: %s\n", inet_ntoa(ip->ip_src)));
    LOG_D(FMT("         To: %s\n", inet_ntoa(ip->ip_dst)));

    /* determine protocol */
    switch(ip->ip_p) {
        case IPPROTO_TCP:
            LOG_D("   Protocol: TCP\n");
            break;
        case IPPROTO_GRE:
            LOG_D("   Protocol: GRE\n");
            break;
        case IPPROTO_UDP:
            LOG_D("   Protocol: UDP\n");
            return;
        case IPPROTO_ICMP:
            LOG_D("   Protocol: ICMP\n");
            return;
        case IPPROTO_IP:
            LOG_D("   Protocol: IP\n");
            return;
        default:
            LOG_D("   Protocol: unknown\n");
            return;
    }

    /*
     *  OK, this packet is TCP.
     */

    /* define/compute tcp header offset */
    tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
    size_tcp = TH_OFF(tcp)*4;
    if (size_tcp < 20) {
        printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
        return;
    }

    printf("   Src port: %d\n", ntohs(tcp->th_sport));
    printf("   Dst port: %d\n", ntohs(tcp->th_dport));

    /* define/compute tcp payload (segment) offset */
    // payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);

    /* compute tcp payload (segment) size */
    // size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);

    /*
     * Print payload data; it might be binary, so don't just
     * treat it as a string.
     */
    // if (size_payload > 0) {
    //     printf("   Payload (%d bytes):\n", size_payload);
    //     // print_payload(payload, size_payload);
    // }

    return;
}


