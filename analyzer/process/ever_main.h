/*
 *=======================================================================
 *    Filename:ever_main.h
 *
 *     Version: 1.0
 *  Created on: March 18, 2018
 *
 *      Author: corvo
 *
 *  此文件中记录了主要的处理程序逻辑
 *=======================================================================
 */
#ifndef EVER_MAIN_H_5UBIZCWM
#define EVER_MAIN_H_5UBIZCWM

#include <vector>
#include <memory>
#include <pthread.h>
#include "con_queue.h"
#include "on_process.h"
#include "packet.h"
#include "reader.h"

using std::vector;
using std::shared_ptr;

/**
 * 2018-03-23: 达到最后处理阶段, 目前想做的是在慢路径中进行数据库写入以及Redis
 *              写入.
 *
 */
class EverflowMain
{
public:

    EverflowMain ();
    void run();
    virtual ~EverflowMain ();

private:
    int processer_cnt;    /**< 记录同时处理的processor个数 */
    int reader_cnt;       /**< 记录同时处理的reader个数 */

    vector<pcap_t*> pcap_vec;
    vector<shared_ptr<Reader>> reader_vec;

    vector<shared_ptr<Processer>> processer_vec; /**< 多个分析器进行 */
    vector<shared_ptr<PKT_QUEUE>> queue_vec; /**< 每个分析器绑定一个队列 */
};


#endif /* end of include guard: EVER_MAIN_H_5UBIZCWM */
