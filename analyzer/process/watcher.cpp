#include "watcher.h"
#include "log.h"
#include "trans.h"
#include "ever_main.h"
#include <hiredis/hiredis.h>
#include "cJSON/cJSON.h"


void Watcher::_inner_pubsub() {
    LOG_D("In watcher pubsub\n");

    redisReply *reply;
    const char* _chanel_str = conf->redis_chanel;
    reply = (redisReply*)redisCommand(this->c_redis_pubsub,
                                    "SUBSCRIBE %s", _chanel_str);

    if(reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
        LOG_D("SUB Reply: " << reply->element[0]->str << "\n");
    } else {
        LOG_W("SUB Reply: " << reply->type << "\n");
    }
    freeReplyObject(reply);

    while(!stop) {
        int rp;
        if ( (rp = redisGetReply(this->c_redis_pubsub,
                                        (void**)&reply))== REDIS_OK) {

            if (reply == NULL) continue;
            if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
                if ( strcmp( reply->element[0]->str, "message") == 0 ) {
                    command_parse(reply->element[2]->str);
                } else {
                    LOG_W(FMT("Received[%s] channel %s: %s\n",
                                reply->element[0]->str,
                                reply->element[1]->str,
                                reply->element[2]->str));
                }
            }
            freeReplyObject(reply);
        } else {
            LOG_E("Reply with pubsub: " << rp << "\n");
        }
    }
}

void Watcher::command_parse(char *commands) {
    LOG_D("Command: " << commands << "\n");
    cJSON *jConf = cJSON_Parse(commands);
    if(!jConf) LOG_E("Read commands err: " << commands << "\n");

    cJSON_Delete(jConf);
}

/**
 * 初始化
 *  {
 *    "ACTION": "INIT",
 *    "ANALYZER_ID":  123
 *  }
 */
void Watcher::send_init() {
    cJSON *jMsg = cJSON_CreateObject();
    cJSON_AddStringToObject(jMsg, "ACTION", "INIT");
    cJSON_AddNumberToObject(jMsg, "ANALYZER_ID", conf->analyzer_id);

    Message m(cJSON_PrintUnformatted(jMsg));
    LOG_D(cJSON_Print(jMsg) << "\n");
    this->_msg_queue->push(m);
    cJSON_Delete(jMsg);
}

void Watcher::wait_command_init() {

}

void Watcher::_inner_push() {
    LOG_D("In watcher push\n");
    const char *_queue_str = conf->redis_queue;

    redisReply *reply;
    while(!stop) {
        Message m = _msg_queue->pop();
        LOG_D("In queue: " << m.msg << "\n");
        reply = (redisReply*)redisCommand(this->c_redis_queue,
                            "LPUSH %s %s", _queue_str, m.msg.c_str());
        LOG_D("Reply " << reply->str << "\n");
        freeReplyObject(reply);
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

        this->c_redis_pubsub = redis_connection_setup(conf);
        this->c_redis_queue = redis_connection_setup(conf);

        LOG_D(FMT("%d, %d\n", this->c_redis_queue->fd, this->c_redis_pubsub->fd));
        if(!this->c_redis_pubsub && !this->c_redis_queue)
            break;
        LOG_D("Connect redis ok\n");
        return;
    }while(0);

// err handler, 正常情况下不该执行到这里.
    LOG_D("Connect error\n");
    try_free();
}
