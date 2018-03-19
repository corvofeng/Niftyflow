#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 18, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:
"""

import tornadoredis
import tornado.gen


# redis客户端 LPUSH q aaa  即可
@tornado.gen.engine
def q_listen():
    c = tornadoredis.Client()
    c.connect()
    while True:
        d = yield tornado.gen.Task(c.blpop, 'q')
        print(d)

