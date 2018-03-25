#include "trans.h"
#include <sys/socket.h> // for inet_ntoa
#include <netinet/in.h>
#include <arpa/inet.h>
#include "log.h"
#include "cJSON/cJSON.h"

#include <stdlib.h>
#include <string.h>

void trans_test() {
    mysql_test();
    redis_test();
}


// 获取在结构体中的偏移

#define INSERT_SAMPLE  "INSERT INTO                  \
    `tbl_trace_data`                                 \
    (`s_ip`, `d_ip`, `generate_time`, `protocal`,    \
     `trace_data`, `is_loop`, `is_drop`, `is_probe`) \
    VALUES                                           \
    (?, ?, ?, ?, ?, ?, ?, ?)"

enum {BUF_SIZE = 4096};
static long unsigned get_trace_json(PKT_TRACE_T* pkt, char* buf) {
    cJSON* tInfo = cJSON_CreateObject();
    cJSON* tArr = cJSON_CreateArray();
    cJSON* tItem;

    cJSON_AddItemToArray(tArr, tItem = cJSON_CreateObject());
    cJSON_AddItemToObject(tItem, "switch_id",
                                    cJSON_CreateNumber(pkt->hp1_switch_id));
    cJSON_AddItemToObject(tItem, "hop_rcvd", cJSON_CreateNumber(pkt->hp1_rcvd));
    cJSON_AddItemToObject(tItem, "hop_timeshift", cJSON_CreateNumber(0));

    for(int i = 0; i < TRACE_CNT - 1; i++) {
        if(pkt->hop_info[i].hop_rcvd <= 0) break;   // 路径已经结束

        cJSON_AddItemToArray(tArr, tItem = cJSON_CreateObject());
        cJSON_AddItemToObject(tItem, "switch_id",
                            cJSON_CreateNumber(pkt->hop_info[i].switch_id));
        cJSON_AddItemToObject(tItem, "hop_rcvd",
                cJSON_CreateNumber(pkt->hop_info[i].hop_rcvd));
        cJSON_AddItemToObject(tItem, "hop_timeshift",
                cJSON_CreateNumber(pkt->hop_info[i].hop_timeshift));
    }
    cJSON_AddItemToObject(tInfo, "trace_info", tArr);
    LOG_D(cJSON_Print(tInfo) << "\n");
    long int len = strlen(cJSON_Print(tInfo));
    strcpy(buf, cJSON_Print(tInfo));

    cJSON_Delete(tInfo);

    return len;
}

void save_trace(MYSQL* conn, PKT_TRACE_T* trace) {

    LOG_D("START SAVER\n");

    MYSQL_BIND    bind[8];
    memset(bind, 0, sizeof(bind));
    char *s_ip = inet_ntoa((in_addr)trace->key.ip_src);
    long unsigned int s_len = strlen(s_ip);
    char *d_ip = inet_ntoa((in_addr)trace->key.ip_dst);
    long unsigned int d_len = strlen(d_ip);
    char buf[BUF_SIZE];

    int generat_time = trace->timestart;
    int proto = trace->key.protocol;
    int loop = trace->is_loop ? 1: 0;
    int drop = trace->is_drop? 1: 0;
    int probe = trace->is_probe? 1: 0;
    long unsigned trace_len = get_trace_json(trace, buf);

    bind[0].buffer_type= MYSQL_TYPE_STRING; // 源IP
    bind[0].buffer= s_ip;
    bind[0].is_null= 0;
    bind[0].length= &s_len;

    bind[1].buffer_type= MYSQL_TYPE_STRING; // 目的IP
    bind[1].buffer= d_ip;
    bind[1].is_null= 0;
    bind[1].length= &d_len;

    bind[2].buffer_type= MYSQL_TYPE_LONG;   // 开始时间
    bind[2].buffer= (char *)&generat_time;
    bind[2].is_null= 0;
    bind[2].length= 0;

    bind[3].buffer_type= MYSQL_TYPE_LONG;   // 协议
    bind[3].buffer= (char* )&proto;
    bind[3].is_null= 0;
    bind[3].length= 0;

    bind[4].buffer_type= MYSQL_TYPE_STRING;   // trace数据, 使用BLOB模式
    bind[4].buffer= buf;
    bind[4].is_null= 0;
    bind[4].length= &trace_len;

    bind[5].buffer_type= MYSQL_TYPE_LONG;   // 是否环路
    bind[5].buffer= (char *)&loop;
    bind[5].is_null= 0;
    bind[5].length= 0;

    bind[6].buffer_type= MYSQL_TYPE_LONG;   // 是否丢包
    bind[6].buffer= (char*)&drop;
    bind[6].is_null= 0;
    bind[6].length= 0;

    bind[7].buffer_type= MYSQL_TYPE_LONG;   // 是否探针
    bind[7].buffer= (char *)&probe;
    bind[7].is_null= 0;
    bind[7].length= 0;


    MYSQL_STMT    *stmt;
    int           param_count;
    my_ulonglong  affected_rows;


    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
      LOG_E(" mysql_stmt_init(), out of memory\n");
      return ;
    }
    if (mysql_stmt_prepare(stmt, INSERT_SAMPLE, strlen(INSERT_SAMPLE)))
    {
        LOG_E(" mysql_stmt_prepare(), INSERT failed\n");
        LOG_E(FMT(" %s\n", mysql_stmt_error(stmt)));
        return ;
    }
    LOG_D(" prepare, INSERT successful\n");

    /* Get the parameter count from the statement */
    param_count= mysql_stmt_param_count(stmt);
    LOG_D(FMT(" total parameters in INSERT: %d\n", param_count));

    if (param_count != 8) /* validate parameter count */
    {
      LOG_E( " invalid parameter count returned by MySQL\n");
      return ;
    }

    if (mysql_stmt_bind_param(stmt, bind))
    {
      LOG_E( " mysql_stmt_bind_param() failed\n");
      LOG_E(FMT(" %s\n", mysql_stmt_error(stmt)));
      return ;
    }

    /* Specify the data values for the first row */
    // int_data= 10;             /* integer */
    // strncpy(str_data, "MySQL", STRING_SIZE); /* string  */
    // str_length= strlen(str_data);

    /* Execute the INSERT statement - 1*/
    if (mysql_stmt_execute(stmt))
    {
      LOG_E(" mysql_stmt_execute(), 1 failed\n");
      LOG_E(FMT(" %s\n", mysql_stmt_error(stmt)));
      return ;
    }

    /* Get the total rows affected */
    affected_rows= mysql_stmt_affected_rows(stmt);
    LOG_D(FMT(" total affected rows(insert 1): %lu\n",
                (unsigned long) affected_rows));
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

    conn = mysql_connection_setup(Conf::instance());
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

MYSQL* mysql_connection_setup(const Conf* c)
{
    // first of all create a mysql instance and initialize the variables within
    MYSQL *connection = mysql_init(NULL);

    // connect to the database with the details attached.
    if (!mysql_real_connect(
                connection,
                c->mysql_host,
                c->mysql_user,
                c->mysql_password,
                c->mysql_database, c->mysql_port, NULL, 0)) {
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



