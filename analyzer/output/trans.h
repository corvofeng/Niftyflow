/*
 *=======================================================================
 *    Filename:trans.h
 *
 *     Version: 1.0
 *  Created on: March 18, 2018
 *
 *      Author: corvo
 *=======================================================================
 */

/**
 * 2018-03-30: 今天想起了save_trace中使用了太多的cJSON_Print, 造成了严重的内存
 *              泄露
 *
 * 2018-03-23: 开始将trace数据存入数据库, 有关C语言访问数据库, 请查看以下几个
 *              文档
 *
 * https://github.com/libgit2/libgit2-backends/blob/master/mysql/mysql.c
 * https://dev.mysql.com/doc/refman/5.7/en/mysql-stmt-execute.html
 * https://dev.mysql.com/doc/refman/5.5/en/c-api-prepared-statement-type-codes.html
 */
#ifndef TRANS_H_9TSB0WUC
#define TRANS_H_9TSB0WUC

#include "trace.h"
#include "conf.h"
#include <stdio.h>
#include <map>
#include <memory>
#include <stdlib.h>
#include <mysql.h>
#include <hiredis/hiredis.h>

bool mysql_test();
bool redis_test(const Conf* c);

MYSQL_RES* mysql_perform_query(MYSQL *connection, char *sql_query);
MYSQL* mysql_connection_setup(const Conf* c);
redisContext* redis_connection_setup(const Conf* c);

/**
 * 将一条trace路径存入数据库中, 这里需要依照数据库结构进行存储.
 *
 * +---------------+--------------+-----------------------------------+
 * | Field         | Type         | Comment                           |
 * +---------------+--------------+-----------------------------------+
 * | id            | int(11)      |                                   |
 * | s_ip          | varchar(120) | 源IP                              |
 * | d_ip          | varchar(120) | 目的IP                            |
 * | protocal      | int(11)      | 协议类型                          |
 * | generate_time | timestamp    | 产生时间                          |
 * | trace_data    | varchar(4096)| trace数据信息, 保存为JSON字符串   |
 * | fdate         | int(11)      | 存入日期, 如果数据量过大则使用索引|
 * | is_loop       | int(11)      | 是否有环                          |
 * | is_drop       | int(11)      | 是否丢包                          |
 * | is_probe      | int(11)      | 是否为探针                        |
 * +---------------+--------------+-----------------------------------+
 *
 */
void save_trace(MYSQL* conn, PKT_TRACE_T* trace);


/**
 *
 * +---------------+---------------+--------------------+
 * | Field         | Type          | Comment            |
 * +---------------+---------------+--------------------+
 * | id            | int(11)       |                    |
 * | rule_id       | int(11)       | 计数器规则ID       |
 * | generate_time | timestamp     | 数据产生时间(自动) |
 * | analyzer_id   | int(11)       | 分析器ID           |
 * | cnt           | int(11)       | 计数值             |
 * | fdate         | int(11)       | 数据产生日期(自动) |
 * +---------------+---------------+--------------------+
 */
class CounterRule;
class Counter;
void save_counter(MYSQL* conn, int rule_id, int analyzer_id, int cnt);

#endif /* end of include guard: TRANS_H_9TSB0WUC */
