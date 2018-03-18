#include "trans.h"

void trans_test() {
    mysql_test();
    redis_test();
}

void redis_test() {
    redisContext *c = redisConnect("127.0.0.1", 6379);
    if (c != NULL && c->err) {
        printf("Error: %s\n", c->errstr);
        // handle error
    } else {
        printf("Connected to Redis\n");
    }

    redisReply *reply;

    reply = (redisReply*) redisCommand(c, "PING");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);

    redisFree(c);
}

void mysql_test() {
    MYSQL *conn = mysql_init(NULL);
    MYSQL_RES *res;
    MYSQL_ROW row;

    struct connection_details mysqlID;
    mysqlID.server = "127.0.0.1";
    mysqlID.user = "root";
    mysqlID.password = "***";
    mysqlID.database = "DCN_shot";

    conn = mysql_connection_setup(mysqlID);

    // assign the results return to the MYSQL_RES pointer
    res = mysql_perform_query(conn, "show tables");

    printf("MySQL Tables in mysql database:\n");
    while ((row = mysql_fetch_row(res)) !=NULL)
        printf("%s\n", row[0]);

    // clean up the database result set 
    mysql_free_result(res);
    // clean up the database link 
    mysql_close(conn);
}

MYSQL* mysql_connection_setup(struct connection_details mysql_details)
{
    // first of all create a mysql instance and initialize the variables within
    MYSQL *connection = mysql_init(NULL);

    // connect to the database with the details attached.
    if (!mysql_real_connect(connection,mysql_details.server,
                mysql_details.user,
                mysql_details.password,
                mysql_details.database, 0, NULL, 0)) {
        printf("Conection error : %s\n", mysql_error(connection));
        exit(1);
    }
    return connection;
}

MYSQL_RES* mysql_perform_query(MYSQL *connection, char *sql_query)
{
    // send the query to the database
    if (mysql_query(connection, sql_query))
    {
        printf("MySQL query error : %s\n", mysql_error(connection));
        exit(1);
    }

    return mysql_use_result(connection);
}



