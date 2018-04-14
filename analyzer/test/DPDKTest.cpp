#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#include <rte_common.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
// #include <rte_per_lcore.h>
// #include <rte_branch_prediction.h>
//#include <rte_interrupts.h>
// #include <rte_random.h>
//#include <rte_debug.h>
// #include <rte_prefetch.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
// #include <rte_mempool.h>
// #include <rte_mbuf.h>

#ifdef __cplusplus /* 如果采用了C++，如下代码使用C编译器 */
    extern "C" { /* 如果没有采用C++，顺序预编译 */
#endif


static volatile bool force_quit;
#define MAX_PKT_BURST 32


#define MAX_RX_QUEUE_PER_LCORE 16
#define MAX_TX_QUEUE_PER_PORT 16

// 每个逻辑核可以监听多个队列
struct lcore_queue_conf {
    unsigned n_rx_port;                             /** 监听的队列数 */
    unsigned rx_port_list[MAX_RX_QUEUE_PER_LCORE];  /** 需要监听的队列id */
} __rte_cache_aligned;
struct lcore_queue_conf lcore_queue_conf[RTE_MAX_LCORE];

/* Per-port statistics struct */
struct l2fwd_port_statistics {
    uint64_t tx;
    uint64_t rx;
    uint64_t dropped;
} __rte_cache_aligned;

// 统计端口中到来的数据包数量.
struct l2fwd_port_statistics port_statistics[RTE_MAX_ETHPORTS];

/* Check the link status of all ports in up to 9s, and print them finally */
static void
check_all_ports_link_status(uint16_t port_num, uint32_t port_mask)
{
#define CHECK_INTERVAL 100 /* 100ms */
#define MAX_CHECK_TIME 90 /* 9s (90 * 100ms) in total */
    uint16_t portid;
    uint8_t count, all_ports_up, print_flag = 0;
    struct rte_eth_link link;

    printf("\nChecking link status");
    fflush(stdout);
    for (count = 0; count <= MAX_CHECK_TIME; count++) {
        if (force_quit)
            return;
        all_ports_up = 1;
        for (portid = 0; portid < port_num; portid++) {
            if (force_quit)
                return;
            if ((port_mask & (1 << portid)) == 0)
                continue;
            memset(&link, 0, sizeof(link));
            rte_eth_link_get_nowait(portid, &link);
            /* print link status if flag set */
            if (print_flag == 1) {
                if (link.link_status)
                    printf(
                            "Port%d Link Up. Speed %u Mbps - %s\n",
                            portid, link.link_speed,
                            (link.link_duplex == ETH_LINK_FULL_DUPLEX) ?
                            ("full-duplex") : ("half-duplex\n"));
                else
                    printf("Port %d Link Down\n", portid);
                continue;
            }
            /* clear all_ports_up flag if any link down */
            if (link.link_status == ETH_LINK_DOWN) {
                all_ports_up = 0;
                break;
            }
        }
        /* after finally printing all link status, get out */
        if (print_flag == 1)
            break;

        if (all_ports_up == 0) {
            printf(".");
            fflush(stdout);
            rte_delay_ms(CHECK_INTERVAL);
        }

        /* set the print_flag if all ports up or timeout */
        if (all_ports_up == 1 || count == (MAX_CHECK_TIME - 1)) {
            print_flag = 1;
            printf("done\n");
        }
    }
}
/* main processing loop */
static void
l2fwd_main_loop(void)
{
    struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
    struct rte_mbuf *m;
    unsigned lcore_id;
    unsigned i, j, portid, nb_rx;
    struct lcore_queue_conf *qconf;

    lcore_id = rte_lcore_id();
    qconf = &lcore_queue_conf[lcore_id];

    if (qconf->n_rx_port == 0) {
        printf("lcore %u has nothing to do\n", lcore_id);
        return;
    }

    printf("entering main loop on lcore %u\n", lcore_id);

    for (i = 0; i < qconf->n_rx_port; i++) {
        portid = qconf->rx_port_list[i];
        printf(" -- lcoreid=%u portid=%u\n", lcore_id, portid);

    }

    while (!force_quit) {
        /*
         * Read packet from RX queues
         */
        for (i = 0; i < qconf->n_rx_port; i++) {

            portid = qconf->rx_port_list[i];
            nb_rx = rte_eth_rx_burst(portid, 0,
                         pkts_burst, MAX_PKT_BURST);

            port_statistics[portid].rx += nb_rx;
            // printf("portid: %d, nb_rx: %d\n", portid, nb_rx);
            for (j = 0; j < nb_rx; j++) {
                m = pkts_burst[j];
                //  rte_prefetch0(rte_pktmbuf_mtod(m, void *));
                printf("%d:%d %d\n", m->port, m->pkt_len, m->nb_segs);
                //l2fwd_simple_forward(m, portid);
                unsigned char* pkt = rte_pktmbuf_mtod(m, unsigned char *);

                u_int i;
                for (i=0; (i < m->pkt_len) ; i++)
                {
                    // Start printing on the next after every 16 octets
                    if ( (i % 16) == 0) printf("\n");

                    // Print each octet as hex (x),
                    // make sure there is always two characters (.2).
                    printf("%.2x ", pkt[i]);
                }

                printf("\n");

                rte_pktmbuf_free(m);
            }
        }
    }
}


static int
l2fwd_launch_one_lcore(__attribute__((unused)) void *dummy)
{
    l2fwd_main_loop();
    return 0;
}

static void
signal_handler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM) {
        printf("\n\nSignal %d received, preparing to exit...\n",
                signum);
        force_quit = true;
    }
}

