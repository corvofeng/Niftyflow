#include "dpdk_adapter.h"
#include "conf.h"

/**
 * 初始化每个给定的端口, 这个函数从一开始就很容易出错, 如果启动不成功,
 * 十之八九要进行调试, 没什么更好的办法, 多打断点, 多调试吧
 * Initialises a given port using global settings and with the rx buffers
 * coming from the mbuf_pool passed as parameter
 */
static inline int
port_init(uint16_t port, struct rte_mempool *mbuf_pool)
{
#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

    struct rte_eth_conf _port_conf = {0};
    _port_conf.rxmode.split_hdr_size = 0;
    _port_conf.rxmode.ignore_offload_bitfield = 1;
    _port_conf.rxmode.offloads = DEV_RX_OFFLOAD_CRC_STRIP;
    _port_conf.txmode.mq_mode = ETH_MQ_TX_NONE;
    _port_conf.txmode.offloads = 0;

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
    if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE) {
        _port_conf.txmode.offloads |=
                    DEV_TX_OFFLOAD_MBUF_FAST_FREE;
        LOG_D("IN offloads modify\n");
    }

    LOG_D(FMT("dev_configure port:%d rx:%d tx:%d\n", port, rxRings, txRings));
    retval = rte_eth_dev_configure(port, rxRings, txRings, &_port_conf);

    LOG_D("dev configure " << retval << " \n");
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
        LOG_D(FMT("Waiting for Link up on %d\n",port));
        sleep(1);
        rte_eth_link_get_nowait(port, &link);
    }

    if (!link.link_status) {
        LOG_D(FMT("Waiting for Link up on %d\n",port));
        return 0;
    }

    struct ether_addr addr;
    rte_eth_macaddr_get(port, &addr);
    LOG_D(FMT("Port %u MAC: %02d" " %02d" " %02d"
                    " %02d" " %02d" " %02d\n",
                    port,
                    addr.addr_bytes[0], addr.addr_bytes[1],
                    addr.addr_bytes[2], addr.addr_bytes[3],
                    addr.addr_bytes[4], addr.addr_bytes[5]));
    rte_eth_promiscuous_enable(port);
    return 0;
}


void dpdk_initer(lcore_queue_conf* lcore_vec, Conf* conf) {

#define l2fwd_rx_queue_per_lcore 4  // 每个核允许监听的最大队列数
#define NUM_MBUFS ((64*1024)-1)
#define MEMPOOL_CACHE_SIZE 256

    static int cur_lcore = 0;   /**< 标识当前的lcore值  */
    static int rx_lcore_id = 0;

    uint16_t nb_ports;  /**< 总共有的端口数 */
    uint16_t portid;

    nb_ports = rte_eth_dev_count();
    if (nb_ports == 0) {
        LOG_E("No Ethernet ports - bye\n");
        exit(-1);
    }

    static struct rte_mempool *l2fwd_pktmbuf_pool
                          = rte_pktmbuf_pool_create("mbuf_pool",
                            nb_ports * NUM_MBUFS,
                            MEMPOOL_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE,
                            rte_socket_id());

    if (l2fwd_pktmbuf_pool == NULL) {
        LOG_E("Cannot init mbuf pool\n");
        exit(-1);
    }


    for (portid = cur_lcore; portid < nb_ports; portid++) {
        /* skip ports that are not enabled */
        if((conf->dpdk_port_mask & (1 << portid)) == 0)
            continue;

        if(lcore_vec->n_rx_port > l2fwd_rx_queue_per_lcore) // 已经超过最大数目
            break;

        while (rte_lcore_is_enabled(rx_lcore_id) == 0) {
            rx_lcore_id++;
            if (rx_lcore_id >= RTE_MAX_LCORE) {
                LOG_E("Not enough cores\n");
                exit(-1);
            }
        }


        if (port_init(portid, l2fwd_pktmbuf_pool) != 0) {
            LOG_E("Cannot initialize port "<< portid << "\n");
            exit(-1);
        }

        /* init port */
        LOG_I(FMT("Initializing port %u... done\n", portid));

        lcore_vec->rx_port_list[lcore_vec->n_rx_port] = portid;
        lcore_vec->n_rx_port ++;

        LOG_I(FMT("ADD RX port %u\n", portid));
    }
    cur_lcore = portid;
}

/* Check the link status of all ports in up to 9s, and print them finally */
void
check_all_ports_link_status(Conf *conf)
{
#define CHECK_INTERVAL 100 /* 100ms */
#define MAX_CHECK_TIME 90 /* 9s (90 * 100ms) in total */
    uint16_t portid;
    uint8_t count, all_ports_up, print_flag = 0;
    struct rte_eth_link link;

    uint16_t port_num = rte_eth_dev_count();
    if (port_num == 0) {
        LOG_E("No Ethernet ports - bye\n");
        exit(-1);
    }

    LOG_I("Checking link status\n");
    for (count = 0; count <= MAX_CHECK_TIME; count++) {
        all_ports_up = 1;
        for (portid = 0; portid < port_num; portid++) {
            if ((conf->dpdk_port_mask & (1 << portid)) == 0)
                continue;
            memset(&link, 0, sizeof(link));
            rte_eth_link_get_nowait(portid, &link);
            /* print link status if flag set */
            if (print_flag == 1) {
                if (link.link_status)
                    LOG_D( FMT(
                            "Port%d Link Up. Speed %u Mbps - %s\n",
                            portid, link.link_speed,
                            (link.link_duplex == ETH_LINK_FULL_DUPLEX) ?
                            ("full-duplex") : ("half-duplex\n")));
                else
                    LOG_D(FMT("Port %d Link Down\n", portid));
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
            LOG_I(".");
            rte_delay_ms(CHECK_INTERVAL);
        }

        /* set the print_flag if all ports up or timeout */
        if (all_ports_up == 1 || count == (MAX_CHECK_TIME - 1)) {
            print_flag = 1;
            LOG_D("Checking link done\n");
        }
    }
}

void close_all_port(Conf* conf)
{
    uint16_t nb_ports;  /**< 总共有的端口数 */
    uint16_t portid;

    nb_ports = rte_eth_dev_count();
    if (nb_ports == 0) {
        LOG_E("No Ethernet ports - bye\n");
        exit(-1);
    }

    for (portid = 0; portid < nb_ports; portid++) {
        if ((conf->dpdk_port_mask & (1 << portid)) == 0)
            continue;

        LOG_I(FMT("Closing port %d...\n", portid));
        rte_eth_dev_stop(portid);
        rte_eth_dev_close(portid);
        LOG_I("Done\n");
    }
    LOG_I("Bye...\n");
}


