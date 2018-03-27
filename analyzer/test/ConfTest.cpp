#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include "conf.h"

int main()
{
    std::ifstream input("../conf/conf.json");
    std::stringstream sstr;
    while (input >> sstr.rdbuf());

    Logger::instance()->init(&std::cout, Level::DEBUG);
    Conf::instance()->ConfRead(sstr.str().c_str());

    printf("Analyer id: %d\n", Conf::instance()->analyzer_id);

    printf("MySQL:\n");
    printf("host:   %s\n", Conf::instance()->mysql_host);
    printf("port;   %d\n", Conf::instance()->mysql_port);
    printf("user:   %s\n", Conf::instance()->mysql_user);
    printf("pwd :   %s\n", Conf::instance()->mysql_password);
    printf("db  :   %s\n", Conf::instance()->mysql_database);

    printf("Redis:\n");
    printf("host:  %s\n", Conf::instance()->redis_host);
    printf("port:  %d\n", Conf::instance()->redis_port);
    printf("auth:  %s\n", Conf::instance()->redis_auth);
    printf("chan:  %s\n", Conf::instance()->redis_chanel);
    printf("que :  %s\n", Conf::instance()->redis_queue);

    return 0;
}
