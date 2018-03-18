#include <stdio.h>
#include <iostream>
#include "reader.h"
#include "trans.h"
#include "on_process.h"
#include "log.h"


int main(int argc, char *argv[])
{
    /**
     * Init log
     */
    Logger::instance()->init(&std::cout, Level::DEBUG);

    Processer p;
    p.run();

    LOG_D("debug log\n");
    LOG_W("warning log\n");

    p.join();
    pcap_read();
    trans_test();
    return 0;
}
