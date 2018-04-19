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
 * 2018-04-19: 修改了时间戳的计算方式, 再次感谢细心的师姐.
 *
 * 2018-04-11: 经过王鑫师姐的提醒, timeval.tv_usec的单位为微秒,
 *          换算成毫秒需要除以1000. 感谢师姐的细心指出
 * 2018-03-24: trace数据结构中的src_ip 与dst_ip数据结构进行了修改. 天啊!!!
 *
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */

typedef struct{
    struct in_addr ip_src; /**< 32bits 源IP地址   */
    struct in_addr ip_dst; /**< 32bits 目的IP地址 */
    uint16_t ip_id;        /**< 16bits 标识符     */
    uint8_t protocol;      /**< 8bits  协议字段   */
} __attribute__((packed)) IP_PKT_KEY_T;

enum {TRACE_CNT = 5};   /**< 只记录5跳信息 */

/**
 * @brief trace数据结构
 */
typedef struct{
    IP_PKT_KEY_T key;

    uint16_t pkt_size;   /**< 16bits 数据包大小  */

    /**
     * 32bits 收到第一个报文的时间戳: 如果使用秒级的计数单位, 是无法刻画出真实
     * 的数据包的时间情况, 这里的时间戳是毫秒级别的时间戳.
     *
     *  Timestamp记录收到报文的时间,
     *  如果对于某一跳交换机超过1秒还没收到其它的报文, 则视为丢包
     */
    uint32_t timestart;

    struct {
        uint16_t switch_id: 12;
        uint16_t hop_rcvd : 2;
        uint16_t hop_timeshift: 10; /**< 与timestart相减得到的偏移, 也为ms */
    } __attribute__((packed)) hop_info[TRACE_CNT - 1];

    uint16_t hp1_switch_id: 12; /**< 第一跳交换机id          */
    uint16_t hp1_rcvd: 2;       /**< 第一跳交换机收到的报文数  */
    uint16_t used: 1;           /**< 以hash表进行存储,
                                     记录hash表中的当前元素是否被占用. */

    uint16_t is_loop: 1;
    uint16_t is_drop: 1;
    uint16_t is_timeout: 1;
    uint16_t is_probe: 1;

    uint16_t reserved : 5;
} __attribute__((packed)) PKT_TRACE_T;


/**
 * 获取基于偏移的毫秒数.
 */
uint32_t get_time_shift(const struct timeval& tv,
                            const struct timeval& start);

// 只比较前12位, 不论是什么类型
#define IS_SAME_ID(id1, id2)  (((id1) & 0x0FFF) == ((id2) & 0x0FFF))

/**
 * 获取hop_timeshift的确切值, 这个函数存在的根本原因是timeshift为10位.
 *
 * 考虑下面一种情况:
 *    如果数据包的时间发生了混乱, trace中的timestart其实并不是第一跳的时间
 *  那么hop_timeshift就有可能为负值, 在保存时, 我们做了截断, 也就是说
 *  最高位会被切掉.
 *
 *   以`-2`为例, 32位的int表示-2会以`11...1110`这种形式来表示, 因为我们将最高
 * 位设计为了符号位, 但在截断之后, 我们保存的数字应该是`1111111110`,
 * 这样会出现什么问题呢? 10位的数字计算机中是没有定义符号位的. 我们就会认为
 * 他是一个很大的正数. 这里, 我们需要首先对其进行有符号扩展, 扩展至32位符号数.
 *
 * 其实我想说, 不通配的数据结构定义会导致无穷无尽的问题,
 */
#define GET_EXACT_TIME_SHIFT(_fake_shift, _exact_shift) do { \
    unsigned int fake = (_fake_shift);                       \
    int&  exact = (_exact_shift);                            \
    if(fake& 0x00000200) {                                   \
        exact = 0XFFFFFC000 | fake ;                         \
    } else {                                                 \
        exact = 0x0 | fake;                                  \
    }                                                        \
} while(0)

// tv_sec % (24 * 3600)  计算当天偏移的秒数
#define GET_TIMESTAMP_OF_DAY(tv)            \
            ( (((tv.tv_sec) % (24 * 3600)) + 28800) * 1000 + (tv.tv_usec) / 1000)

#define GET_CUR_DAY(tv) ((tv.tv_sec) - (tv.tv_sec)%(24 * 3600))

#endif /* end of include guard: TRACE_H_PAJBWR5N */
