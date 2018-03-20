#include "ever_main.h"

EverflowMain::EverflowMain() {
    this->processer_cnt = 3;
    for(int i = 0; i < this->processer_cnt; i++) {

        auto p = shared_ptr<Processer>(new Processer());
        shared_ptr<PKT_QUEUE> q = shared_ptr<PKT_QUEUE>(new PKT_QUEUE);

        p->bind_queue(q);
        processer_vec.push_back(p);
        queue_vec.push_back(q);
    }
}

void EverflowMain::run() {
    for(auto p : processer_vec) {
        p->run();
    }

    for(auto p : processer_vec) {
        p->join();
    }
}

EverflowMain::~EverflowMain() {
}
