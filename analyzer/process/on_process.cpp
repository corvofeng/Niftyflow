#include "on_process.h"
#include <sys/socket.h> // for inet_ntoa inet_aton
#include <netinet/in.h>
#include <arpa/inet.h>


Processer::Processer(): _stop(false){

}

Processer::~Processer() {

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
 */
static int bkt_hash_func_(const PARSE_PKT* pkt);

/**
 * @brief 将pkt的信息填入到trace数据所在的位置中
 */
static void trace_add_pkt(PKT_TRACE_T* trace, const PARSE_PKT* pkt);

/**
 * @brief hash值相同时, 检查Key值, 判断数据包中是否真的与trace中的路径是匹配的.
 */
static bool is_exact_trace(const PKT_TRACE_T* p_trace, const PARSE_PKT* pkt);

void Processer::bind_queue(shared_ptr<PKT_QUEUE> q) {
    this->q_ = q;
    return ;
}

void Processer::_inner_slow_path() {
    LOG_D("Inner slow path\n");
    while(!_stop) {
    }
}

void Processer::_inner_fast_path() {
    LOG_D("Inner fast path\n");
    while(!_stop) {
        shared_ptr<PARSE_PKT> pkt = this->q_->pop();
        LOG_D("src: " << inet_ntoa(pkt->ip_inner->ip_src) << "\n");

        PKT_TRACE_T *saved_trace = NULL;

        // 确定索引
        int bkt_num = bkt_hash_func_(pkt.get()) % ROW_SIZE;

        do {
            // 从桶0中寻找
            saved_trace = find_trace_in_bkt(_bkts[0].b[bkt_num], pkt.get());
            if(saved_trace) {
                break;
            }

            // 从桶1中查找
            saved_trace = find_trace_in_bkt(_bkts[1].b[bkt_num], pkt.get());
            if(saved_trace) {
                break;
            }

            // 桶0, 桶1, 均没有匹配的trace, 从桶1中寻找第一个空的数据结构
            PKT_TRACE_T* slot = _bkts[1].b[bkt_num];
            for(int i = 0; i < COL_SIZE; i++) {
                if(!slot[i].used) {
                    saved_trace = &slot[i];
                    break;
                }
            }

        }while(0);
        if(saved_trace == NULL) {
            LOG_W("There is not enough slots\n");
        }
        trace_add_pkt(saved_trace, pkt.get());
    }
}

PKT_TRACE_T* Processer::find_trace_in_bkt(PKT_TRACE_T bkt_slot[COL_SIZE],
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
    if(!trace->used) {
        trace->used = true;
        trace->key.src_ip = pkt->ip_inner->ip_src.s_addr;
        trace->key.dst_ip = pkt->ip_inner->ip_dst.s_addr;
        trace->key.protocol = pkt->ip_inner->ip_p;
        trace->key.ip_id = pkt->ip_inner->ip_id;
        trace->hp1_switch_id = c;

        trace->hp1_rcvd = 1;
        trace->is_loop = 0;
        trace->is_drop = 0;
        trace->is_timeout = 0;
        trace->is_probe = 0;
    } else {
    
    }
}

static bool is_exact_trace(const PKT_TRACE_T* p_trace, const PARSE_PKT* pkt) {
    struct sniff_ip* ip = pkt->ip_inner;

    return (ip->ip_src.s_addr  == p_trace->key.src_ip) &&
            (ip->ip_dst.s_addr == p_trace->key.dst_ip) &&
            (ip->ip_p          == p_trace->key.protocol) &&
            (ip->ip_id         == p_trace->key.ip_id);
}


static int bkt_hash_func_(const PARSE_PKT* pkt) {
    struct sniff_ip* ip = pkt->ip_inner;    // 使用内层的IP数据包
    int ret =
        ((size_t) ip->ip_src.s_addr * 59 ) ^
        ((size_t) ip->ip_dst.s_addr) ^
        ((size_t) ip->ip_p) ^
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