// Invalid In C++
// static struct rte_eth_conf port_conf = {
//     .rxmode = {
//         .split_hdr_size = 0,
//         .ignore_offload_bitfield = 1,
//         .offloads = DEV_RX_OFFLOAD_CRC_STRIP
//     }
//     .txmode = {
//         .mq_mode = ETH_MQ_TX_NONE,
//     }
// };

static struct rte_eth_conf port_conf;

#define MAX_PKT_BURST 32
#define BURST_TX_DRAIN_US 100 /* TX drain every ~100us */
#define MEMPOOL_CACHE_SIZE 256



#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024


struct rte_mempool * l2fwd_pktmbuf_pool = NULL;


/*
 * Initialises a given port using global settings and with the rx buffers
 * coming from the mbuf_pool passed as parameter
 */
static inline int
port_init(uint16_t port, struct rte_mempool *mbuf_pool)
{
        struct rte_eth_conf _port_conf = port_conf;
        const uint16_t rxRings = 1, txRings = rte_lcore_count() - 1;
        int retval;
        uint16_t q;
        uint16_t nb_rxd = RX_RING_SIZE;
        uint16_t nb_txd = TX_RING_SIZE;
        struct rte_eth_dev_info dev_info;

        struct rte_eth_rxconf rx_conf;
        struct rte_eth_txconf txconf;

        if (port >= rte_eth_dev_count())
                return -1;
        rte_eth_dev_info_get(port, &dev_info);
        if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
                _port_conf.txmode.offloads |=
                        DEV_TX_OFFLOAD_MBUF_FAST_FREE;
        retval = rte_eth_dev_configure(port, rxRings, txRings, &_port_conf);
        if (retval != 0)
                return retval;
        retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
        if (retval != 0)
                return retval;

        rx_conf = dev_info.default_rxconf;
        rx_conf.offloads = _port_conf.rxmode.offloads;

        retval = rte_eth_rx_queue_setup(port, 0, nb_rxd,
                                rte_eth_dev_socket_id(port),
                                                &rx_conf, mbuf_pool);
        if (retval < 0)
            return retval;

        txconf = dev_info.default_txconf;
        txconf.txq_flags = ETH_TXQ_FLAGS_IGNORE;
        txconf.offloads = _port_conf.txmode.offloads;
        for (q = 0; q < txRings; q++) {
                retval = rte_eth_tx_queue_setup(port, q, nb_txd,
                                                rte_eth_dev_socket_id(port),
                                                &txconf);
                if (retval < 0)
                        return retval;
        }
        retval = rte_eth_dev_start(port);
        if (retval < 0)
                return retval;
        struct rte_eth_link link;
        rte_eth_link_get_nowait(port, &link);
        while (!link.link_status) {
                printf("Waiting for Link up on port %" PRIu16 "\n", port);
                sleep(1);
                rte_eth_link_get_nowait(port, &link);
        }
        if (!link.link_status) {
                printf("Link down on port %" PRIu16 "\n", port);
                return 0;
        }
        struct ether_addr addr;
        rte_eth_macaddr_get(port, &addr);
        printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
                        " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
                        port,
                        addr.addr_bytes[0], addr.addr_bytes[1],
                        addr.addr_bytes[2], addr.addr_bytes[3],
                        addr.addr_bytes[4], addr.addr_bytes[5]);
        rte_eth_promiscuous_enable(port);
        return 0;
}
#define NUM_MBUFS ((64*1024)-1)


