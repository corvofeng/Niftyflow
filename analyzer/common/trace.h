/*
 *=======================================================================
 *    Filename:trace.h
 *
 *     Version: 1.0
 *  Created on: March 18, 2018
 *
 *      Author: corvo
 *=======================================================================
 */

#ifndef TRACE_H_PAJBWR5N
#define TRACE_H_PAJBWR5N

#include <sys/types.h>

typedef struct{
    u_int32_t src_ip;    //源 IP 地址,32bits
    u_int32_t dst_ip;    //目的 IP 地址,32bits
    u_int16_t ip_ID;     //标识符,16 bits
    u_int8_t protocol;   //协议字段,8 bits
}IP_PKT_KEY_T;

/**
 * @brief trace数据结构
 */
typedef struct{
    IP_PKT_KEY_T key;
    u_int16_t pkt_size;   // 数据包大小

    u_int32_t hop1_timestamp; // 收到第一个报文的时间戳:
                               // Timestamp记录收到报文的时间,
                               // 如果对于某一跳交换机超过1秒还没收到其报文,
                               // 则视为丢包

    u_int16_t switch_id[5];      // 只保留交换机id
    u_int16_t hop1_rcvd : 2;     // 交换机收到的报文数

    u_int16_t used : 1;          // 以hash表进行存储,
                                // 记录hash表中的当前元素是否被占用.
    u_int16_t rcvd : 5;
    u_int16_t hop2_rcvd : 2;
    u_int16_t hop3_rcvd : 2;
    u_int16_t hop4_rcvd : 2;
    u_int16_t hop5_rcvd : 2;
    u_int16_t hop2_timestamp : 10;
    u_int16_t hop3_timestamp : 10;
    u_int16_t hop4_timestamp : 10;
    u_int16_t hop5_timestamp : 10;
} PKT_TRACE_T;




#endif /* end of include guard: TRACE_H_PAJBWR5N */
