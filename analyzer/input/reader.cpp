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

using namespace std;

/**
 * 2018-03-26: 在解析数据包完成后, 立刻进行计数器累加操作
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

/**
 * @brief 根据p->_data构建完整的数据包p, static表示仅在该文件中使用, 下面两个
 *        并不想提供给别人使用, 仅仅作为Reader功能的一部分, 单独来维护.
 */
static void pkt_init(shared_ptr<PARSE_PKT> p);

/**
 * https://stackoverflow.com/questions/3215232/hash-function-for-src-dest-ip-port
 * @brief 根据数据包的元素进行hash操作, 并没有什么特别的尝试.
 *          目前对于非TCP数据包, 直接返回0.
 */
static int hash_func(shared_ptr<PARSE_PKT> pkt);

void Reader::_inner_read_and_push() {
    struct pcap_pkthdr *header;

    const u_char *data;
    LOG_D("_inner_read_and_push\n");

    u_int packetCount = 0;
    while (int returnValue = pcap_next_ex(this->_pcap, &header, &data) >= 0) {
        while(this->pause)  // 程序启动时首先检查
            this->is_pause = true;

        if (header->len != header->caplen)
           LOG_W(FMT("Warning! Capture size different than packet size: %ld bytes\n",
                     header->len));

        shared_ptr<PARSE_PKT> pkt = _pkt_generater(header, data);
        if(pkt) {
            run_counter(pkt);
            _push_to_queue(pkt);
        }
    }
}

void Reader::run_counter(shared_ptr<PARSE_PKT> pkt) {
    for(auto& item : *this->_counter_map) {
        const CounterRule& c_rule = item.first;
        shared_ptr<Counter> cnt = item.second;

        // 只要违反一条规则, 马上进行下一个判断
        if(c_rule.ip_src.s_addr != 0 && c_rule.ip_src.s_addr != pkt->ip_inner->ip_src.s_addr)
            continue;
        if(c_rule.ip_dst.s_addr != 0 && c_rule.ip_dst.s_addr != pkt->ip_inner->ip_dst.s_addr)
            continue;
        if(c_rule.protocol != -1 &&
                c_rule.protocol != pkt->ip_inner->ip_p)
            continue;
        // if(c_rule.switch_id != -1 ) TODO: 补全交换机ID信息
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
    pkt_init(pkt);

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


static void pkt_init(shared_ptr<PARSE_PKT> p) {
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
            LOG_E("   Protocol: unknown\n");
            return;
    }
    p->gre = (struct sniff_std_gre*)(p->_data + SIZE_ETHERNET + size_ip_outer);
    size_gre = 1 * 4;

    if(GRE_TYPE(p->gre) != ETH_P_ERSPAN) {  /** 如果GRE数据包中不是ERSPAN格式, 则退出 */
        LOG_E(FMT("   Protocol: %X\n", GRE_TYPE(p->gre)));
        LOG_E    ("   Protocol: unknown\n");
        return ;
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
            return;
        default:
            LOG_E(FMT("   Protocol: %2X \n", p->ip_inner->ip_p));
            LOG_E    ("   Protocol: unknown\n");
            return;
    }
    p->tcp = (struct sniff_tcp*)
        (p->_data + SIZE_ETHERNET * 2 + size_ip_outer + size_gre + size_ip_inner);

    LOG_D("Parse Over\n");
    return ;
}

/**
 * https://www.rhyous.com/2011/11/13/how-to-read-a-pcap-file-from-wireshark-with-c/
 * How to read a packet capture file.
 */
int pcap_read()
{
    LOG_I("pcap read\n");
    /*
     * Step 2 - Get a file name
     */

    string file = "/home/corvo/Dropbox/课程文件/毕业设计/grecap.cap";

    /*
     * Step 3 - Create an char array to hold the error.
     */

    // Note: errbuf in pcap_open functions is assumed to be able to hold at least PCAP_ERRBUF_SIZE chars
    //       PCAP_ERRBUF_SIZE is defined as 256.
    // http://www.winpcap.org/docs/docs_40_2/html/group__wpcap__def.html
    char errbuff[PCAP_ERRBUF_SIZE];

    /*
     * Step 4 - Open the file and store result in pointer to pcap_t
     */

    // Use pcap_open_offline
    // http://www.winpcap.org/docs/docs_41b5/html/group__wpcapfunc.html#g91078168a13de8848df2b7b83d1f5b69
    pcap_t * pcap = pcap_open_offline(file.c_str(), errbuff);

    /*
     * Step 5 - Create a header and a data object
     */

    // Create a header object:
    // http://www.winpcap.org/docs/docs_40_2/html/structpcap__pkthdr.html
    struct pcap_pkthdr *header;

    // Create a character array using a u_char
    // u_char is defined here:
    // typedef unsigned char   u_char;
    const u_char *data;

    /*
     * Step 6 - Loop through packets and print them to screen
     */
    u_int packetCount = 0;
    while (int returnValue = pcap_next_ex(pcap, &header, &data) >= 0)
    {
        // Print using printf. See printf reference:
        // http://www.cplusplus.com/reference/clibrary/cstdio/printf/

        // Show the packet number
        printf("Packet # %i\n", ++packetCount);

        // Show the size in bytes of the packet
        printf("Packet size: %d bytes\n", header->len);

        // Show a warning if the length captured is different
        if (header->len != header->caplen)
            printf("Warning! Capture size different than packet size: %ld bytes\n", header->len);

        // Show Epoch Time
        printf("Epoch Time: %d:%d seconds\n", header->ts.tv_sec, header->ts.tv_usec);

        //parse_ETH_PKT(data, header->caplen);
        // got_packet(NULL, header, data);

        // loop through the packet and print it as hexidecimal representations of octets
        // We also have a function that does this similarly below: PrintData()
        for (u_int i=0; (i < header->caplen ) ; i++)
        {
            // Start printing on the next after every 16 octets
            if ( (i % 16) == 0) printf("\n");

            // Print each octet as hex (x), make sure there is always two characters (.2).
            printf("%.2x ", data[i]);
        }

        // Add two lines between packets
        printf("\n\n");
        break;
    }
    pcap_close(pcap);
}



