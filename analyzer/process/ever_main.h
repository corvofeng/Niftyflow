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
#include "ip.h"
#include "con_queue.h"
#include "on_process.h"

using std::vector;
using std::shared_ptr;

class EverflowMain
{
public:

    EverflowMain ();
    void run();
    virtual ~EverflowMain ();

private:
    int process_cnt;    /**< 记录同时处理的分析器个数 */

    vector<shared_ptr<Processer>> processer_vec; /**< 多个分析器进行 */
    vector<shared_ptr<Queue<IP_PKT>>> queue_vec; /**< 每个分析器绑定一个队列*/
};


#endif /* end of include guard: EVER_MAIN_H_5UBIZCWM */
