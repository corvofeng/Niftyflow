#include "ever_main.h"

EverflowMain::EverflowMain() {
    this->processer_cnt = 3;
    this->reader_cnt = 1;   // 目前为单线程读入

    std::string file = "/home/corvo/Dropbox/课程文件/毕业设计/grecap.cap";
    char errbuff[PCAP_ERRBUF_SIZE];

    pcap_t * p = pcap_open_offline(file.c_str(), errbuff);
    pcap_vec.push_back(p);

    for(int i = 0; i < this->processer_cnt; i++) {

        auto p = shared_ptr<Processer>(new Processer());
        shared_ptr<PKT_QUEUE> q = shared_ptr<PKT_QUEUE>(new PKT_QUEUE);

        p->bind_queue(q);
        processer_vec.push_back(p);
        queue_vec.push_back(q);
    }

    for(int i = 0; i < reader_cnt && i < pcap_vec.size(); i++) {
        auto r = shared_ptr<Reader>(new Reader());
        r->bind_queue_vec(&this->queue_vec);
        r->bind_pcap(pcap_vec[i]);
        reader_vec.push_back(r);
    }
}

void EverflowMain::run() {
    for(auto p : processer_vec)
        p->run();
    for(auto r: reader_vec)
        r->run();

    for(auto p : processer_vec)
        p->join();

    for(auto r: reader_vec)
        r->join();
}

EverflowMain::~EverflowMain() {

    for(auto p: pcap_vec) {
        pcap_close(p);
    }
}
