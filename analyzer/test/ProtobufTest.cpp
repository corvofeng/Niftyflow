#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "proto/message.pb.h"


int main() {

    using tutorial::Person;
    Person john;
    john.set_id(1234);
    john.set_name("John Doe");

    return 0;
}

