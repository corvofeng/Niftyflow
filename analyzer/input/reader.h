/*
 *=======================================================================
 *    Filename:reader.h
 *
 *     Version: 1.0
 *  Created on: March 18, 2018
 *
 *      Author: corvo
 *=======================================================================
 */

#ifndef READER_H_AKS58L6J
#define READER_H_AKS58L6J

#include "log.h"
#include "packet.h"
#include "cnt_rule.h"
#include "atom_counter.h"
#include "dpdk_adapter.h"
#include <pcap.h>
#include <pthread.h>
#include <vector>
#include <map>
#include <atomic>

using std::vector;
using std::map;
using std::shared_ptr;

/**
 * 2018-04-13: 准备添加对DPDK的支持, 想在使用pcap库的相同位置, 插入DPDK代码.
 *
 *             无锁队列: 在DPDK内部, 已经实现了无锁队列, 与当前程序中队列的
 *                 实现目的是一致的. 可以进行修改.
 *                 具体查看例程中的Distributor Sample Application
 *
 *             线程监听多个队列: 对于多网卡设备, 可以在一个DPDK的收包线程中监听
 *                 多个网卡设备, 从而对一个线程中进行复用, DPDKTest就是这样方式.
 *                 具体查看例程中的Load Balancer Sample Application
 *
 *             例程查看: https://dpdk.org/doc/guides/sample_app_ug/index.html
 *
 *            当我们暂时无法确定程序的瓶颈时, 我想
 *            还是以最简单的方式进行即可, 而后再慢慢提高性能. 当前的程序一定
 *            不是最快的, 但却是最稳定的
 *
 *
 * 2018-03-29: 将Reader中对pause的判断提前, 可以保证程序没有被初始化时
 *             整个的Reader是暂停的.
 *
 * 2018-03-26: 在解析数据包完成后, 立刻进行计数器累加操作
 *
 * 2018-03-23: 初次将break去掉, 使用valgrind检测, 程序运行正常, 检测出几个丢包
 *              问题
 *
 * 2018-03-22: 使用valgrind检测, 一直发现内存错误, 原因是分配内存时使用了
 *              `pkt->_data = new u_char[pkt->header.len];`
 *              这里的pkt->header还没有进行赋值, 所以其len是0的. 之后想要读
 *              内存就会发生错误. 改为
 *                  `pkt->_data = new u_char[>header->len];`之后使用valgrind
 *              测试, 已经可以通过.
 *
 * 2018-03-21: 添加hash函数, 进过解析后的数据包将会进行hash操作, 而后将其放在
 *              相应的队列中
 */

enum {M_PCAP, M_DPDK};

class Reader {
private:
    pthread_t t_reader;

    /**
     * @brief 多个队列, 每个队列有自己的互斥锁.
     */
    vector<shared_ptr<PKT_QUEUE>> *_queue_vec;


    /**
     * @brief 计数器规则, 首先, 这是一个可能会被更改的map, 我们可能会动态的添加
     *          删除规则. 目前我想到的方式是:
     *          1. 使许多读入线程空转, 在线程空转期间修改规则.
     *              如果我们的添加删除操作并不那么频繁, 那这样做是可以的
     *          2. 直接使用互斥锁, 一个线程更新时其他线程无法读取,
     *              但这就有了另一种情况, 如果我们使用一个互斥锁, 可能多线程读取
     *              时也会产生互斥. 如果使用多个互斥锁又不能达到要求.
     *
     *      最终我决定计数器规则选择第一种, 使用自旋锁, 需要修改时请求暂停,
     *      在得到is_pause_ok返回true时, 证明线程暂停成功,立刻进行规则的增加与
     *      删除.
     */
    map<CounterRule, shared_ptr<Counter>> *_counter_map;

    std::atomic_bool _force_quit;

    std::atomic_bool is_pause;  /**< 返回暂停成功, 初始时认为程序处于暂停状态 */
    std::atomic_bool pause;     /**< 外部控制变量, 置位为1表示我们需要程序暂停 */

    int mode;       /**< 表示该程序属于DPDK或是PCAP模式 */


