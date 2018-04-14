#include <iostream>
#include <sstream>
#include <fstream>
#include "ever_main.h"
#include "watcher.h"
#include "log.h"
#include "conf.h"
#include <rte_eal.h>


extern std::atomic_bool force_quit;

static void
signal_handler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM) {
        printf("\n\nSignal %d received, preparing to exit...\n",
                signum);
        Watcher::instance()->make_quit();
        EverflowMain::instance()->make_quit();
    }
}

int main(int argc, char *argv[])
{
    rte_eal_init(argc, argv);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    //  Init log
    Logger::instance()->init(&std::cout, Level::DEBUG);

    // Init config
    std::ifstream input("../conf/conf.json");
    std::stringstream sstr;
    while (input >> sstr.rdbuf());
    Conf::instance()->ConfRead(sstr.str().c_str());

    Watcher *watcher = Watcher::instance();

    // Init main process
    EverflowMain* eMain = EverflowMain::instance();

    // Init connect and read config
    watcher->init(Conf::instance(), eMain);
    watcher->init_connect();
    watcher->send_init();

    watcher->run();

    eMain->run();
    eMain->join();

    watcher->join();

    return 0;
}
