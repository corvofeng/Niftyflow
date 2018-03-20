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
#include "trace.h"
#include "log.h"
#include "con_queue.h"
#include <memory>
#include "packet.h"

struct HashBacket {
    enum {ROW_SIZE=1024, COL_SIZE=4};
    PKT_TRACE_T bkt[ROW_SIZE][COL_SIZE];
};

using std::shared_ptr;

/**
 * @brief 每个线程持有一个对象, 每个对象启动后其实是两个线程, 分别进行快慢
 * 路径的处理
 *
 * 使用方法:
 *  Processer p;
 *  p.bind_queue(shared_ptr<Queue<IP_PKT>>(new Queue<IP_PKT>))
 *  p.run();
 *  p.join();
 */
class Processer{
private:
    Processer(Processer&); // 禁止拷贝

    int proc_id;
    pthread_t t_fast;
    pthread_t t_slow;

    bool is_slow_over;
    pthread_mutex_t is_over_mtx;

    shared_ptr<PKT_QUEUE> q_;   // USE shared_ptr Pointer

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

    /**
     * @brief 绑定监听队列, 之后将会从队列中读取数据包
     *
     * @param q
     */
    void bind_queue(shared_ptr<PKT_QUEUE> q);

    void join() {
        (void) pthread_join(t_fast, NULL);
        (void) pthread_join(t_slow, NULL);
    }

    /**
     * @brief 为了使用pthread_create, 故而定义static函数, 将self参数放置在
     * context中进行调用, 其实本质是在调用_inner_slow_path()函数
     *
     * @return
     */
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