    // 接下来的两个分别代表不同的获取数据的方式
    pcap_t                  *_pcap;     /**< pcap句柄, 不断访问句柄以获得数据 */
    struct lcore_queue_conf *_lcore_queue; /**< 每个Reader监听的多个队列 */
    unsigned lcore_id;                  /**< 标示当前的线程ID */

    void _inner_pcap_read_and_push();
    void _inner_dpdk_read_and_push();

    /**
     * @brief 构建解析时需要的数据包
     */
    shared_ptr<PARSE_PKT> _pkt_generater(
            const struct pcap_pkthdr* header,
            const u_char *packet);

    void run_counter(shared_ptr<PARSE_PKT> pkt);

    /**
     * @brief 根据数据包的hash结果, 将其放置在不同的队列中
     */
    void _push_to_queue(shared_ptr<PARSE_PKT> pkt);

public:
    explicit Reader():
            _queue_vec(NULL), _pcap(NULL), _lcore_queue(NULL),
            _counter_map(NULL), _force_quit(false),
            is_pause(true), pause(true), mode(M_PCAP) {}

    void set_mode(int _m) {
        this->mode = _m;
    }

    void bind_queue_vec(vector<shared_ptr<PKT_QUEUE>>* queue_vec) {
        this->_queue_vec = queue_vec;
    }

    void bind_counter_map(map<CounterRule, shared_ptr<Counter>>* counter_map) {
        this->_counter_map = counter_map;
    }

    void run() {
        LOG_D("Creat Reader process\n");
        if(M_PCAP == mode) {    // PCAP模式
            LOG_I("In PCAP mode start!!!\n");
            LOG_D(FMT("pcap       : %d\n", !(_pcap == NULL)));
            LOG_D(FMT("queue_vec  : %d\n", !(_queue_vec == NULL)));
            LOG_D(FMT("counter_map: %d\n", !(_counter_map == NULL)));

            if(!_pcap || !_queue_vec || !_counter_map) {
                LOG_D("Must bind queue vec and pcap, then called run\n");
                throw std::runtime_error("Must bind queue and pcap");
                return ;
            }
            pthread_create(&t_reader, NULL, &Reader::pcap_read_and_push, this);
        } else if(M_DPDK == mode) { // DPDK 模式
            LOG_I("In DPDK mode start!!!\n");
            if(!_lcore_queue) {
                LOG_D("Must bind lcore_queue , then called run\n");
                throw std::runtime_error("Must bind lcore_queue");
            }
            rte_eal_mp_remote_launch(&Reader::dpdk_read_and_push,
                                this, CALL_MASTER);
        }
    }

    void bind_pcap(pcap_t *pcap){
        this->_pcap = pcap;
    }

    void bind_dpdk(lcore_queue_conf *lcore_queue_conf, unsigned lcore_id){
        this->_lcore_queue = lcore_queue_conf;
        this->lcore_id = lcore_id;
    }

    // 强制当前的Reader停止
    void make_quit() {
        LOG_D("READER STOP\n");
        this->_force_quit = true;
    }

    // 暂停与重新启动, 主要是在更新删除计数器规则时使用
    // 如果暂停, 所有的信息将会立刻停止解析, 所有读线程会停下来
    void do_pause() {
        this->pause = true;
    }

    void cancel_pause() {
        this->is_pause = false;
        this->pause = false;
    }

    bool is_pause_ok() {
        return this->is_pause;
    }

    void join() {
        if(M_PCAP == mode) {
            (void) pthread_join(t_reader, NULL);
            LOG_I("In PCAP mode stop!!!\n");

        } else if (M_DPDK == mode) {
            unsigned lcore_id;
            if (rte_eal_wait_lcore(lcore_id) < 0) {
                LOG_E("DPDK wait close error\n");
            }
            LOG_I("In DPDK mode stop!!!\n");

        }
    }

    static void* pcap_read_and_push(void *context) {
        Reader *r = (Reader*)context;
        r->_inner_pcap_read_and_push();
    }

    static int dpdk_read_and_push(void *context) {
        Reader* r = (Reader*)context;
        r->_inner_dpdk_read_and_push();
    }
};

#endif /* end of include guard: READER_H_AKS58L6J */
