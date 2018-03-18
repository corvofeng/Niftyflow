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
#include <pcap.h>

int pcap_read();

/**
 * @brief GRE 解除封装
 */
void gre_decap(u_char* data);

#endif /* end of include guard: READER_H_AKS58L6J */
