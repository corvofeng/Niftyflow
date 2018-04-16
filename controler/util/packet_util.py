#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: ts=4 sw=4 tw=99 et foldmethod=indent:

"""
@Date   : April 16, 2018
@Author : corvo



Study from https://www.binarytides.com/raw-socket-programming-in-python-linux/

构造各种类型的注入报文. 构造这些报文只是为了注入到交换机中, 并不承担真正的通讯
功能.
有一点很重要, 这个需要ROOT权限 !!!
"""


import socket
from struct import *
from tornado.log import app_log
import sys


def checksum(msg):
    """ 计算TCP checksum, 参考如下:
    http://www.tcpipguide.com/free/t_TCPChecksumCalculationandtheTCPPseudoHeader-2.htm

    TCP checksum: 伪IP头部 + 未添加checksum的TCP头部 + 以及TCP数据所组成
    UDP checksum: 伪IP头部 + 未添加checksum的UDP头部 + 以及UDP数据所组成

    鉴于这是自己实现的, 可能会计算错误, 我在调试时, 使用Wireshark的
    TCP/UDP checksum功能进行调试.

    但有一点请注意, TCP, UDP中的Checksum不是大端序.
    """

    s = 0
    if len(msg) % 2 == 0:
        # loop taking 2 characters at a time
        for i in range(0, len(msg), 2):
            w = msg[i] + (msg[i+1] << 8 )
            s = s + w
    else:
        for i in range(0, len(msg) - 1, 2):
            w = msg[i] + (msg[i+1] << 8 )
            s = s + w
        w = msg[-1]
        s = s + w

    s = (s >> 16) + (s & 0xffff);
    s = s + (s >> 16);

    #complement and mask to 4 byte short
    s = ~s & 0xffff

    return s

def packet_print(raw_data):
    """ 16进制输出数据
    """
    return (' '.join('{:02x}'.format(x) for x in raw_data[:]))

def build_ipv4_header(ip_src, ip_dst, ip_proto):
    """
     IPV4数据包头部

      0                   1                   2                   3   
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |Version|  IHL  |Type of Service|          Total Length         |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |         Identification        |Flags|      Fragment Offset    |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |  Time to Live |    Protocol   |         Header Checksum       |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                       Source Address                          |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                    Destination Address                        |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                    Options                    |    Padding    |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    """
    # ip header fields
    ip_ihl = 5
    ip_ver = 4
    ip_tos = 0
    ip_tot_len = 0  # kernel will fill the correct total length
    ip_id = 54321   #Id of this packet
    ip_frag_off = 0
    ip_ttl = 255
    ip_proto = ip_proto
    ip_check = 0    # kernel will fill the correct checksum
    ip_saddr = socket.inet_aton(ip_src)
    ip_daddr = socket.inet_aton(ip_dst)
    ip_ihl_ver = (ip_ver << 4) + ip_ihl

    # the ! in the pack format string means network order
    ip_header = pack('!BBHHHBBH4s4s' ,  \
            ip_ihl_ver, ip_tos, ip_tot_len, \
            ip_id, ip_frag_off, \
            ip_ttl, ip_proto, ip_check, \
            ip_saddr, \
            ip_daddr)

    app_log.info(packet_print(ip_header))
    return ip_header

def build_gre_packet():
    """
    https://en.wikipedia.org/wiki/Generic_Routing_Encapsulation
    GRE数据包头部:
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |C| |K|S| Reserved0       | Ver |         Protocol Type         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |      Checksum (optional)      |       Reserved1 (Optional)    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                         Key (optional)                        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                 Sequence Number (Optional)                    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    扩展位的使用: 只包含`Key`, 不添加`Checksum`以及`Sequence Number`
    """
    IPV4_FRAME = 0x0800

    gre_c = 0
    gre_k = 1
    gre_s = 0

    gre_proto = IPV4_FRAME

    gre_CKS_R =  (gre_c << 7) + (0 << 6) +  (gre_k << 5) + (gre_s << 4)
    gre_R_VER = 0

    gre_KEY = 0x1234

    gre_packet = pack('!BBHL', gre_CKS_R, gre_R_VER, gre_proto, gre_KEY)

    return gre_packet


