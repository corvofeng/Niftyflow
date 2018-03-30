/*
 *=======================================================================
 *    Filename:atom_counter.h
 *
 *     Version: 1.0
 *  Created on: March 26, 2018
 *
 *      Author: corvo
 *=======================================================================
 */

#ifndef ATOM_COUNTER_H_XYKFT3NZ
#define ATOM_COUNTER_H_XYKFT3NZ

#include "lock.h"

/**
 * 2018-03-30: 将STL的mutex改为pthread_mutex_t, 今后的程序绝不混用
 *
 *  线程安全的计数器, 由于采用了多线程的输入方式, 读
 *  数据时必须加锁进行.
 */

class Counter {
public:
    Counter(): cnt(0) {
        pthread_mutex_init(&mutex_, NULL);
    }

    Counter(const int i): cnt(i) {
        pthread_mutex_init(&mutex_, NULL);
    }
    ~Counter() {
        pthread_mutex_destroy(&mutex_);
    }
    void set_zero() {
        Lock l(&mutex_);
        this->cnt = 0;
    }

    /**
     * @brief: 获取当前值 并清零
     *
     */
    int get_and_clear() {
        Lock l(&mutex_);
        int ret = this->cnt;
        this->cnt = 0;
        return ret;
    }

    /**
     * @brief cnt+=1
     */
    void add_one() {
        Lock l(&mutex_);
        this->cnt += 1;
    }

private:
    int cnt;
    pthread_mutex_t mutex_;
};


#endif /* end of include guard: ATOM_COUNTER_H_XYKFT3NZ */
