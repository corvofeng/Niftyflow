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
 * 2018-03-30: 其实是29号的工作, 但我觉得29号写的太多了, 决定预先使用一天.
 *              主要是将Watcher改为了单例模式, 主要是为了能在信号处理函数中被
 *              截获并调用相关函数.
 *
 * 2018-03-29: 经过研究后发现, 28日总结出的问题是不太对的. 真正的原因是, 没有
 *              判断reply的数据类型就直接使用了, `reply->str`, 原来在push线程中
 *              reply其实是integer类型的, 如果不按照类型取数据, 就连其他线程
 *              也被阻塞了, 之后我可能会单独进行测试一下, 希望未来大家能在使用
 *              Redis中的reply时, 首先确定返回类型, 再进行输出. 否则程序真的
 *              不知道会在什么地方阻塞.
 *
 *              同样, 因为昨天的误发现了自己将pthread 与STL混用了,
 *              也对队列进行了修改, 就当是为了防止未来可能的错误吧.
 *
 * 2018-03-28:  redis的阻塞到底是什么原因啊, 好烦, 我都改了一上午了.
 *              最后发现Linux下的pthread和 STL里面的mutex和condition不能混用,
 *              condition会把pthread的线程都阻塞.
 *
 *  **Attention**:
 *              而后进行内存泄露问题的查找, cJSON_Print会进行动态的内存分配,
 *              需要在使用过之后free, 释放内存
 *
 * 2018-03-27: 上午一直在调redis的PubSub, 直到下午重启之后, 换了个函数调用就
 *              可以了, 想想也真是玄学, 你根本不知道它会在哪里阻塞
 *
 * 2018-03-26: 最后想了想, 其实把Redis与MySQL的初始化放在一块儿也是可以的.
 */

#include "atom_counter.h"
#include "cnt_rule.h"
#include "con_queue.h"
#include "log.h"
#include "message.h"
#include <memory>
#include <unordered_set>
#include <map>
#include <mysql.h>
#include <hiredis/hiredis.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

using std::map;
using std::shared_ptr;
using std::unordered_set;
class EverflowMain;
class Conf;

/**
 * @brief 这是个类中包含了两个独立于主程序的线程, 一个负责向队列中推送信息,
 *       另一个线程用于读取Pubsub下, 控制器发送的控制指令, 初始化以及更新
 *       也由这个线程完成. 也就是说, 所有与控制器交互的逻辑全部在这里表现.
 *
 *  作为整个程序的监听者存在, 提交报警信息, 接受控制器发来的指令.
 */
class Watcher
{
private:
    Watcher():_counter_map(NULL), _main(NULL),
                stop(false), is_command_init_ok(false)
    {
        /* zero interval means no reset of timer */
        timerclear(&counter_tval.it_interval);
        timerclear(&counter_tval.it_value);

        counter_tval.it_value.tv_sec = 20;  /* 20 second timeout */
        pthread_mutex_init(&_counter_map_mtx, NULL);
    }

public:
    static Watcher* instance() {
        static Watcher w;
        return &w;
    }

    ~Watcher () {
        try_free();
        pthread_mutex_destroy(&_counter_map_mtx);
    }

    /**
     * @brief 初始化函数, 绑定配置文件以及EverflowMain中的几个重要变量,
     *          重要变量包括以下的_counter_map, _msg_queue, _out_switch_set.
     *          通过绑定这些变量, 可以进行对其的监听或是修改.
     */
    void init(Conf* conf, EverflowMain* main);

    /**
     * @brief 向队列中发送请求, 而后进入等待状态.
     *  直到收到分析器发回的初始化信息, wait_command_init才会返回, 确保初始化成功
     */
    void send_init();
    void wait_command_init();

    void _inner_pubsub();       // 接受控制器发来的信息
    void _inner_push();         // 从Message队列中取出并推送.
    void _inner_save_counter(); // 轮询所有counter, 将其保存在数据库中
    void init_connect();

    //  防止在修改规则时产生上传操作, 这里采用加锁的模式
    enum {ADD_RULE, DEL_RULE};
    void on_update_counter_rule(std::vector<CounterRule>& rules, int act);

    /**
     * @brief 解析从控制器发来的信息, 如果控制信息有效,
     *         将会执行相关指令, 十分庞大, 请查看文档后进行修改
     *
     *         控制器的广播有可能针对所有分析器(id为0)
     *         也有可能只针对特定的一台(id为分析器的id),
     *         只有这两种情况才能影响交换机的配置
     */
    void command_parse(char *commands);

private:
    pthread_t t_pubsub_chanel;  /**< 线程变量 */
    pthread_t t_push_queue;

    Conf *conf;              /**< 全局配置, 初始化时传入 */
    EverflowMain* _main;     /**< 主要类的句柄, 可以借此访问其中的成员, 初始化时获取 */

    pthread_mutex_t _counter_map_mtx;              /**< 防止上传时的线程不安全*/
    map<CounterRule, shared_ptr<Counter>>* _counter_map;  /**< 记录计数器的规则*/
    Queue<Message>* _msg_queue;                           /**< 消息队列        */
    unordered_set<int>* _out_switch_set;                  /** 出口交换机的id列表 */

    MYSQL *c_mysql;               /**< 数据库连接 */
    redisContext *c_redis_pubsub; /**< redis连接 */
    redisContext *c_redis_queue;  /**< redis连接 */

    bool stop;
    bool is_command_init_ok;      /**< 标记是否第一次获取初始化的请求结果 */

    struct itimerval counter_tval; /**< 计数器间隔时间差 */

public:
    void try_free() {
        if(c_mysql)
            mysql_close(c_mysql);
        if(c_redis_queue)
            redisFree(c_redis_queue);
        if(c_redis_pubsub)
            redisFree(c_redis_pubsub);
        c_mysql = NULL;
        c_redis_pubsub = NULL;
        c_redis_queue = NULL;
    }

    // 此函数是不能被主动调用的, 并且只能被SIGALRM激活
    static void timely_save_counter(int signo) {
        if(signo != SIGALRM) {
            LOG_E("This function can't be called\n");
            return ;
        }

        Watcher* w = Watcher::instance();
        w->_inner_save_counter();

        // 重复设置定时器
        (void) setitimer(ITIMER_REAL, &w->counter_tval, NULL);
    }

    // 以下几个函数均是线程相关的定义函数, 与真正的逻辑关系不大
    // 开始run时, 马上进行计数
    void run() {
        (void) setitimer(ITIMER_REAL, &this->counter_tval, NULL);
        pthread_create(&t_pubsub_chanel , NULL , &Watcher::pubsub_process     , this);
        pthread_create(&t_push_queue    , NULL , &Watcher::push_queue_process , this);
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
