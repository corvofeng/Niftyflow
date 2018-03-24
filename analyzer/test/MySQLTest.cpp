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

    


}

int main() {
    Logger::instance()->init(&std::cout, Level::DEBUG);
    save_test();
    //mysql_test();
    return 0;
}

