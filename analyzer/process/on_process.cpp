#include "on_process.h"


Processer::Processer() {

}

Processer::~Processer() {

}

void Processer::bind_queue(std::shared_ptr<Queue<IP_PKT>> q) {
    this->q_ = q;
    return ;
}

void Processer::_inner_slow_path() {
    LOG_D("Inner slow path\n");

}

void Processer::_inner_fast_path() {
    LOG_D("Inner fast path\n");

}
