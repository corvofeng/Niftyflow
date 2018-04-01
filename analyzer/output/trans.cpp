#include "trans.h"
#include <sys/socket.h> // for inet_ntoa
#include <netinet/in.h>
#include <arpa/inet.h>
#include "log.h"
#include "cJSON/cJSON.h"

#include <stdlib.h>
#include <string.h>


// 获取在结构体中的偏移

#define INSERT_TRACE_SAMPLE  "INSERT INTO            \
    `tbl_trace_data`                                 \
    (`s_ip`, `d_ip`, `generate_time`, `protocol`,    \
     `trace_data`, `is_loop`, `is_drop`, `is_probe`) \
     VALUES                                          \
     (?, ?, ?, ?, ?, ?, ?, ?)"

#define INSERT_COUNTER_SAMPLE " INSERT INTO `tbl_counter` \
    (`rule_id`, `analyzer_id`, `cnt`)                     \
    VALUES                                                \
    (   ?,          ?,          ? )"


enum {BUF_SIZE = 4096};

/**
 * @brief  每次插入数据库都要进行下面的操作, 所有固定下来了
 *
 * @param conn
 * @param bind   做好的bind参数
 * @param st     数据库语句
 * @param need_parm 需要的参数数量, 用于检查参数
 */
static void _in_saver(MYSQL* conn, MYSQL_BIND* bind,
                                const char* st, int need_parm);
/**
 * @brief 将trace数据中的路径信息转换为JSON字符串.
 *  {
 *    "trace_info": [
 *      {
 *        "switch_id": 234,
 *        "hop_rcvd": 2,
 *        "hop_timeshift": 0
 *      },
 *      {
 *        "switch_id": 113,
 *        "hop_rcvd": 2,
 *        "hop_timeshift": 12
 *      },
 *      ...
 *    ]
 *  }
 */
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

        int real_shift;
        // 获取真实的偏移时间, 具体原因请查看@common/trace.h
        GET_EXACT_TIME_SHIFT(pkt->hop_info[i].hop_timeshift, real_shift); 
        cJSON_AddItemToObject(tItem, "hop_timeshift",
                cJSON_CreateNumber(real_shift));
    }
    cJSON_AddItemToObject(tInfo, "trace_info", tArr);

    char * info = cJSON_Print(tInfo);

    LOG_D(info << "\n");
    long int len = strlen(info);
    strcpy(buf, info);

    free(info);     // 使用cJSON_Print分配的内存一定要释放.
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

    _in_saver(conn, bind, INSERT_TRACE_SAMPLE, 8);
}

void save_counter(MYSQL* conn, int rule_id, int analyzer_id, int cnt) {
    MYSQL_BIND    bind[8];
    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type= MYSQL_TYPE_LONG;   // rule_id
    bind[0].buffer= (char *)&rule_id;
    bind[0].is_null= 0;
    bind[0].length= 0;

    bind[1].buffer_type= MYSQL_TYPE_LONG;   // analyzer_id
    bind[1].buffer= (char *)&analyzer_id;
    bind[1].is_null= 0;
    bind[1].length= 0;

    bind[2].buffer_type= MYSQL_TYPE_LONG;   // 计数值
    bind[2].buffer= (char *)&cnt;
    bind[2].is_null= 0;
    bind[2].length= 0;

    _in_saver(conn, bind, INSERT_COUNTER_SAMPLE, 3);
}

bool redis_test(const Conf* conf) {
    redisContext *c = redisConnect(conf->redis_host, conf->redis_port);
    if (c != NULL && c->err) {
        LOG_E("Error: "<< c->errstr << "\n");
        // handle error
        return false;
    } else {
        LOG_E("Connected to Redis\n");
    }

    redisReply *reply;

    reply = (redisReply*) redisCommand(c, "PING");
    LOG_D("PING: " << reply->str << "\n");
    freeReplyObject(reply);

    redisFree(c);
    return true;
}

