#include <iostream>
#include <sstream>
#include <fstream>
#include "ever_main.h"
#include "log.h"
#include "conf.h"


int main(int argc, char *argv[])
{
    //  Init log
    Logger::instance()->init(&std::cout, Level::DEBUG);
    std::ifstream input("../conf/conf.json");
    std::stringstream sstr;
    while (input >> sstr.rdbuf());
    Conf::instance()->ConfRead(sstr.str().c_str());

    EverflowMain eMain;

    eMain.run();

    return 0;
}
