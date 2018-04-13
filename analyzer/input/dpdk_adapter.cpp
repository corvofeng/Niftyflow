#include "dpdk_adapter.h"


void dpdk_initer(lcore_queue_conf* lcore_vec, Conf* conf) {
    static int cur_lcore = 0;   /**< 标识当前的lcore值  */


}

/* Check the link status of all ports in up to 9s, and print them finally */

// static void
// check_all_ports_link_status(uint16_t port_num, uint32_t port_mask)
// {
// #define CHECK_INTERVAL 100 /* 100ms */
// #define MAX_CHECK_TIME 90 /* 9s (90 * 100ms) in total */
//     uint16_t portid;
//     uint8_t count, all_ports_up, print_flag = 0;
//     struct rte_eth_link link;
// 
//     printf("\nChecking link status");
//     fflush(stdout);
//     for (count = 0; count <= MAX_CHECK_TIME; count++) {
//         if (force_quit)
//             return;
//         all_ports_up = 1;
//         for (portid = 0; portid < port_num; portid++) {
//             if (force_quit)
//                 return;
//             if ((port_mask & (1 << portid)) == 0)
//                 continue;
//             memset(&link, 0, sizeof(link));
//             rte_eth_link_get_nowait(portid, &link);
//             /* print link status if flag set */
//             if (print_flag == 1) {
//                 if (link.link_status)
//                     printf(
//                             "Port%d Link Up. Speed %u Mbps - %s\n",
//                             portid, link.link_speed,
//                             (link.link_duplex == ETH_LINK_FULL_DUPLEX) ?
//                             ("full-duplex") : ("half-duplex\n"));
//                 else
//                     printf("Port %d Link Down\n", portid);
//                 continue;
//             }
//             /* clear all_ports_up flag if any link down */
//             if (link.link_status == ETH_LINK_DOWN) {
//                 all_ports_up = 0;
//                 break;
//             }
//         }
//         /* after finally printing all link status, get out */
//         if (print_flag == 1)
//             break;
// 
//         if (all_ports_up == 0) {
//             printf(".");
//             fflush(stdout);
//             rte_delay_ms(CHECK_INTERVAL);
//         }
// 
//         /* set the print_flag if all ports up or timeout */
//         if (all_ports_up == 1 || count == (MAX_CHECK_TIME - 1)) {
//             print_flag = 1;
//             printf("done\n");
//         }
//     }
// }


