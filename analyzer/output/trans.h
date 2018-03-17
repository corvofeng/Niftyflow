#ifndef TRANS_H
#define TRANS_H

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
void mysql_test();
void redis_test();
MYSQL_RES* mysql_perform_query(MYSQL *connection, char *sql_query);
MYSQL* mysql_connection_setup(struct connection_details mysql_details);

#endif
