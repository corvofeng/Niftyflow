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
 *
 * 2018-03-23: 开始将trace数据存入数据库, 有关C语言访问数据库, 请查看以下几个
 *              文档
 *
 * https://github.com/libgit2/libgit2-backends/blob/master/mysql/mysql.c
 * https://dev.mysql.com/doc/refman/5.7/en/mysql-stmt-execute.html
 * https://dev.mysql.com/doc/refman/5.5/en/c-api-prepared-statement-type-codes.html
 *
 */
#ifndef TRANS_H_9TSB0WUC
#define TRANS_H_9TSB0WUC

#include "trace.h"
#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <hiredis/hiredis.h>

// just going to input the general details and not the port numbers
typedef struct connection_details
{
    char *server;
    char *user;
    char *password;
    char *database;
} connection_details;


void trans_test();
bool mysql_test();
bool redis_test();

MYSQL_RES* mysql_perform_query(MYSQL *connection, char *sql_query);
MYSQL* mysql_connection_setup(struct connection_details mysql_details);

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
 * | trace_data    | blob         | trace数据信息, 保存为二进制字符串 |
 * | fdate         | int(11)      | 存入日期, 如果数据量过大则使用索引|
 * | is_loop       | int(11)      | 是否有环                          |
 * | is_drop       | int(11)      | 是否丢包                          |
 * | is_probe      | int(11)      | 是否为探针                        |
 * +---------------+--------------+-----------------------------------+
 *
 */
void save_trace(MYSQL* conn, PKT_TRACE_T* trace);

#endif /* end of include guard: TRANS_H_9TSB0WUC */
