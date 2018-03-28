#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 28, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:
"""

"""
2018-03-28: 初始化建立配置文件, 这里将会初始化Redis连接, 但是, 这里并不初始化
            MySQL, MySQL作为短连接使用, 用完立刻关闭.
            程序的即时负载并不很大, 使用长连接会严重占用服务器资源. 每次请求到来
            连接即可

"""

import tornado
import pymysql
import tornadoredis
from tornado.options import define, options
from tornado.log import app_log

define('port', default=9999, help='run on the given port', type=int)
define('debug', default=False, help='debug or relase', type=bool)
define('autoreload', default=True, help='when modify auto reolad', type=bool)


define('redis_host', default='127.0.0.1', help='redis host')
define('redis_port', default=6379, help='redis port')
define('redis_pass', default=None, help='redis password')

define('redis_queue', default='ever_queue', help='redis listen queue')
define('redis_chanel', default='ever_chanel', help='redis publish chanel')

define('mysql_host', default='127.0.0.1', help='mysql host')
define('mysql_user', default='root', help='mysql user')
define('mysql_port', default=3306, help='mysql port')
define('mysql_pass', default='', help='mysql password')
define('mysql_char', default='utf8', help='mysql charset')
define('mysql_db', default='DCN_shot', help='mysql db')


tornado.options.parse_config_file('./server.conf')

def redis_connect(host='localhost', port=6379, password=None,):
    r_cli = tornadoredis.Client(host=host, port=port, password=password)
    r_cli.connect()
    return r_cli

redis_client_queue = redis_connect(options.redis_host, options.redis_port,
                options.redis_pass)

redis_client_pubsub = redis_connect(options.redis_host, options.redis_port,
                options.redis_pass)

def mysql_conn():
    db = pymysql.connect(host=options.mysql_host,
                      user=options.mysql_user,
                      password=options.mysql_pass,
                      db=options.mysql_db,
                      charset=options.mysql_char,
                      cursorclass=pymysql.cursors.DictCursor)
    return db

def mysql_close(db):
    db.close()

app_log.info('run settings')
