#include <iostream>
#include "log.h"

int main(int argc, char *argv[])
{
    /**
     * Init log
     */
    Logger::instance()->init(&std::cout, Level::DEBUG);

    LOG_D("debug   log\n");
    LOG_W("warning log\n");
    LOG_I("info    log\n");
    LOG_E("error   log");

    return 0;
}
