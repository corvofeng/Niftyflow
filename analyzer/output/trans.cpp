#include "trans.h"
#include <sys/socket.h> // for inet_ntoa
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>

void trans_test() {
    mysql_test();
    redis_test();
}


// 获取在结构体中的偏移
#define  MEM_OFFSET(SCT, MEM) (&((SCT*)0)->MEM)

typedef struct BINDS{
    enum_field_types type;
    bool is_null;
    int offset;
};


void save_trace(MYSQL* conn, PKT_TRACE_T* trace) {

    struct BINDS arr[] = {
        {MYSQL_TYPE_STRING, },
    };

    MYSQL_BIND    bind[8];
    // memset(bind, 0, sizeof(bind));
    //char *s_ip = inet_ntoa((in_addr)trace->key.src_ip);
    //char *d_ip = inet_ntoa((in_addr)trace->key.dst_ip);
 //   int proto = trace->key.protocol;
    int generat_time = trace->timestart;

  //   bind[0].buffer_type= MYSQL_TYPE_STRING; // 源IP
  //   bind[0].buffer= "s_ip";
  //   bind[0].is_null= 0;
  //   bind[0].length=  5;

  //   bind[1].buffer_type= MYSQL_TYPE_STRING; // 目的IP
  //   bind[1].buffer= "d_ip"
  //   bind[1].is_null= 0;
  //   bind[1].length= 5;

  //   bind[0].buffer_type= MYSQL_TYPE_STRING; // 
  //   bind[0].buffer= "s_ip";
  //   bind[0].is_null= 0;
  //   bind[1].length= 5;

}

bool redis_test() {
    redisContext *c = redisConnect("127.0.0.1", 6379);
    if (c != NULL && c->err) {
        printf("Error: %s\n", c->errstr);
        // handle error
        return false;
    } else {
        printf("Connected to Redis\n");
    }

    redisReply *reply;

    reply = (redisReply*) redisCommand(c, "PING");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);

    redisFree(c);
    return true;
}

bool mysql_test() {
    MYSQL *conn = mysql_init(NULL);
    MYSQL_RES *res;
    MYSQL_ROW row;

    struct connection_details mysqlID;
    mysqlID.server = "127.0.0.1";
    mysqlID.user = "root";
    mysqlID.password = "fengyuhao";
    mysqlID.database = "DCN_shot";

    conn = mysql_connection_setup(mysqlID);
    if(conn == NULL) exit(-1);

    // assign the results return to the MYSQL_RES pointer
    res = mysql_perform_query(conn, "show tables");

    printf("MySQL Tables in mysql database:\n");
    while ((row = mysql_fetch_row(res)) !=NULL)
        printf("%s\n", row[0]);

    // clean up the database result set 
    mysql_free_result(res);
    // clean up the database link 
    mysql_close(conn);
    return true;
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
        return NULL;
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



