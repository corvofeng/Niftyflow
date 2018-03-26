/*
 *=======================================================================
 *    Filename:message.h
 *
 *     Version: 1.0
 *  Created on: March 26, 2018
 *
 *      Author: corvo
 *=======================================================================
 */

#ifndef MESSAGE_H_5FVTS0J4
#define MESSAGE_H_5FVTS0J4

#include <string>

/**
 * @brief 向Redis队列中发送的信息, 本来想要自己写类来实现,
 * 后来想到使用std::string即可, 但为了阅读性, 我还是封装一层好了.
 */
struct Message {
    std::string msg;
};
#endif /* end of include guard: MESSAGE_H_5FVTS0J4 */