redisContext* redis_connection_setup(const Conf* conf) {
    redisContext *c = redisConnect(conf->redis_host, conf->redis_port);
    if (c != NULL && c->err) {
        LOG_E(c->errstr << "\n");
        // handle error
        return NULL;
    }
    redisReply *reply;
    if (conf->redis_auth) {
        reply = (redisReply*) redisCommand(c, "AUTH %s", conf->redis_auth);
        LOG_D("AUTH: " << reply->str << "\n");
        freeReplyObject(reply);
    }

    reply = (redisReply*) redisCommand(c, "PING");
    LOG_D("PING: " << reply->str << "\n");
    freeReplyObject(reply);

    return c;
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
        LOG_E("Conection error : "<< mysql_error(connection) << "\n");
        return NULL;
    }

    // 断线之后将会进行一次重连工作
    bool reconnect = 1;
    mysql_options(connection, MYSQL_OPT_RECONNECT, &reconnect);

    return connection;
}


static void _in_saver(MYSQL* conn, MYSQL_BIND* bind,
                            const char* st, int need_parm) {
    MYSQL_STMT    *stmt;
    int           param_count;
    my_ulonglong  affected_rows;

    stmt = mysql_stmt_init(conn);

    do {
        if (!stmt) {
            LOG_E(" Mysql_stmt_init(), out of memory\n");
            break;
        }
        if (mysql_stmt_prepare(stmt, st, strlen(st))) {
            LOG_E(" Mysql_stmt_prepare(), INSERT failed\n");
            LOG_E(" " << mysql_stmt_error(stmt) << "\n");
            break;
        }
        LOG_D(" Prepare, INSERT successful\n");

        /* Get the parameter count from the statement */
        param_count= mysql_stmt_param_count(stmt);
        LOG_D(" Total parameters in INSERT: " << param_count << "\n");

        if (param_count != need_parm) {  /* validate parameter count */ 
            LOG_E( " Invalid parameter count returned by MySQL\n");
            break;
        }

        if (mysql_stmt_bind_param(stmt, bind)) {
            LOG_E( " Mysql_stmt_bind_param() failed\n");
            LOG_E(" " << mysql_stmt_error(stmt) << "\n");
            break;
        }

        if (mysql_stmt_execute(stmt)) {
            LOG_E(" Mysql_stmt_execute(), 1 failed\n");
            LOG_E(" " << mysql_stmt_error(stmt) << "\n");
            break;
        }

        /* Get the total rows affected */
        affected_rows= mysql_stmt_affected_rows(stmt);
        LOG_D(" Total affected rows(insert 1): "
                << (unsigned long) affected_rows << "\n");
    } while(0);

    /* Close the statement */
    if (mysql_stmt_close(stmt)) {
        /* mysql_stmt_close() invalidates stmt, so call          */
        /* mysql_error(mysql) rather than mysql_stmt_error(stmt) */
        LOG_E(" Failed while closing the statement\n");
        LOG_E(" " << mysql_error(conn) << "\n");
    }
    LOG_D("Clear over\n");
}



bool mysql_test() {
    MYSQL *conn = mysql_connection_setup(Conf::instance());
    MYSQL_RES *res;
    MYSQL_ROW row;

    if(conn == NULL) exit(-1);

    // assign the results return to the MYSQL_RES pointer
    res = mysql_perform_query(conn, "show tables");

    LOG_D("MySQL Tables in mysql database:\n");
    while ((row = mysql_fetch_row(res)) !=NULL)
        LOG_D(row[0] << "\n");

    // clean up the database result set 
    mysql_free_result(res);
    // clean up the database link 
    mysql_close(conn);
    return true;
}



MYSQL_RES* mysql_perform_query(MYSQL *connection, char *sql_query)
{
    // send the query to the database
    if (mysql_query(connection, sql_query))
    {
        LOG_E("MySQL query error :  " << mysql_error(connection) << "\n");
        exit(1);
    }

    return mysql_use_result(connection);
}
