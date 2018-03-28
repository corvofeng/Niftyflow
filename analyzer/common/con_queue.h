/*
 *=======================================================================
 *    Filename:con_queue.h
 *
 *     Version: 1.0
 *  Created on: March 19, 2018
 *
 *      Author: corvo
 *=======================================================================
 */

#ifndef CON_QUEUE_H_NWUO1JLQ
#define CON_QUEUE_H_NWUO1JLQ


/**
 * copy from
 * https://juanchopanzacpp.wordpress.com/2013/02/26/concurrent-queue-c11/
 */

#include <pthread.h>
// #include <mutex>
// #include <condition_variable>
#include <queue>
#include "lock.h"

using std::queue;

template <typename T>
class Queue
{
    public:
        T pop()
        {
            Lock l(&mutex_);
            while (queue_.empty())
            {
                pthread_cond_wait(&cond_, &mutex_); /** 此时期望的行为: 进程进入休眠状态 */
            }
            auto item = queue_.front();
            queue_.pop();
            return item;
        }

        void pop(T& item)
        {
            Lock l(&mutex_);
            while (queue_.empty())
            {
                pthread_cond_wait(&cond_, &mutex_); /** 此时期望的行为: 进程进入休眠状态 */
            }
            item = queue_.front();

            queue_.pop();
        }

        void push(const T& item)
        {
            Lock l(&mutex_);
            queue_.push(item);
            pthread_cond_signal(&cond_); /** 解除休眠状态 */
        }

        void push(T&& item)
        {
            Lock l(&mutex_);
            queue_.push(std::move(item));
            pthread_cond_signal(&cond_); /** 解除休眠状态 */
        }

        Queue() {
            pthread_mutex_init(&mutex_, NULL);
            pthread_cond_init(&cond_, NULL);
        }
        ~Queue() {
            pthread_mutex_destroy(&mutex_);
            pthread_cond_destroy(&cond_);
        }

    private:
        std::queue<T> queue_;
        pthread_mutex_t mutex_;
        pthread_cond_t cond_;
        // std::mutex mutex_;
        // std::condition_variable cond_;
};


#endif /* end of include guard: CON_QUEUE_H_NWUO1JLQ */
