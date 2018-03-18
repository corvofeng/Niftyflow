#include "on_process.h"


Processer::Processer() {

}

Processer::~Processer() {

}

void Processer::_inner_slow_path() {
    printf("Inner slow path\n");
}

void Processer::_inner_fast_path() {
    printf("Inner fast path\n");
}
