#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cJSON/cJSON.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include "conf.h"

int main()
{
    std::ifstream input("../conf/conf.json");
    std::stringstream sstr;

    while (input >> sstr.rdbuf());

    std::cout << sstr.str() << std::endl;
    cJSON* jConf = cJSON_Parse(sstr.str().c_str());
    //cJSON* jAllCandidate = cJSON_GetObjectItem(jConf, "candidate");

    std::cout << cJSON_Print(jConf);


    cJSON_Delete(jConf);
    return 0;
}
