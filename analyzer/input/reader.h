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
#include <pcap.h>
#include <pthread.h>
#include <vector>

int pcap_read();
using std::vector;
using std::shared_ptr;

class Reader {
    pthread_t t_reader;
    vector<shared_ptr<PKT_QUEUE>> *_queue_vec;
    pcap_t *_pcap;

    void _inner_read_and_push();

    /**
     * @brief 构建解析时需要的数据包
     *
     * @param header
     * @param 
     *
     * @return 
     */
    shared_ptr<PARSE_PKT> _pkt_generater(
            const struct pcap_pkthdr* header,
            const u_char *packet);

    /**
     * @brief 根据数据包的hash结果, 将其放置在不同的队列中
     *
     * @param pkt
     */
    void _push_to_queue(shared_ptr<PARSE_PKT> pkt);
public:
    Reader(): _queue_vec(NULL), _pcap(NULL) {}
    void bind_queue_vec(vector<shared_ptr<PKT_QUEUE>>* queue_vec) {
        this->_queue_vec = queue_vec;
    }

    void run() {
        if(!_pcap || !_queue_vec) {
            throw std::runtime_error("Must bind queue and pcap");
            LOG_D("Must bind queue vec and pcap, then called run\n");
            return ;
        }
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


/**
 * @brief GRE 解除封装
 */
void gre_decap(u_char* data);

#endif /* end of include guard: READER_H_AKS58L6J */
