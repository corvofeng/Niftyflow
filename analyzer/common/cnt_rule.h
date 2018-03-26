/*
 *=======================================================================
 *    Filename:filter.h
 *
 *     Version: 1.0
 *  Created on: March 26, 2018
 *
 *      Author: corvo
 *=======================================================================
 */


#ifndef FILTER_H_JI4PYOA3
#define FILTER_H_JI4PYOA3

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */


/**
 *
 *  2018-03-26: 计数器规则部分我放在最后进行书写, 并不是因为他困难,
 *              只是我想最后在考虑filter相关的问题, filter需要与控制器部分
 *              进行交互, 在初始化时进行配置, 或是由控制器进行更新.
 */


/**
 * @brief 计数器规则: 由以下的一个结构体组成了一个规则, 满足规则的数据包将
 *          会被计数.
 */
struct CounterRule {
    struct in_addr ip_src; // 32bits 源IP地址
    struct in_addr ip_dst; // 32bits 目的IP地址
    int switch_id: 12;     // 交换机ID
    char protocol;         // 协议类型
};

#endif /* end of include guard: FILTER_H_JI4PYOA3 */
