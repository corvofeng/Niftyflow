/*
 *=======================================================================
 *    Filename:dpdk_adapter.h
 *
 *     Version: 1.0
 *  Created on: April 13, 2018
 *
 *      Author: corvo
 *=======================================================================
 */

#ifndef DPDK_ADAPTER_H_2EAD61OS
#define DPDK_ADAPTER_H_2EAD61OS

#define MAX_RX_QUEUE_PER_LCORE 16

#include <rte_common.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_ether.h>
#include <rte_ethdev.h>

/**
 * 2018-04-13: 创建DPDK初始化函数, 允许使用Conf进行初始化.
 */

class Conf;

/**
 * 每个逻辑核可以监听多个队列
 */
struct lcore_queue_conf {
    unsigned n_rx_port;                             /** 监听的队列数 */
    unsigned rx_port_list[MAX_RX_QUEUE_PER_LCORE];  /** 需要监听的队列id */
} __rte_cache_aligned;


/**
 * Per-port statistics struct
 */
struct l2fwd_port_statistics {
    uint64_t tx;
    uint64_t rx;
    uint64_t dropped;
} __rte_cache_aligned;

/**
 * @brief: DPDK的连接初始化工具 为当前的核添加队列
 */
void dpdk_initer(lcore_queue_conf* lcore_conf, Conf* conf);

/**
 * @brief: DPDK初始化检查
 */
void check_all_ports_link_status(Conf *conf);


/**
 * @brief 关闭DPDK端口
 */
void close_all_port(Conf* conf);

#endif /* end of include guard: DPDK_ADAPTER_H_2EAD61OS */
