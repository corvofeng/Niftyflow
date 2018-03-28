#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 18, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:
"""

import tornado.gen
from tornado.log import app_log
from conn import redis_client_queue
from conn import redis_client_pubsub


# redis客户端 LPUSH q aaa  即可
@tornado.gen.engine
def q_listen():
    while True:
        msg = yield tornado.gen.Task(redis_client_queue.blpop, 'ever_queue')
        parse_request(msg)


def parse_request(msg):
    """解析客户端来的消息
    """
    app_log.debug(msg)
    generate_sub(msg)


def deal_alert(msg):
    """处理预警
    """
    pass


def deal_request(msg):
    """处理请求, 主要是分析器初始化时的请求
    """
    pass


def generate_sub(msg):
    app_log.debug("Ret msg %s" % msg)
    redis_client_pubsub.publish("ever_chanel", msg)
