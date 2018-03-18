#include <stdio.h>
#include "reader.h"
#include "trans.h"
#include "on_process.h"


int main(int argc, char *argv[])
{
    Processer p;
    p.run();
    p.join();
    pcap_read();
    trans_test();
    return 0;
}
