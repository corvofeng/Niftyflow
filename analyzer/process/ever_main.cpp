#include "ever_main.h"

EverflowMain::EverflowMain() {
    this->process_cnt = 3;
    for(int i = 0; i < this->process_cnt; i++) {

        auto p = shared_ptr<Processer>(new Processer());
        auto q_ = shared_ptr<Queue<IP_PKT>>(new Queue<IP_PKT>);

        p->bind_queue(q_);
        processer_vec.push_back(p);
        queue_vec.push_back(q_);
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
