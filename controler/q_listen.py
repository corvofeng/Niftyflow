#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 18, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:
"""

import json
import tornado.gen
from tornado.log import app_log
from settings import redis_client_queue
from settings import redis_client_pubsub
from settings import mysql_conn
from settings import mysql_close
from settings import options


# redis客户端 LPUSH q aaa  即可
@tornado.gen.engine
def q_listen():
    while True:
        msg = yield tornado.gen.Task(redis_client_queue.blpop, options.redis_queue)
        parse_request(msg)


def parse_request(msg):
    """解析客户端来的消息
    """
    try:
        msg = json.loads(msg['ever_queue'])
        app_log.debug(msg)
        if msg.get('ACTION') == 'ALERT':
            deal_alert(msg)
        else:
            deal_request(msg)

    except json.decoder.JSONDecodeError as e:
        app_log.error("Get wrong msg {}".format(msg))


def deal_alert(msg):
    """处理预警
    """
    pass


def deal_request(msg):
    """处理请求, 主要是分析器初始化时的请求
    """
    ret_analyzer_id = msg.get('ANALYZER_ID')

    if msg.get('ACTION') == 'INIT':
        ret_msg = {}
        init_msg = on_init()
        init_msg['COMMAND'] = 'INIT'
        ret_msg['ANALYZER_ID'] = ret_analyzer_id
        ret_msg['MESSAGE'] = init_msg
        generate_sub(json.dumps(ret_msg))

    else:
        app_log.error("Unknow action")


def on_init():
    db = mysql_conn()

    ret_msg = {}

    ret_msg['COUNTER'] = get_counter_rules(db)
    ret_msg['SWH_ID'] = None   # TODO: 添加出口交换机的数组

    return ret_msg

def get_counter_rules(conn):
    """ 获取当前有效的rule信息
    """
    counter_rules = []
    try:
        with conn.cursor() as cursor:
            sql = "SELECT * FROM counter_rule WHERE is_valid = 1"
            cursor.execute(sql)
            rules = cursor.fetchall()
            print(rules)

            # [{'id': 1, 'rule_name': '""',
            #  'ip_src': '192.3.2.3', 'ip_dst': '',
            #  'protocol': -1, 'is_valid': 1}]
            for r in rules:
                rule_item = {}
                rule_item['CNT_ID'] = r['id']
                rule_item['SRC_IP'] = r['ip_src']
                rule_item['DST_IP'] = r['ip_dst']
                rule_item['SWH_ID'] = r['switch_id']
                rule_item['PTL'] = r['protocol']
                counter_rules.append(rule_item)
    except Exception as e:
        app_log.debug('Get rule err {}'.format(e))

    finally:
        mysql_close(db)


def generate_sub(msg):
    app_log.debug("Ret msg %s" % msg)
    redis_client_pubsub.publish(options.redis_chanel, msg)
