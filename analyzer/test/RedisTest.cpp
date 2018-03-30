#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "trans.h"
#include <iostream>
#include <sstream>
#include <fstream>

int main() {
    Logger::instance()->init(&std::cout, Level::DEBUG);
    std::ifstream input("../conf/conf.json");
    std::stringstream sstr;
    while (input >> sstr.rdbuf());
    Conf::instance()->ConfRead(sstr.str().c_str());


    if(redis_test(Conf::instance()))
        return 0;
    else
        return -1;
}

