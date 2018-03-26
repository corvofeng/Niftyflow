/*
 *=======================================================================
 *    Filename:watcher.h
 *
 *     Version: 1.0
 *  Created on: March 26, 2018
 *
 *      Author: corvo
 *=======================================================================
 */

#ifndef WATCHER_H_2RGQCSXG
#define WATCHER_H_2RGQCSXG

#include "atom_counter.h"
#include "cnt_rule.h"
#include <map>

using std::map;

class Watcher
{
public:

    watcher:_counter_map(NULL) ();
    ~watcher ();
    void bind_counter_map(map<CounterRule, Counter>* counter_map) {
        this->_counter_map = counter_map;
    }
    pthread_t t_pubsub_chanel;
    pthread_t t_push_queue;

    void _inner_pubsub() {}
    void _inner_push();

private:
    map<CounterRule, Counter>* _counter_map;

    static void* pubsub_process(void *context) {
       Watcher *w = (Watcher*)context;
        w->_inner_pubsub();
    }



};

#endif /* end of include guard: WATCHER_H_2RGQCSXG */
