/*
 *=======================================================================
 *    Filename:watcher.h
 *
 *     Version: 1.0
 *  Created on: March 26, 2018
 *
 *      Author: corvo
 *=======================================================================
 */

#ifndef WATCHER_H_2RGQCSXG
#define WATCHER_H_2RGQCSXG

/**
 *
 *
 * 2018-03-26: 最后想了想, 其实把Redis与MySQL的初始化放在一块儿也是可以的.
 */

#include "atom_counter.h"
#include "cnt_rule.h"
#include "con_queue.h"
#include "message.h"
#include <map>
#include <mysql.h>
#include <hiredis/hiredis.h>

using std::map;
class EverflowMain;
class Conf;

/**
 * @brief 这是个类中包含了两个独立于主程序的线程, 一个负责向队列中推送信息,
 *       另一个线程用于读取Pubsub下, 控制器发送的控制指令, 初始化以及更新
 *       也由这个线程完成. 也就是说, 所有与控制器交互的逻辑全部在这里表现.
 *
 *  作为整个程序的监听者存在
 */
class Watcher
{
public:

    Watcher():_counter_map(NULL), _main(NULL){}
    ~Watcher () {
        if(c_mysql)
            mysql_close(c_mysql);
        if(c_redis)
            redisFree(c_redis);
    }
    void bind_counter_map(map<CounterRule, Counter>* counter_map) {
        this->_counter_map = counter_map;
    }
    pthread_t t_pubsub_chanel;
    pthread_t t_push_queue;

    void _inner_pubsub();
    void _inner_push();
    void init_connect(const Conf* conf);

private:
    EverflowMain* _main;            // 主要类的句柄, 可以借此访问其中的成员
    map<CounterRule, Counter>* _counter_map;
    Queue<Message> msg_queue;

    MYSQL *c_mysql;             // 数据库连接
    redisContext *c_redis;      // redis连接

public:

    void run() {
        pthread_create(&t_pubsub_chanel, NULL, &Watcher::pubsub_process, this);
        pthread_create(&t_push_queue, NULL, &Watcher::push_queue_process, this);
    }

    void join() {
        (void) pthread_join(t_pubsub_chanel, NULL);
        (void) pthread_join(t_push_queue, NULL);
    }

    static void* pubsub_process(void *context) {
        Watcher *w = (Watcher*)context;
        w->_inner_pubsub();
    }

    static void* push_queue_process(void *context) {
        Watcher *w = (Watcher*)context;
        w->_inner_push();
    }
};

#endif /* end of include guard: WATCHER_H_2RGQCSXG */
