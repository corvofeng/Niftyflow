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

#ifndef TRANS_H_9TSB0WUC
#define TRANS_H_9TSB0WUC



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

#endif /* end of include guard: TRANS_H_9TSB0WUC */
