#include "on_process.h"
#include <sys/socket.h> // for inet_ntoa inet_aton
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h> // for clock
#include <sys/time.h>   // for gettimeofday


Processer::Processer(): _stop(false){
    for(int i = 0; i < BKT_SIZE; i++) {
        _bkts[i].b = new PKT_TRACE_T*[ROW_SIZE];
        for(int j = 0; j < ROW_SIZE; j++) {
            _bkts[i].b[j] = new PKT_TRACE_T[COL_SIZE];
            memset(_bkts[i].b[j], 0, COL_SIZE * sizeof(PKT_TRACE_T));
        }
    }
}

Processer::~Processer() {
    for(int i = 0; i < BKT_SIZE; i++) {
        for(int j = 0; j < ROW_SIZE; j++) {
            delete [] _bkts[i].b[j];
        }
        delete [] _bkts[i].b;
    }
}

/**
 * @brief 通过数据包信息, 确定switch_id.
 */
static unsigned char query_switch_id(const PARSE_PKT* pkt);

/**
 * @brief trace数据中添加新的数据包.
 */
static void trace_add_bkt(PKT_TRACE_T* trace, const PARSE_PKT* pkt);

/**
 * @brief 确定数据包所在的hash桶的位置.
 * trace路径通过源IP, 目的IP, IP的标识, 协议来确定, 也就是数据包中内层IP的这些信息.
 * 只能返回正数!!
 */
static unsigned int bkt_hash_func_(const PARSE_PKT* pkt);

/**
 * @brief 将pkt的信息填入到trace数据所在的位置中
 *
 * 填入时有两种情况:
 *  1. 当前的trace数据未使用, 则对其进行初始操作.
 *  2. 当前的trace数据已经被使用, 寻找当前数据包这一跳的位置, 并且检查丢包与环路
 */
static void trace_add_pkt(PKT_TRACE_T* trace, const PARSE_PKT* pkt);

/**
 * @brief hash值相同时, 检查Key值, 判断数据包中是否真的与trace中的路径是匹配的.
 */
static bool is_exact_trace(const PKT_TRACE_T* p_trace, const PARSE_PKT* pkt);


void Processer::_inner_slow_path() {
    LOG_D("Inner slow path\n");
    while(!_stop) {
        int wait_times = 0;
        while(is_slow_over) wait_times++; // spinlock
        if(wait_times > PROCESS_THRESHOLD) LOG_W("FAST PROCESS IS TOO SLOW, JUST FINE\n");

        LOG_D("Parse Bucket in slow path\n");

        auto _bkt = _bkts[2].b;
        for(int i = 0; i < ROW_SIZE; i++) {
            for(int j = 0; j < COL_SIZE; j++) {
                //只要桶中前方的元素没被占用, 那么后方的元素也不可能被使用,
                //直接跳出即可
                PKT_TRACE_T* t = &_bkt[i][j];
                if(t->used == 0)
                    break;

                if(t->is_loop) {
                    LOG_D("CHECK LOOP\n");
                }

                if(t->is_drop) {
                    LOG_D("CHECK DROP\n");
                }
            }
            // 重置当前槽, 以提供后续使用
            memset(_bkt[i], 0, sizeof(PKT_TRACE_T) * COL_SIZE);
        }

        {
            Lock l(&is_over_mtx);
            is_slow_over = true;
        }
    }
}

void Processer::_inner_fast_path() {
    LOG_D("Inner fast path\n");

    clock_t start = clock();
    while(!_stop) {
        start = clock();
        shared_ptr<PARSE_PKT> pkt = this->q_->pop();
        LOG_D("src: " << inet_ntoa(pkt->ip_inner->ip_src) << "\n");

        // 确定桶中的索引
        unsigned int bkt_num = bkt_hash_func_(pkt.get()) % ROW_SIZE;
        LOG_D("Get bkt_idx " << bkt_num << "\n");

        PKT_TRACE_T *saved_trace = NULL;

        do {// 确定数据包可以插入的trace

            // 从桶0中寻找
            saved_trace = find_trace_in_bkt(_bkts[0].b[bkt_num], pkt.get());
            if(saved_trace) break;

            // 从桶1中查找
            saved_trace = find_trace_in_bkt(_bkts[1].b[bkt_num], pkt.get());
            if(saved_trace) break;

            // 桶0, 桶1, 均没有匹配的trace, 试图从桶1中寻找第一个空的数据结构
            PKT_TRACE_T* slot = _bkts[1].b[bkt_num];
            for(int i = 0; i < COL_SIZE; i++) {
                if(!slot[i].used) {
                    saved_trace = &slot[i];
                    break;
                }
            }
        } while(0);

        if(saved_trace == NULL) {   // 检查是否找到可以接受数据包的Trace
            LOG_E("There is not enough slots\n");
            return ;
        }

        trace_add_pkt(saved_trace, pkt.get());

        if(clock() - start > 1000) {
            int wait_times = 0;
            while(!is_slow_over) wait_times++ ;  // spinlock, 检查慢路径是否结束
            if(wait_times > PROCESS_THRESHOLD) LOG_W("SLOW PROCESS IS TOO SLOW\n");

            auto TMP = _bkts[0].b;
            _bkts[0].b = _bkts[1].b;
            _bkts[1].b = _bkts[2].b;
            _bkts[2].b = TMP;

            {
                Lock l(&is_over_mtx);
                is_slow_over = false;
            }
        }
    }
}

