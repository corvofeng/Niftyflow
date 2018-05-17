/*
 *=======================================================================
 *    Filename:reader.cpp
 *
 *     Version: 1.0
 *  Created on: March 20, 2018
 *
 *      Author: corvo
 *=======================================================================
 */

#include <string>
#include "reader.h"
#include <sys/time.h>

using namespace std;

/**
 * @brief 根据p->_data构建完整的数据包p, static表示仅在该文件中使用, 下面两个
 *        并不想提供给别人使用, 仅仅作为Reader功能的一部分, 单独来维护.
 *        返回0表示初始化成功
 */
static int pkt_init(shared_ptr<PARSE_PKT> p);

/**
 * https://stackoverflow.com/questions/3215232/hash-function-for-src-dest-ip-port
 * @brief 根据数据包的元素进行hash操作, 并没有什么特别的尝试.
 *          目前对于非TCP数据包, 直接返回0.
 */
static int hash_func(shared_ptr<PARSE_PKT> pkt);

void Reader::_inner_dpdk_read_and_push() {
#define MAX_PKT_BURST 32
    LOG_I("_inner_dpdk_read_and_push\n");

    struct lcore_queue_conf *qconf;
    struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
    struct rte_mbuf *m;
    unsigned i, j, portid, nb_rx;
    struct pcap_pkthdr header;
    qconf = this->_lcore_queue;

    // 检查绑定的所有队列的信息
    if (qconf->n_rx_port == 0) {
        LOG_D(FMT("lcore %u has nothing to do\n", lcore_id));
        return;
    }

    for (i = 0; i < qconf->n_rx_port; i++) {
        portid = qconf->rx_port_list[i];
        LOG_D(FMT(" -- lcoreid=%u portid=%u\n", lcore_id, portid));
    }

    while(1) {

        while(this->pause)  // 程序启动时首先检查
            this->is_pause = true;

        if(this->_force_quit) break;

        /*
         * Read packet from RX queues
         */
        for (i = 0; i < qconf->n_rx_port; i++) {

            portid = qconf->rx_port_list[i];
            nb_rx = rte_eth_rx_burst(portid, 0,
                         pkts_burst, MAX_PKT_BURST);

            // 目前不增加统计功能
            // port_statistics[portid].rx += nb_rx;
            // printf("portid: %d, nb_rx: %d\n", portid, nb_rx);

            for (j = 0; j < nb_rx; j++) {
                m = pkts_burst[j];
                printf("%d:%d %d\n", m->port, m->pkt_len, m->nb_segs);
                //l2fwd_simple_forward(m, portid);
                u_char* data = rte_pktmbuf_mtod(m, u_char *);


                // u_int i;
                // for (i=0; (i < m->pkt_len) ; i++)
                // {
                //     // Start printing on the next after every 16 octets
                //     if ( (i % 16) == 0) printf("\n");
                //     // Print each octet as hex (x),
                //     // make sure there is always two characters (.2).
                //     printf("%.2x ", pkt[i]);
                // }
                // printf("\n");

                header.len = m->pkt_len;
                header.caplen = m->pkt_len;
                gettimeofday(&header.ts, NULL);

                shared_ptr<PARSE_PKT> pkt = _pkt_generater(&header, data);
                if(pkt) {
                    run_counter(pkt);
                    _push_to_queue(pkt);
                }

                rte_pktmbuf_free(m);
            }
        }
    }

    this->is_pause = true;
    LOG_I("DPDK read over\n");
}

void Reader::_inner_pcap_read_and_push() {
    struct pcap_pkthdr *header;

    const u_char *data;
    LOG_I("_inner_pacp_read_and_push\n");

    u_int packetCount = 0;
    int returnValue;

    while (returnValue = pcap_next_ex(this->_pcap, &header, &data) >= 0) {

        while(this->pause)  // 程序启动时首先检查
            this->is_pause = true;

        if(this->_force_quit) // 强制退出
            break;

        if (header->len != header->caplen)
           LOG_W(FMT("Capture size different than packet size: %ld bytes\n",
                     header->len));

        shared_ptr<PARSE_PKT> pkt = _pkt_generater(header, data);
        if(pkt) {   // 非GRE数据包, 默认丢弃
            run_counter(pkt);
            _push_to_queue(pkt);
        }
    }
    this->is_pause = true;
    LOG_I("PCAP read over\n");
}

void Reader::run_counter(shared_ptr<PARSE_PKT> pkt) {
    LOG_D("Run counter \n");
    for(auto& item : *this->_counter_map) {
        const CounterRule& c_rule = item.first;
        shared_ptr<Counter>& cnt = item.second;

        // 如果规则不是缺省的, 那么只要违反一条规则, 马上进行对下一个规则的判断
        if(c_rule.ip_src.s_addr != 0
                && c_rule.ip_src.s_addr != pkt->ip_inner->ip_src.s_addr)
            continue;


        if(c_rule.ip_dst.s_addr != 0
                && c_rule.ip_dst.s_addr != pkt->ip_inner->ip_dst.s_addr)
            continue;

        LOG_D("GET IP " << inet_ntoa(pkt->ip_inner->ip_src) << "\n");
        LOG_D("C_RULE " << c_rule.protocol << "\n");
        if(c_rule.protocol != -1 &&
                c_rule.protocol != pkt->ip_inner->ip_p)
            continue;

        // TODO: 补全交换机ID信息, 目前的数据报文中还没有交换机ID信息
        // if(c_rule.switch_id != -1 )

        LOG_D("Rule been satisfy\n");
        cnt->add_one();
    }
}

