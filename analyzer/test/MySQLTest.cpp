#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "trans.h"
#include <stdio.h>
#include <sys/socket.h> // for inet_aton
#include "log.h"
#include <netinet/in.h>
#include <arpa/inet.h>


void save_test() {
    PKT_TRACE_T trace;
    LOG_I("In MySQLTest save! get trace size: " << sizeof(trace) << "\n");
    char* src = "192.168.1.112";
    char* dst = "192.128.1.123";
    inet_aton(src, &trace.key.ip_src);
    inet_aton(dst, &trace.key.ip_dst);

    trace.key.protocol = 0x80;
    trace.timestart = 234566;
    trace.hp1_switch_id = 234;
    trace.hp1_rcvd = 2;

    trace.hop_info[0] = {
        .switch_id = 113,
        .hop_rcvd = 2,
        .hop_timeshift = 12
    };
    trace.hop_info[1] = {
        .switch_id = 89,
        .hop_rcvd = 3,
        .hop_timeshift = 14
    };

    trace.is_loop = 1;
    trace.is_drop = 0;
    trace.is_probe = 1;

    struct connection_details mysqlID;
    mysqlID.server = "127.0.0.1";
    mysqlID.user = "root";
    mysqlID.password = "***";
    mysqlID.database = "DCN_shot";

    MYSQL *conn = mysql_init(NULL);
    conn = mysql_connection_setup(mysqlID);
    if(conn == NULL) exit(-1);

    save_trace(conn, &trace);

    mysql_close(conn);
}

int main() {
    Logger::instance()->init(&std::cout, Level::DEBUG);
    save_test();
    //mysql_test();
    return 0;
}

