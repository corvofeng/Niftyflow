#include <stdio.h>
#include <iostream>
#include "reader.h"
#include "trans.h"
#include "on_process.h"
#include "ever_main.h"
#include "log.h"


int main(int argc, char *argv[])
{
    /**
     * Init log
     */
    Logger::instance()->init(&std::cout, Level::DEBUG);

    EverflowMain eMain;

    LOG_D("debug log\n");
    LOG_W("warning log\n");
    LOG_D("size of trace: " << sizeof(PKT_TRACE_T) << "\n");
    eMain.run();

    pcap_read();
    // trans_test();
    return 0;
}
