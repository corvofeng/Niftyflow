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

/**
 * 2018-03-22: 添加了毫秒数的换算工具. int是32位的时间戳, 大概有$$21*10^{8}$$
 *          20×10^8 ÷ 3600 ÷ 24 ÷ 1000 等于23, 也就是说, 换算成毫秒表示时间,
 *          只能表示23天, 连一个月都做不到.
 *
 *          这里, 我决定再精简一些, 将时间戳换算为基于当前一天的偏移,
 *          如果数据包中的日期是3月22日,那么trace中的时间戳表示的是
 *          从3-22日0点起经过的毫秒数.
 *
 * 2018-03-19: 直到今天trace的数据结构终于敲定
 */

#include <stdint.h>

typedef struct{
    uint32_t src_ip;    // 32bits 源IP地址
    uint32_t dst_ip;    // 32bits 目的IP地址
    uint16_t ip_id;     // 16bits 标识符
    uint8_t protocol;   // 8bits  协议字段
} __attribute__((packed)) IP_PKT_KEY_T;

/**
 * @brief trace数据结构
 */
typedef struct{
    IP_PKT_KEY_T key;

    uint16_t pkt_size;   // 16bits 数据包大小

    /**
     * 32bits 收到第一个报文的时间戳: 如果使用秒级的计数单位, 是无法刻画出真实
     * 的数据包的时间情况, 这里的时间戳是毫秒级别的时间戳.
     *
     *  Timestamp记录收到报文的时间,
     *  如果对于某一跳交换机超过1秒还没收到其报文,
     *  则视为丢包
     */
    uint32_t timestart;

    struct {
        uint16_t switch_id: 12;
        uint16_t hop_rcvd : 2;
        uint16_t hop_timeshift: 10; // 与timestart相减得到的偏移, 也为ms
    } __attribute__((packed)) hop_info[4];

    uint16_t hp1_switch_id: 12; // 第一跳交换机id
    uint16_t hp1_rcvd: 2;       // 第一跳交换机收到的报文数
    uint16_t used: 1;           // 以hash表进行存储,
                                  // 记录hash表中的当前元素是否被占用.

    uint16_t is_loop: 1;
    uint16_t is_drop: 1;
    uint16_t is_timeout: 1;
    uint16_t is_probe: 1;

    uint16_t reserved : 5;
} __attribute__((packed)) PKT_TRACE_T;


/**
 * 通过函数毫秒数
 */
uint32_t get_time_start(struct timeval tv);



#endif /* end of include guard: TRACE_H_PAJBWR5N */
