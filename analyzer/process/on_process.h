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
#include <mysql.h>
#include "con_queue.h"
#include <memory>
#include "packet.h"

/**
 *  快慢路径中的切换使用了自旋锁, 真正的处理过程中, 执行较慢的一个线程将会拖慢
 *  另一个, 如果是这样, 需要针对慢的线程进行优化.
 *
 * 2018-03-30: 为每个Processor添加一个`MYSQL*`的连接对象.
 *
 * 2018-03-23: 今天已经将快慢路径的框架实现, 接下来将要试着保存至数据库与Redis中
 *
 * 2018-03-22: 今天进行了讨论, 其中有几个要点:
 *            1. 使用外层数据的源IP地址标识路由器 ID,
 *            2. 判断有无丢包, 检查trace路径中有至少两个出口交换机.
 *
 * 2018-03-21: 开始编写快路径的处理, 发现了一个比较难的问题, 就是交换机的ID
 *           很难直接确定. 理想的情况是数据包中携带自己的交换机信息,
 *           但是目前还没有.
 *
 *             根据目前的数据包结构以及数据包中的数据分布, 暂时使用外层
 *           IP数据包源地址的, 例如IP源地址是190.1.1.19, 则取19作为交换机ID,
 *           并不是说这样做是正确的选择.
 *
 *             假如传播过程中使用了三层交换机, 那么这个IP并不是真正的交换机IP,
 *          同样, 使用了二层交换机后, 不论是哪个MAC地址, 都不能称之为真正交换机
 *          的MAC地址.
 *
 *            如果我们想要得知真正的交换机ID, 最好是在GRE数据包中的可选位加入
 *          交换机ID, 这样才能得到不经其他交换机修改的ID信息.
 *
 *            ID信息的获取请查看`on_process.cpp`中的static函数
 */

enum {BKT_SIZE = 3, ROW_SIZE=1024, COL_SIZE=4}; // 使用enum只是因为他不占空间, 类似于define;
enum {PROCESS_THRESHOLD = 1000000};             // 如果处理需要等待这么久, 就需要报警了

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
    bool _stop;
    pthread_t t_fast;
    pthread_t t_slow;

    struct {   // NOTE: 为了理解方便, 这里没有使用3维数组, 使用匿名类更方便
        PKT_TRACE_T **b;                /**< 二维数组, 在构造函数中初始化 */
    }  _bkts[BKT_SIZE];                 /**<  三个hash桶  */

    bool is_slow_over;
    pthread_mutex_t is_over_mtx;

    shared_ptr<PKT_QUEUE> q_;   /**< USE shared_ptr Pointer */
    MYSQL* conn;                /**< MySQL连接对象          */

    void get_packets();
    void _inner_slow_path();
    void _inner_fast_path();

    /**
     * 从指定桶中寻找数据包, 并将其所在的地址返回, 如果找不到, 则返回NULL
     */
    PKT_TRACE_T* find_trace_in_bkt(PKT_TRACE_T bkt[COL_SIZE],
                                    const PARSE_PKT* pkt);

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
