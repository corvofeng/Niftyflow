/*
 *=======================================================================
 *    Filename:reader.h
 *
 *     Version: 1.0
 *  Created on: March 18, 2018
 *
 *      Author: corvo
 *=======================================================================
 */

#ifndef READER_H_AKS58L6J
#define READER_H_AKS58L6J

#include "log.h"
#include "packet.h"
#include "cnt_rule.h"
#include "atom_counter.h"
#include <pcap.h>
#include <pthread.h>
#include <vector>
#include <map>

int pcap_read();
using std::vector;
using std::map;
using std::shared_ptr;

class Reader {
private:
    pthread_t t_reader;

    /**
     * @brief 多个队列, 每个队列有自己的互斥锁.
     */
    vector<shared_ptr<PKT_QUEUE>> *_queue_vec;


    /**
     * @brief 计数器规则, 首先, 这是一个可能会被更改的map, 我们可能会动态的添加
     *          删除规则. 目前我想到的方式是:
     *          1. 使许多读入线程空转, 在线程空转期间修改规则.
     *              如果我们的添加删除操作并不那么频繁, 那这样做是可以的
     *          2. 直接使用互斥锁, 一个线程更新时其他线程无法读取, 
     *              但这就有了另一种情况, 如果我们使用一个互斥锁, 可能多线程读取
     *              时也会产生互斥. 如果使用多个互斥锁又不能达到要求.
     *
     *      最终我决定计数器规则选择第一种, 使用自旋锁, 需要修改时请求暂停, 
     *      在得到is_pause_ok返回true时, 证明线程暂停成功,立刻进行规则的增加与
     *      删除.
     */
    map<CounterRule, shared_ptr<Counter>> *_counter_map;
    bool stop;

    pcap_t *_pcap;
    bool is_pause;  // 返回暂停成功
    bool pause;

    void _inner_read_and_push();

    /**
     * @brief 构建解析时需要的数据包
     */
    shared_ptr<PARSE_PKT> _pkt_generater(
            const struct pcap_pkthdr* header,
            const u_char *packet);

    void run_counter(shared_ptr<PARSE_PKT> pkt);

    /**
     * @brief 根据数据包的hash结果, 将其放置在不同的队列中
     */
    void _push_to_queue(shared_ptr<PARSE_PKT> pkt);

public:
    Reader(): _queue_vec(NULL), _pcap(NULL), _counter_map(NULL), stop(false) {}
    void bind_queue_vec(vector<shared_ptr<PKT_QUEUE>>* queue_vec) {
        this->_queue_vec = queue_vec;
    }

    void bind_counter_map(map<CounterRule, shared_ptr<Counter>>* counter_map) {
        this->_counter_map = counter_map;
    }

    void run() {
        if(!_pcap || !_queue_vec || !_counter_map) {
            throw std::runtime_error("Must bind queue and pcap");
            LOG_D("Must bind queue vec and pcap, then called run\n");
            return ;
        }
        LOG_D("Creat Reader process\n");
        pthread_create(&t_reader, NULL, &Reader::read_and_push, this);
    }

    void bind_pcap(pcap_t *pcap){
        this->_pcap = pcap;
    }

    // 暂停与重新启动, 主要是在更新删除计数器规则时使用
    // 如果暂停, 所有的信息将会立刻停止解析, 所有读线程会停下来
    void do_pause() {
        this->pause = true;
    }

    void cancel_pause() {
        this->is_pause = false;
        this->pause = false;
    }

    bool is_pause_ok() {
        return this->is_pause;
    }

    void join() {
        (void) pthread_join(t_reader, NULL);
    }

    static void* read_and_push(void *context) {
        Reader *r = (Reader*)context;
        r->_inner_read_and_push();
    }
};

#endif /* end of include guard: READER_H_AKS58L6J */
