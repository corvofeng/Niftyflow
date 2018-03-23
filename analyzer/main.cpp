#include <iostream>
#include "ever_main.h"
#include "log.h"

int main(int argc, char *argv[])
{
    /**
     * Init log
     */
    Logger::instance()->init(&std::cout, Level::DEBUG);

    EverflowMain eMain;

    eMain.run();
    return 0;
}
