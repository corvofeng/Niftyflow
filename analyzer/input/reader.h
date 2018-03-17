#include <pcap.h>




int pcap_read();

/**
 * @brief GRE 解除封装
 */
void gre_decap(u_char* data);

