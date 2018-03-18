/*
 *=======================================================================
 *    Filename:on_process.h
 *
 *     Version: 1.0
 *  Created on: March 18, 2018
 *
 *      Author: corvo
 *=======================================================================
 */
#ifndef ON_PROCESS_H_0N731EYB
#define ON_PROCESS_H_0N731EYB

#include <pthread.h>
#include <stdio.h>

/**
 * @brief 每个线程持有一个对象
 */
class Processer{
private:
    Processer(Processer&); // 禁止拷贝


    int proc_id;
    pthread_t t_fast;
    pthread_t t_slow;

    bool is_slow_over;
    pthread_mutex_t is_over_mtx;

    void get_packets();
    void _inner_slow_path();
    void _inner_fast_path();

public:
    Processer();
    ~Processer();
    void run() {
        pthread_create(&t_fast, NULL, &Processer::fast_path_process, this);
        pthread_create(&t_slow, NULL, &Processer::slow_path_process, this);
    }

    void join() {
        (void) pthread_join(t_fast, NULL);
        (void) pthread_join(t_slow, NULL);
    }

    static void* slow_path_process(void *context) {
        Processer *p = (Processer*)context;
        p->_inner_slow_path();
    }
    static void* fast_path_process(void *context) {
        Processer *p = (Processer*)context;
        p->_inner_fast_path();
    }
};


#endif /* end of include guard: ON_PROCESS_H_0N731EYB */