def build_tcp_header(ip_src, ip_dst, ip_proto, port_src, port_dst, user_data):
    """
    TCP 数据包必须计算checksum,
    https://blog.csdn.net/junlon2006/article/details/68924251

    TCP 数据包头部:
       0                   1                   2                   3   
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |          Source Port          |       Destination Port        |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                        Sequence Number                        |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    Acknowledgment Number                      |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Data |           |U|A|P|R|S|F|                               |
      | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
      |       |           |G|K|H|T|N|N|                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           Checksum            |         Urgent Pointer        |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    Options                    |    Padding    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                             data                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    """

    # tcp header fields
    tcp_source = port_src # source port
    tcp_dest = port_dst # destination port
    tcp_seq = 454
    tcp_ack_seq = 0
    tcp_doff = 5    # 4 bit field, size of tcp header, 5 * 4 = 20 bytes

    #tcp flags
    tcp_fin = 0
    tcp_syn = 1
    tcp_rst = 0
    tcp_psh = 1
    tcp_ack = 0
    tcp_urg = 0
    tcp_window = 5840  # socket.htons (5840)    #   maximum allowed window size
    tcp_check = 0
    tcp_urg_ptr = 0

    tcp_offset_res = (tcp_doff << 4) + 0
    tcp_flags = (tcp_fin << 0 )+ (tcp_syn << 1) + (tcp_rst << 2) \
              + (tcp_psh << 3) + (tcp_ack << 4) + (tcp_urg << 5)

    # the ! in the pack format string means network order
    # 未计算校验和时的数据包
    tcp_header = pack('!HHLLBBHHH',     \
            tcp_source, tcp_dest,       \
            tcp_seq,  \
            tcp_ack_seq,  \
            tcp_offset_res, tcp_flags,  tcp_window,  \
            tcp_check, tcp_urg_ptr)

    app_log.info('TCP len:{}, Date len:{}'.format(len(tcp_header), len(user_data)))
    tcp_length = len(tcp_header) + len(user_data)

    # 伪头部
    source_address = socket.inet_aton(ip_src)
    dest_address = socket.inet_aton(ip_dst)
    placeholder = 0
    protocol = ip_proto
    psh = pack('!4s4sBBH' , source_address , dest_address , placeholder , protocol , tcp_length);
    psh = psh + tcp_header + user_data.encode();

    tcp_check = checksum(psh)
    #print tcp_checksum
    # make the tcp header again and fill the correct checksum
    # Remember checksum is NOT in network byte order
    tcp_header = pack('!HHLLBBH' ,
                    tcp_source, tcp_dest,
                    tcp_seq,
                    tcp_ack_seq,
                    tcp_offset_res, tcp_flags,  tcp_window)  \
            + pack('H' , tcp_check)     \
            + pack('!H' , tcp_urg_ptr)

    app_log.info(packet_print(tcp_header))
    return tcp_header


def build_udp_header(ip_src, ip_dst, ip_proto, port_src, port_dst, user_data):
    """
    UDP数据包头部:
       0      7 8     15 16    23 24    31  
      +--------+--------+--------+--------+ 
      |     Source      |   Destination   | 
      |      Port       |      Port       | 
      +--------+--------+--------+--------+ 
      |                 |                 | 
      |     Length      |    Checksum     | 
      +--------+--------+--------+--------+ 
      |                                     
      |          data octets ...            
      +---------------- ...                 

        Checksum在IPv4中是可选的, 在IPv6中是必须的.
    """
    udp_len = len(user_data) + 8
    udp_header = pack('!HHHH', port_src, port_dst, udp_len, 0)
    app_log.info('UDP len:{}, Date len:{}'.format(len(udp_header), len(user_data)))

    # 伪头部
    source_address = socket.inet_aton(ip_src)
    dest_address = socket.inet_aton(ip_dst)
    placeholder = 0
    protocol = ip_proto
    psh = pack('!4s4sBBH' , source_address , dest_address , placeholder , protocol , udp_len);

    psh = psh + udp_header + user_data.encode();

    app_log.info(packet_print(psh))
    udp_check = checksum(psh)

    udp_header = pack('!HHH', port_src, port_dst, udp_len) + pack('H', udp_check)

    return udp_header

############################################################################
# 以下为 测试用例
############################################################################

ip_s = '192.168.0.101'
ip_d = '137.0.32.3'
user_data = 'How are you?'

def test_tcp():
    ip_proto = socket.IPPROTO_TCP
    ipv4_header = build_ipv4_header(ip_src=ip_s,     \
              ip_dst = ip_d,    \
              ip_proto= ip_proto)

    tcp_header = build_tcp_header(ip_src = ip_s,     \
                                  ip_dst = ip_d,    \
                                  ip_proto = ip_proto,   \
                                  port_src=123, port_dst=1234,  \
                                  user_data = user_data)

    inner_packet = ipv4_header + tcp_header + user_data.encode()

    return inner_packet

def test_udp():
    ip_proto = socket.IPPROTO_UDP
    ipv4_header = build_ipv4_header(ip_src=ip_s,     \
              ip_dst = ip_d,    \
              ip_proto= ip_proto)

    udp_header = build_udp_header(ip_src = ip_s,     \
                                  ip_dst = ip_d,    \
                                  ip_proto = ip_proto,   \
                                  port_src=123, port_dst=1234,  \
                                  user_data = user_data)

    inner_packet = ipv4_header + udp_header + user_data.encode()
    return inner_packet



def test_gre():
    ip_proto = socket.IPPROTO_TCP
    ipv4_header = build_ipv4_header(ip_src=ip_s,     \
              ip_dst = ip_d,    \
              ip_proto= ip_proto)

    tcp_header = build_tcp_header(ip_src = ip_s,     \
                                  ip_dst = ip_d,    \
                                  ip_proto = ip_proto,   \
                                  port_src=123, port_dst=1234,  \
                                  user_data = user_data)

    inner_packet = ipv4_header + tcp_header + user_data.encode()

    outer_ipv4 = build_ipv4_header(ip_src = '127.0.0.1',
            ip_dst = ip_d,
            ip_proto = socket.IPPROTO_GRE)

    outer_packet = outer_ipv4 + build_gre_packet() + inner_packet

    return outer_packet


def main():
    """测试程序

    """

    #create a raw socket
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_RAW)
    except socket.error as e:
        app_log.error('Socket could not be created. Error Code : {}'.format(e))
        sys.exit()

    # final full packet - syn packets dont have any data
    #Send the packet finally - the port specified has no effect
    s.sendto(test_tcp(), (ip_d, 0 ))    # put this in a loop if you want
    s.sendto(test_udp(), (ip_d, 0 ))    # put this in a loop if you want
    s.sendto(test_gre(), (ip_d, 0 ))    # put this in a loop if you want


if __name__ == "__main__":
    from tornado.options import define, options, parse_command_line
    parse_command_line()
    main()
