#include "watcher.h"
#include "log.h"
#include "trans.h"
#include "ever_main.h"
#include <hiredis/hiredis.h>

void Watcher::_inner_pubsub() {
    LOG_D("In watcher pubsub\n");

    redisReply *reply;
    reply = (redisReply*)redisCommand(this->c_redis,
                            "SUBSCRIBE %s", conf->redis_chanel);

    LOG_D("Listen: " << conf->redis_chanel << "\n");
    freeReplyObject(reply);

    while(!stop) {
        if (redisGetReply(this->c_redis, (void**)&reply) == REDIS_OK) {
            if (reply == NULL) continue;
            if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {

                /*
                printf( "Received[] channel %s %s: %s\n",
                            reply->element[0]->str,
                            reply->element[1]->str,
                            reply->element[2]->str );
                            */
                command_parse(reply->element[2]->str);
            }
            freeReplyObject(reply);
        }
    }
}

void Watcher::command_parse(char *commands) {
    LOG_D("Command: " << commands << "\n");
}

void Watcher::_inner_push() {
    LOG_D("In watcher push\n");
    while(!stop) {
        Message msg = _msg_queue->pop();
    }
}

void Watcher::init(Conf* conf, EverflowMain* main) {
    this->conf = conf;
    this->_main = main;
    this->_msg_queue =  main->get_message_queue();
    this->_out_switch_set = main->get_out_switch_set();
    this->_counter_map = main->get_counter_map();
}

void Watcher::init_connect() {
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
