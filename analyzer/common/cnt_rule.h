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
 *  2018-03-29: 考虑过之后, 决定将计数器规则进行更严格一点的定义, 之后将会在
 *              规则中完善.
 *
 *  2018-03-26: 计数器规则部分我放在最后进行书写, 并不是因为他困难,
 *              只是我想最后在考虑filter相关的问题, filter需要与控制器部分
 *              进行交互, 在初始化时进行配置, 或是由控制器进行更新.
 */


/**
 * @brief 计数器规则: 由以下的一个结构体组成了一个规则, 满足规则的数据包将
 *          会被计数.
 *
 * **Attention**:
 *  对于每个计数器:  rule_id是必须要有的.
 *                   ip_srcip_dst 缺省:  0
 *                   switch_id    缺省: -1
 *                  协议类型      缺省: -1
 */
struct CounterRule {
    int rule_id;
    struct in_addr ip_src; /**<  32bits 源IP地址   */
    struct in_addr ip_dst; /**<  32bits 目的IP地址 */
    int switch_id: 12;     /**<  交换机ID          */
    unsigned char protocol;/**<  协议类型          */

    // 必须要求rule_id
    explicit CounterRule(int _rule_id):
        rule_id(_rule_id), switch_id(-1), protocol(-1) {
        this->ip_src.s_addr = 0;
        this->ip_dst.s_addr = 0;
    }

    CounterRule(const CounterRule& cr) {
        this->rule_id = cr.rule_id;
        this->ip_src = cr.ip_src;
        this->ip_dst = cr.ip_dst;
        this->switch_id = cr.switch_id;
        this->protocol = cr.protocol;
    }

    bool operator < (const CounterRule& r2) const {
        return this->rule_id < r2.rule_id;
    }

};

#endif /* end of include guard: FILTER_H_JI4PYOA3 */