PKT_TRACE_T* Processer::find_trace_in_bkt(PKT_TRACE_T* bkt_slot,
                                    const PARSE_PKT* pkt) {
    PKT_TRACE_T *slot = bkt_slot;
    for(int i = 0; i < COL_SIZE; i++) {
        if(slot[i].used && is_exact_trace(&slot[i], pkt)) {
            return &slot[i];
        }
    }
    return NULL;
}

static void trace_add_pkt(PKT_TRACE_T* trace, const PARSE_PKT* pkt) {
    int c = query_switch_id(pkt);

    LOG_D("Read a new packet:" << c << "\n");

    struct timeval tv_shift = pkt->header.ts;
    int time_stamp = GET_TIMESTAMP_OF_DAY(tv_shift);

    // LOG_D(FMT("Epoch Time: %d: %d seconds\n", tv_shift.tv_sec, tv_shift.tv_usec));

    if(!trace->used) {  /** 当前trace第一次使用  */
        trace->used = true;
        trace->key.src_ip = pkt->ip_inner->ip_src.s_addr;
        trace->key.dst_ip = pkt->ip_inner->ip_dst.s_addr;
        trace->key.protocol = pkt->ip_inner->ip_p;
        trace->key.ip_id = pkt->ip_inner->ip_id;
        trace->hp1_switch_id = c;

        trace->hp1_rcvd = 1;
        trace->is_loop = 0;
        trace->is_drop = 1;     // 初始化drop为1, 满足条件后drop置0.
        trace->is_timeout = 0;
        trace->is_probe = 0;

        trace->timestart = time_stamp;

    } else {        /** 当前trace已经存在, 现在向其中添加一跳的数据包 */
        if (trace->hp1_switch_id = c) {
            trace->hp1_rcvd ++;

            // 一个交换机出现了三次, 认为是环路
            if(trace->hp1_rcvd >= 3)trace->is_loop = true;
        }

        // 检查之前的trace数据中, 是否已经存在来自该交换机的数据包
        int exist_idx = -1;
        for(int i = 0; i < TRACE_CNT - 1; i++) {
            if(IS_SAME_ID(c, trace->hop_info[i].switch_id)) {
                exist_idx = i;
                break;
            }
        }

        if(exist_idx == -1) { /** 之前不存在相同的交换机, 寻找第一个可以放置的位置进行放置 */
            bool is_full = true;
            for(int i = 0; i < TRACE_CNT - 1; i++) {
                if(trace->hop_info[i].hop_rcvd == 0) {
                    trace->hop_info[i].switch_id = c;
                    trace->hop_info[i].hop_rcvd = 1;
                    trace->hop_info[i].hop_timeshift =
                                (time_stamp - trace->timestart) & 0x000003FF;
                    is_full = false;
                    // trace->is_drop = false; // 当前认为所有Trace数据包均丢包
                    break;
                }
            }
            if (is_full) {  // 路径上有超过5个不同的交换机, 目前认为是丢包
                LOG_E("There is a trace exceed 5 hops\n");
                trace->is_drop = true;
            }

        } else {    // 之前已有, 需要更新之前的信息
            trace->hop_info[exist_idx].hop_rcvd ++;
            if(trace->hop_info[exist_idx].hop_rcvd >= 3) trace->is_loop = true;
        }
    }
}

static bool is_exact_trace(const PKT_TRACE_T* p_trace, const PARSE_PKT* pkt) {
    struct sniff_ip* ip = pkt->ip_inner;

    return (ip->ip_src.s_addr  == p_trace->key.src_ip)   &&
            (ip->ip_dst.s_addr == p_trace->key.dst_ip)   &&
            (ip->ip_p          == p_trace->key.protocol) &&
            (ip->ip_id         == p_trace->key.ip_id);
}

static unsigned int bkt_hash_func_(const PARSE_PKT* pkt) {
    struct sniff_ip* ip = pkt->ip_inner;    // 使用内层的IP数据包
    unsigned int ret =
        ((size_t) ip->ip_src.s_addr * 59 ) ^
        ((size_t) ip->ip_dst.s_addr)       ^
        ((size_t) ip->ip_p)                ^
        ((size_t) ip->ip_id);
    return ret;
}

static unsigned char query_switch_id(const PARSE_PKT* pkt) {

    // 目前取外层数据包的源IP地址的最后一个字节, pkt中使用的是大端字节序
    // 所以取最后一个char即可
    // tricks: 这里使用了`char* + 3`取出其地址, 再获得其数据
    unsigned char* s = (unsigned char *)(&pkt->ip_outer->ip_src);
    return *(s + 3);
}

void Processer::bind_queue(shared_ptr<PKT_QUEUE> q) {
    this->q_ = q;
    return ;
}
