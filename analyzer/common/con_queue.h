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

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class Queue
{
    public:
        T pop()
        {
            std::unique_lock<std::mutex> mlock(mutex_);
            while (queue_.empty())
            {
                cond_.wait(mlock);
            }
            auto item = queue_.front();
            queue_.pop();
            return item;
        }

        void pop(T& item)
        {
            std::unique_lock<std::mutex> mlock(mutex_);
            while (queue_.empty())
            {
                cond_.wait(mlock);
            }
            item = queue_.front();
            queue_.pop();
        }

        void push(const T& item)
        {
            std::unique_lock<std::mutex> mlock(mutex_);
            queue_.push(item);
            mlock.unlock();
            cond_.notify_one();
        }

        void push(T&& item)
        {
            std::unique_lock<std::mutex> mlock(mutex_);
            queue_.push(std::move(item));
            mlock.unlock();
            cond_.notify_one();
        }

    private:
        std::queue<T> queue_;
        std::mutex mutex_;
        std::condition_variable cond_;
};


#endif /* end of include guard: CON_QUEUE_H_NWUO1JLQ */
