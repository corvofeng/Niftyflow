#include "watcher.h"
#include "log.h"
#include "trans.h"
#include <hiredis/hiredis.h>

void Watcher::_inner_pubsub() {


}
void Watcher::_inner_push() {

}

void Watcher::init_connect(const Conf* conf) {
    do {
        this->c_mysql = mysql_connection_setup(conf);
        if(this->c_mysql == NULL)
            break;
        LOG_D("Connect mysql ok\n");

        this->c_redis = redis_connection_setup(conf);

        if(this->c_redis == NULL)
            break;
        LOG_D("Connect redis ok\n");

        return;
    }while(0);

// err handler
    if(c_mysql)
        mysql_close(c_mysql);
    if(c_redis)
        redisFree(c_redis);
}
