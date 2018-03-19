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
    u_int32_t src_ip;    // 32bits 源IP地址
    u_int32_t dst_ip;    // 32bits 目的IP地址
    u_int16_t ip_ID;     // 16bits 标识符
    u_int8_t protocol;   // 8bits  协议字段
} __attribute__((packed)) IP_PKT_KEY_T;

/**
 * @brief trace数据结构
 */
typedef struct{
    IP_PKT_KEY_T key;

    u_int16_t pkt_size;   // 16bits 数据包大小

    /**
     * 32bits 收到第一个报文的时间戳: 如果使用秒级的计数单位, 是无法刻画出真实
     * 的数据包的时间情况, 这里的时间戳是毫秒级别的时间戳, 每个时间都是基于
     * 当前年份的偏移.
     *
     *  Timestamp记录收到报文的时间,
     *  如果对于某一跳交换机超过1秒还没收到其报文,
     *  则视为丢包
     */
    u_int32_t timestart;

    struct {
        u_int16_t switch_id: 12;
        u_int16_t hop_rcvd : 2;
        u_int16_t hop_timeshift: 10; // 与timestart相减得到的偏移, 也为ms
    } __attribute__((packed)) hop_info[4];

    u_int16_t hp1_switch_id: 12; // 第一跳交换机id
    u_int16_t hp1_rcvd: 2;       // 第一跳交换机收到的报文数
    u_int16_t used: 1;           // 以hash表进行存储,
                                  // 记录hash表中的当前元素是否被占用.

    u_int16_t is_loop: 1;
    u_int16_t is_drop: 1;
    u_int16_t is_timeout: 1;
    u_int16_t is_probe: 1;

    u_int16_t reserved : 5;
} __attribute__((packed)) PKT_TRACE_T;

#endif /* end of include guard: TRACE_H_PAJBWR5N */
