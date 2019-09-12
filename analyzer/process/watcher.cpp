#include "watcher.h"
#include "log.h"
#include "ever_main.h"
#include "trans.h"
#include <vector>
#include <unistd.h>
#include <hiredis/hiredis.h>
#include "cJSON/cJSON.h"

using std::vector;

void Watcher::_inner_pubsub() {
    LOG_I("In watcher pubsub\n");

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

    // 必须在开始监听频道后才能发送初始化信息
    this->send_init();

    while(!stop) {
        int rp;
        if ( (rp = redisGetReply(this->c_redis_pubsub,
                                        (void**)&reply))== REDIS_OK) {

            if (reply == NULL) continue;
            LOG_D("Get reply " << reply->element[0]->str << "\n");
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
        LOG_D("GET FORCE QUIT" << _force_quit << "\n");
        if(_force_quit) break;
    }
    LOG_I("Pubsub exit\n");
}


/**
 * 样例输入
 * {
 *  "ANALYZER_ID": 123,
 *  "MESSAGE": {
 *    "COUNTER": [
 *      {
 *        "CNT_ID": 1,
 *        "SRC_IP": "192.3.2.3",
 *        "DST_IP": "",
 *        "SWH_ID": 0,
 *        "PTL": -1
 *      }
 *    ],
 *    "SWH_ID": null,
 *    "COMMOND": "INIT"
 *  }
 *}
 */
void Watcher::command_parse(char *commands) {
    LOG_D("Command: " << commands << "\n");
    cJSON *jConf = cJSON_Parse(commands);
    if(!jConf) LOG_E("Read commands err: " << commands << "\n");

    cJSON* jId = cJSON_GetObjectItem(jConf, "ANALYZER_ID");

    vector<CounterRule> rules;
    do {
        if(jId == NULL || jId->type != cJSON_Number) {// 字段错误
            LOG_E("Read commands err: " << commands << "\n");
            break;
        }
        if(jId->valueint != 0 && jId->valueint != conf->analyzer_id){// 不是针对自己
            LOG_W("Not for me\n");
            break;
        }

        cJSON* jMsg = cJSON_GetObjectItem(jConf, "MESSAGE");
        if(jMsg == NULL) {
            LOG_E("Read Message is empty !!\n");
            break;
        }
        cJSON* jCounters = cJSON_GetObjectItem(jMsg, "COUNTER");
        if(!jCounters || jCounters->type != cJSON_Array) {
            LOG_E("Invalid counter message!!\n");
            break;
        }
        cJSON* jCommand = cJSON_GetObjectItem(jMsg, "COMMAND");
        if(!jCommand || jCommand->type != cJSON_String) {
            LOG_E("Invalid command !!\n");
            break;
        }


        int n = cJSON_GetArraySize(jCounters);
        cJSON* jRuleItem;
        for(int i = 0; i < n; i++) {
            jRuleItem = cJSON_GetArrayItem(jCounters, i);
            cJSON* jId = cJSON_GetObjectItem(jRuleItem, "CNT_ID");
            cJSON* jIp_src = cJSON_GetObjectItem(jRuleItem, "SRC_IP");
            cJSON* jIp_dst = cJSON_GetObjectItem(jRuleItem, "DST_IP");
            cJSON* jSwh_id = cJSON_GetObjectItem(jRuleItem, "SWH_ID");
            cJSON* jPtl = cJSON_GetObjectItem(jRuleItem, "PTL");
            LOG_D("Get rule " << jId->valueint << "\n");

            if(jId == NULL || jId->type != cJSON_Number) {
                LOG_E("Invalid msg\n");
                continue;
            }
            CounterRule r(jId->valueint);

            int flag;
            flag = inet_aton(jIp_src->valuestring, &r.ip_src);
            if(flag == 0) LOG_E("Parse SRC ip failed\n");

            flag = inet_aton(jIp_dst->valuestring, &r.ip_dst);
            if(flag == 0) LOG_E("Parse DST ip failed\n");

            r.switch_id = jSwh_id->valueint;
            r.protocol = jPtl->valueint;

            LOG_D("Get ptl " <<  r.protocol << "\n");
            // r.ip_dst = inet_aton(jIp_src->)
            rules.push_back(r);
        }

        if (strcmp(jCommand->valuestring, "DEL_RULE") == 0) {
            on_update_counter_rule(rules, DEL_RULE);   // 只有在命令为DEL时才进行删除
        } else {
            on_update_counter_rule(rules, ADD_RULE);
        }

    }while(0);

    cJSON_Delete(jConf);
}

void Watcher::on_update_counter_rule(vector<CounterRule>& rules, int act) {
    Lock l(&this->_counter_map_mtx);
    if(act != ADD_RULE && act != DEL_RULE) {
        LOG_E("Unkonw action " << act << "\n");
        return;
    }

    _main->reader_pause();
    if(act == ADD_RULE) {
        _main->add_rules(rules);
    } else if (act == DEL_RULE) {
        _main->del_rules(rules);
    } else {
        throw std::runtime_error("This stmt can't be called");
    }
    _main->reader_active();
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

    char* s = cJSON_PrintUnformatted(jMsg);
    Message m(s);
    LOG_D(s << "\n");
    this->_msg_queue->push(m);
    free(s);    // 使用cJSON_Print时会调用malloc, 需要进行释放
    cJSON_Delete(jMsg);
}

void Watcher::make_quit() {
    this->_force_quit = true;

    cJSON *jMsg = cJSON_CreateObject();
    cJSON_AddStringToObject(jMsg, "ACTION", "QUIT");
    cJSON_AddNumberToObject(jMsg, "ANALYZER_ID", conf->analyzer_id);

    char* s = cJSON_PrintUnformatted(jMsg);

    Message m(s);
    free(s);    // 使用cJSON_Print时会调用malloc, 需要进行释放
    cJSON_Delete(jMsg);

    this->_msg_queue->push(m);
}

void Watcher::_inner_push() {
    LOG_I("In watcher push\n");
    const char *_queue_str = conf->redis_queue;

    redisReply *reply;
    while(!stop) {
        Message m = _msg_queue->pop();
        LOG_D("GET FORCE QUIT" << _force_quit << "\n");

        LOG_D("In queue: " << m.msg << "\n");
        reply = (redisReply*)redisCommand(this->c_redis_queue,
                            "LPUSH %s %s", _queue_str, m.msg.c_str());
        if(reply->type == REDIS_REPLY_INTEGER) {
            LOG_D("Reply " << reply->integer << "\n");
        } else {
            LOG_W("Get return err\n");
        }

        freeReplyObject(reply);
        if(_force_quit) break;
    }
    LOG_I("Push queue over\n");
}

void Watcher::_inner_save_counter() {
    LOG_D("Start save counter\n");
    map<int, int> r_2_c;    /**<  ruleid => cnt */

    {
        Lock l(&this->_counter_map_mtx);    // 加锁, 防止map的添加删除
        for(auto& item : (*_counter_map)) {
            const CounterRule& rule = item.first;
            shared_ptr<Counter>& cnt  = item.second;
            int r = rule.rule_id;           // rule_id
            int c = cnt->get_and_clear();   // cnt, 线程安全
            r_2_c.insert(std::make_pair(r, c));
        }
    }

    int analyzer_id = Conf::instance()->analyzer_id;
    for(auto& item: r_2_c) {
        int r = item.first;
        int c = item.second;
        LOG_D("Get counter " << r << " : " << c << "\n");
        save_counter(this->c_mysql, item.first,analyzer_id, c);
    }
    LOG_D("Start save counter over\n");
}

void Watcher::init(Conf* conf, EverflowMain* main) {
    this->conf = conf;
    this->_main = main;
    this->_msg_queue =  main->get_message_queue();
    this->_out_switch_set = main->get_out_switch_set();
    this->_counter_map = main->get_counter_map();

    // 添加定时器
    (void) signal(SIGALRM, Watcher::timely_func);
}

void Watcher::_inner_calculte_pcaket() {
    vector<shared_ptr<PKT_QUEUE>> * p_vec = this->_main->get_queue_vec();
    int pkt_cnt = 0;
    for(int i = 0; i < p_vec->size(); i++) {
        pkt_cnt += (*p_vec)[i]->size();
    }
    LOG_I(FMT("After %ds. we has %d packets\n", TIME_VAL, pkt_cnt));
}

void Watcher::init_connect() {
    do {
        this->c_mysql = mysql_connection_setup(conf);
        if(this->c_mysql == NULL)
            break;
        LOG_D("Connect mysql ok\n");

        this->c_redis_pubsub = redis_connection_setup(conf);
        this->c_redis_queue = redis_connection_setup(conf);

        if(!this->c_redis_pubsub && !this->c_redis_queue)
            break;
        LOG_D(FMT("%d, %d\n", this->c_redis_queue->fd, this->c_redis_pubsub->fd));
        LOG_D("Connect redis ok\n");
        return;
    }while(0);

// err handler, 正常情况下不该执行到这里.
    LOG_D("Connect error\n");
    try_free();
}
