/*
 *=======================================================================
 *    Filename:ever_main.h
 *
 *     Version: 1.0
 *  Created on: March 18, 2018
 *
 *      Author: corvo
 *
 *  此文件中记录了主要的处理程序逻辑
 *=======================================================================
 */
#ifndef EVER_MAIN_H_5UBIZCWM
#define EVER_MAIN_H_5UBIZCWM

#include <vector>
#include <memory>
#include <map>
#include <pthread.h>
#include "con_queue.h"
#include "on_process.h"
#include "packet.h"
// #include "reader.h"
#include "atom_counter.h"
#include "cnt_rule.h"
#include <unordered_set>
#include "message.h"

using std::vector;
using std::map;
using std::shared_ptr;
using std::unordered_set;

class lcore_queue_conf;
class Reader;

/**
 * 2018-03-27: 添加了函数用来增加和删除规则
 *
 * 2018-03-23: 达到最后处理阶段, 目前想做的是在慢路径中进行数据库写入以及Redis
 *              写入.
 */
class EverflowMain
{
public:

    EverflowMain();
    ~EverflowMain ();

    // 在调用reader_pause之后才可以调用增加删除规则, 暂停功能一定要慎用.
    // 会导致所有reader均暂停
    void reader_pause();
    void add_rules(vector<CounterRule>& rules);
    void del_rules(vector<CounterRule>& rules);
    void reader_active();

    // TODO: 增加删除出口交换机
    void processer_pause();
    void processer_active();

    Queue<Message>* get_message_queue() {return &message_queue;}
    map<CounterRule, shared_ptr<Counter>>* get_counter_map() {return &counter_map;}
    unordered_set<int>* get_out_switch_set() {return &out_switch_set;}

    // 线程相关的函数, 启动输入和解析线程.
    void run();
    void join();
private:
    int processer_cnt;    /**< 记录同时处理的processor个数 */
    int reader_cnt;       /**< 记录同时处理的reader个数 */

    int cur_id;           /**< 记录当前分析器的ID 只要初始化获得,
                                一旦确定, 将不会改变 */

    // 以下三个变量只由Watcher进行增加删除, 增加删除时需要将reader或是processer全部暂停
    Queue<Message> message_queue;           /**< 消息队列设置  */
    map<CounterRule, shared_ptr<Counter>> counter_map;  /**< 记录计数器的规则 */
    unordered_set<int> out_switch_set;      /**< 出口交换机的id */


    // 下面两个为reader监听的函数.
    vector<pcap_t*> pcap_vec;
    vector<lcore_queue_conf> lcore_vec;

    vector<shared_ptr<Reader>> reader_vec;  /**< 可以多个线程进行读取,
                                               每个线程可以向多个队列中写入 */

    vector<shared_ptr<Processer>> processer_vec; /**< 多个分析器进行 */
    vector<shared_ptr<PKT_QUEUE>> queue_vec; /**< 每个分析器绑定一个队列 */
};
#endif /* end of include guard: EVER_MAIN_H_5UBIZCWM */
