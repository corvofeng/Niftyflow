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

#include <mutex>

/**
 *  线程安全的计数器, 由于采用了多线程的输入方式, 读
 *  数据时必须加锁进行.
 */

class Counter {
public:
    Counter(): cnt(0) {}

    Counter(const int i): cnt(i) {}
    void set_zero() {
        std::unique_lock<std::mutex> mlock(mutex_);
        this->cnt = 0;
    }

    /**
     * @brief: 获取当前值 并清零
     *
     */
    int get_and_clear() {
        std::unique_lock<std::mutex> mlock(mutex_);
        int ret = this->cnt;
        this->cnt = 0;
        return ret;
    }

    /**
     * @brief cnt+=1
     */
    void add_one() {
        std::unique_lock<std::mutex> mlock(mutex_);
        this->cnt += 1;
    }

private:
    int cnt;
    std::mutex mutex_;
};


#endif /* end of include guard: ATOM_COUNTER_H_XYKFT3NZ */