int main(int argc, char** argv) {

    port_conf.rxmode.split_hdr_size = 0;
    port_conf.rxmode.ignore_offload_bitfield = 1;
    port_conf.rxmode.offloads = DEV_RX_OFFLOAD_CRC_STRIP;
    port_conf.txmode.mq_mode = ETH_MQ_TX_NONE;

    struct lcore_queue_conf *qconf;
    uint16_t nb_ports;

    uint16_t portid;
    uint16_t lcore_cnt = 2;
    int ret;
    unsigned lcore_id, rx_lcore_id;
    unsigned int nb_lcores = 1;

    unsigned int l2fwd_rx_queue_per_lcore = 3;   // 每个核最多只允许监听三个队列
    uint32_t l2fwd_enabled_port_mask = 0x1; // 只启动1号网卡

    ret = rte_eal_init(argc, argv);

    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if(lcore_cnt > RTE_MAX_LCORE) {
        printf("Can't bigger than %d\n", RTE_MAX_LCORE);
        return -1;
    }

    nb_ports = rte_eth_dev_count();
    if (nb_ports == 0)
        rte_exit(EXIT_FAILURE, "No Ethernet ports - bye\n");

    /* create the mbuf pool */
    l2fwd_pktmbuf_pool = rte_pktmbuf_pool_create("mbuf_pool",
            nb_ports * NUM_MBUFS,
            MEMPOOL_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE,
            rte_socket_id());

    if (l2fwd_pktmbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "Cannot init mbuf pool\n");

    rx_lcore_id = 0;
    /* Initialize the port/queue configuration of each logical core */
    for (portid = 0; portid < nb_ports; portid++) {
        /* skip ports that are not enabled */
        if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
            continue;

        /* get the lcore_id for this port */
        while (rte_lcore_is_enabled(rx_lcore_id) == 0 ||
                lcore_queue_conf[rx_lcore_id].n_rx_port ==
                l2fwd_rx_queue_per_lcore) {  // 超过每个核可以监听的最大队列数
            rx_lcore_id++;
            if (rx_lcore_id >= RTE_MAX_LCORE)
                rte_exit(EXIT_FAILURE, "Not enough cores\n");
        }

        if (qconf != &lcore_queue_conf[rx_lcore_id]) {
            /* Assigned a new logical core in the loop above. */
            qconf = &lcore_queue_conf[rx_lcore_id];
            nb_lcores++;
        }

        qconf->rx_port_list[qconf->n_rx_port] = portid;
        qconf->n_rx_port++;
        printf("Lcore %u: RX port %u\n", rx_lcore_id, portid);
    }

    /* Initialise each port */
    for (portid = 0; portid < nb_ports; portid++) {
        /* skip ports that are not enabled */
        if ((l2fwd_enabled_port_mask & (1 << portid)) == 0) {
                printf("\nSkipping disabled port %d\n", portid);
                continue;
        }
        if (port_init(portid, l2fwd_pktmbuf_pool) != 0)
                rte_exit(EXIT_FAILURE, "Cannot initialize port %u\n",
                                portid);
        /* init port */
        printf("Initializing port %u... done\n", portid);
    }

    check_all_ports_link_status(nb_ports, l2fwd_enabled_port_mask);

    rte_eal_mp_remote_launch(l2fwd_launch_one_lcore, NULL, CALL_MASTER);

    RTE_LCORE_FOREACH_SLAVE(lcore_id) {
        if (rte_eal_wait_lcore(lcore_id) < 0) {
            ret = -1;
            break;
        }
    }

    for (portid = 0; portid < nb_ports; portid++) {
        if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
            continue;
        printf("Closing port %d...", portid);
        rte_eth_dev_stop(portid);
        rte_eth_dev_close(portid);
        printf(" Done\n");
    }
    printf("Bye...\n");

    return ret;
}

/* 采用C编译器编译的C语言代码段 */
#ifdef __cplusplus /* 结束使用C编译器 */
    }
#endif


