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
        msg = yield tornado.gen.Task(c.blpop, 'ever_queue')
        parse_request(msg)


def parse_request(msg):
    """解析客户端来的消息
    """
    print(msg)


def deal_alert(msg):
    """处理预警
    """
    pass


def deal_request(msg):
    """处理请求, 主要是分析器初始化时的请求
    """
    pass


