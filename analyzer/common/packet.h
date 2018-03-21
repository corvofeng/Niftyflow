/*
 *=======================================================================
 *    Filename:packet.h
 *
 *     Version: 1.0
 *  Created on: March 19, 2018
 *
 *      Author: corvo
 *=======================================================================
 */

#ifndef PACKET_H_WL06DAMQ
#define PACKET_H_WL06DAMQ

/**
 *  2018-03-21: 将头文件<asm/byteorder.h>去掉.
 *
 *  2018-03-20: 在解析GRE数据包时, 发现了大端小端的问题, 数据包中默认为大端
 *              字节序, 而我们系统上为小端字节序. **对于超过8位的数据**,
 *              若是不进行字节序转换, 将会出现数据包不匹配.
 *
 *              一个典型问题就是GRE中的 **Protocol type**, 它是一个16位的数据
 *              在使用时, 需要将其重新拼接, 假如使用IP包中的源IP与目的IP, 需要
 *              进行转换.
 *
 *              对于8位及以下的数据将不会存在这个问题, 不必过分担心, 望周知.
 *
 *
 *  2018-03-19:  **吐槽一下**: 今天的工作白做了, 当我发现了下面的网页
 *              http://www.tcpdump.org/sniffex.c, 我发现自己傻得像个两百斤的胖子
 *              而后又发现了 "/usr/include/netinet/in.h",
 *              我开始明白, 自己其实是个两百五十斤的胖子.
 */

#include <pcap.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include "con_queue.h"
//#include <asm/byteorder.h>


/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN 6

/* Ethernet header */
struct sniff_ethernet {
        u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
        u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
        uint16_t ether_type;                     /* IP? ARP? RARP? etc */
};

/* IP header
 * https://en.wikipedia.org/wiki/IPv4#Header, 在数据包中, 必须考虑数据包的大小端
 * 问题
 *
 */
struct sniff_ip {
        u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
        u_char  ip_tos;                 /* type of service */
        uint16_t ip_len;                 /* total length */
        uint16_t ip_id;                  /* identification */
        uint16_t ip_off;                 /* fragment offset field */
        #define IP_RF 0x8000            /* reserved fragment flag */
        #define IP_DF 0x4000            /* dont fragment flag */
        #define IP_MF 0x2000            /* more fragments flag */
        #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
        u_char  ip_ttl;                 /* time to live */
        u_char  ip_p;                   /* protocol */
        uint16_t ip_sum;                 /* checksum */
        struct  in_addr ip_src,ip_dst;  /* source and dest address */
};

#define IP_V(ip)                (((ip)->ip_vhl) >> 4)
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
        uint16_t th_sport;               /* source port */
        uint16_t th_dport;               /* destination port */
        tcp_seq th_seq;                 /* sequence number */
        tcp_seq th_ack;                 /* acknowledgement number */
        u_char  th_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
        u_char  th_flags;
        #define TH_FIN  0x01
        #define TH_SYN  0x02
        #define TH_RST  0x04
        #define TH_PUSH 0x08
        #define TH_ACK  0x10
        #define TH_URG  0x20
        #define TH_ECE  0x40
        #define TH_CWR  0x80
        #define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
        uint16_t th_win;                 /* window */
        uint16_t th_sum;                 /* checksum */
        uint16_t th_urp;                 /* urgent pointer */
};


/**
 * @brief https://en.wikipedia.org/wiki/Generic_Routing_Encapsulation
 */
struct sniff_std_gre {
    uint16_t gre_mhl;   /* C KS Reserved0  Version    */
    uint16_t gre_type;
    uint16_t gre_sum; /* optional */
    uint16_t gre_res1; /* optional */
    uint32_t gre_key; /* optional */
    uint32_t gre_seq; /* optional */
};

#define GRE_C(gre)               (((gre)->gre_mhl) & 0x8000)
#define GRE_K(gre)               (((gre)->gre_mhl) & 0x4000)
#define GRE_K(gre)               (((gre)->gre_mhl) & 0x4000)
#define GRE_RES0(gre)            (((gre)->gre_mhl) >> 3)
#define GRE_S(gre)               (((gre)->gre_mhl) & 0x2000)
#define GRE_V(gre)               (((gre)->gre_mhl) & 0x0007)

// JUST for little little endian
#define GRE_TYPE(gre)            (((gre)->gre_type >> 8)|((gre)->gre_type << 8) & 0xFFFF)


// Just a copy from #include <linux/if_ether.h>
#define ETH_P_ERSPAN	0x88BE		/* ERSPAN type II		*/

/**
 * **只读, 不可写**
 *    所有的数据包首先会被解析成如下格式, 以供之后的快路径, 慢路径使用
 * @brief Only readable, do not modify the content
 */
struct PARSE_PKT{
    struct sniff_ethernet *eth_outer;   // 内外层的eth头部
    struct sniff_ethernet *eth_inner;

    struct sniff_std_gre *gre;          // GRE头部

    struct sniff_ip *ip_outer;          // 内外层的IPv4头部
    struct sniff_ip *ip_inner;

    struct sniff_tcp * tcp;            // 最内层的TCP头部

    u_char *_data;

    struct pcap_pkthdr header;         // pcap头部, 可以查询时间

    PARSE_PKT(): _data(NULL) {}
    ~PARSE_PKT() {
        if(_data) free(_data);
    }
};

void PARSE_PKT_print(PARSE_PKT* p) ;

void
got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);


typedef Queue<std::shared_ptr<PARSE_PKT>> PKT_QUEUE;

#endif /* end of include guard: PACKET_H_WL06DAMQ */
