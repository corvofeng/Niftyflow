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
    vector<shared_ptr<PKT_QUEUE>> *_queue_vec;
    map<CounterRule, Counter> *_counter_map;
    bool stop;

    pcap_t *_pcap;

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

    void bind_counter_map(map<CounterRule, Counter>* counter_map) {
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

    void join() {
        (void) pthread_join(t_reader, NULL);
    }

    static void* read_and_push(void *context) {
        Reader *r = (Reader*)context;
        r->_inner_read_and_push();
    }
};

#endif /* end of include guard: READER_H_AKS58L6J */