shared_ptr<PARSE_PKT> Reader::_pkt_generater(
            const struct pcap_pkthdr* header,
            const u_char *packet) {
    /**
     * 这里进行了malloc操作, 但是malloc之后, 马上将其交给了智能指针中的成员,
     * 完全可以保证内存不泄露. 但是为了性能, 这里最好使用对象池来请求.
     * 我很懒, 希望未来有人可以做. 有关对象池的使用, 请查看:
     *      https://stackoverflow.com/a/27837534/5563477
     * 主要原理是使用shared_ptr, 将析构函数重写, 使其析构时添加回池中.
     *
     * 由于总是动态的分配释放内存会有较大的损失, 程序中的数据包总是
     * 保持在一个固定的数量中, 这里是一个很适合的场景
     *   http://gameprogrammingpatterns.com/object-pool.html
     *
     * TODO: 不用对象池真的很慢
     */
    shared_ptr<PARSE_PKT> pkt;

    try {
        pkt = shared_ptr<PARSE_PKT>(new PARSE_PKT);
        pkt->_data = new u_char[header->len];
    } catch (std::bad_alloc&) {
        LOG_E("malloc error\n");
        return NULL;
    }

    // 因为header中为数字型的成员, 直接赋值即可.
    pkt->header = *header;   // copy ts and len, they are nums, = is alright.

    memcpy(pkt->_data, packet, pkt->header.len);
    if( pkt_init(pkt) != 0)
        return NULL;

    return pkt;
}

void Reader::_push_to_queue(shared_ptr<PARSE_PKT> pkt) {
    int idx = hash_func(pkt) % (*_queue_vec).size();
    (*_queue_vec)[idx]->push(pkt);  // 不理解的话, 请查阅_queue_vec的结构
}

static int hash_func(shared_ptr<PARSE_PKT> pkt) {
    int ret = 0;
    if(pkt->tcp == NULL) // 非TCP数据包, 均返回0
        return ret;

    struct sniff_ip* ip = pkt->ip_inner;    // 使用内层的IP数据包
    struct sniff_tcp* tcp = pkt->tcp;

    ret =
        ((size_t) ip->ip_src.s_addr * 59 ) ^
        ((size_t) ip->ip_dst.s_addr) ^
        ((size_t) tcp->th_sport) ^
        ((size_t) tcp->th_dport) ^
        ((size_t) tcp->th_seq);
    return ret;
}


static int pkt_init(shared_ptr<PARSE_PKT> p) {
    p->eth_outer = (struct sniff_ethernet*)(p->_data);
    p->ip_outer = (struct sniff_ip*)(p->_data + SIZE_ETHERNET);

    int size_ip_outer = 0;
    int size_gre = 0;
    int size_ip_inner = 0;

    size_ip_outer = (IP_HL(p->ip_outer))*4;

    switch(p->ip_outer->ip_p) {
        case IPPROTO_GRE:
            LOG_D("   Protocol: GRE\n");
            break;
        default:
            LOG_W("   Protocol: unknown\n");
            return -1;
    }
    p->gre = (struct sniff_std_gre*)(p->_data + SIZE_ETHERNET + size_ip_outer);
    size_gre = 1 * 4;

    if(GRE_TYPE(p->gre) != ETH_P_ERSPAN) {  /** 如果GRE数据包中不是ERSPAN格式, 则退出 */
        LOG_E(FMT("   Protocol: %X\n", GRE_TYPE(p->gre)));
        LOG_E    ("   Protocol: unknown\n");
        return 0;
    }

    if(GRE_C(p->gre))   // 如果有C位, Checksum 和 Reserved1 都存在
        size_gre += 1 * 4;

    if(GRE_K(p->gre))
        size_gre += 1 * 4;

    if(GRE_S(p->gre))
        size_gre += 1 * 4;


    p->eth_inner = (struct sniff_ethernet*)
                        (p->_data + SIZE_ETHERNET + size_ip_outer + size_gre);

    p->ip_inner = (struct sniff_ip*)
                        (p->_data + SIZE_ETHERNET * 2 + size_ip_outer + size_gre);

    size_ip_inner = IP_HL(p->ip_inner) * 4;

    switch(p->ip_inner->ip_p) {
        case IPPROTO_TCP:
            LOG_D("   Protocol: TCP\n");
            break;
        case IPPROTO_ICMP:
            LOG_D("   Protocol: ICMP\n");
            return 0;
        default:
            LOG_E(FMT("   Protocol: %2X \n", p->ip_inner->ip_p));
            LOG_E    ("   Protocol: unknown\n");
            return 0;
    }
    p->tcp = (struct sniff_tcp*)
        (p->_data + SIZE_ETHERNET * 2 + size_ip_outer + size_gre + size_ip_inner);

    LOG_D("Parse Over\n");
    return 0;
}

