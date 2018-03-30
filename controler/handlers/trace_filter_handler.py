#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 17, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:

 异常处理, 保证不抛掷


2018-03-30: 开始编写处理逻辑
"""

from tornado.log import access_log
from handlers.base_handler import BaseHandler
from util.time_util import *


QUERY_SQL = "SELECT * FROM tbl_trace_data WHERE 1"

class TraceFilterHandler(BaseHandler):

    def post(self):
        """
        这里是与用户交互的部分, 我们一定要做输入检验, 防止被注入
        其中, 程序里必须要求时间参数, 起止时间均需要, 其他参数为可选参数
        is_loop, is_drop, is_probe 只允许0,1作为输入, 或者不输入
        """
        start_time = self.get_argument('start_time', None)
        end_time = self.get_argument('end_time', None)
        ip_src = self.get_argument('ip_src', '0.0.0.0')
        ip_dst = self.get_argument('ip_dst', '0.0.0.0')
        protocol = self.get_argument('protocol', -1)
        is_loop = self.get_argument('is_loop', -1)
        is_drop = self.get_argument('is_drop', -1)
        is_probe = self.get_argument('is_probe', -1)

        try:    # IP检验
            import socket
            socket.inet_aton(ip_src)
            socket.inet_aton(ip_dst)
        except OSError as e:
            access_log.error('Get wrong ip {}, {}'.format(ip_src, ip_dst))
            self.write_json(None, status_code=400, msg='参数错误')
            return
        except TypeError as e: # 这是为None的情况
            pass

        try: # 时间检验
            start_time = to_time_stamp(start_time)
            end_time = to_time_stamp(end_time)
        except (ValueError, TypeError) as e:
            access_log.error('Get err time {}, {}'.format(start_time, end_time))
            self.write_json(None, status_code=400, msg='参数错误')
            return

        if start_time > end_time:   # 起止时间一定要有大小
            access_log.warning('Get err time {}, {}'.format(start_time, end_time))
            self.write_json(None, status_code=400, msg='参数错误')
            return


        try: # 试图进行转换
            is_probe = int(is_probe)
            is_drop = int(is_drop)
            is_loop = int(is_loop)
            protocol = int(protocol)
        except (ValueError, TypeError) as e:
            access_log.error('Get Err {}, {}, {}, {}' \
                              .format(is_probe, is_drop, is_loop, protocol))
            self.write_json(None, status_code=400, msg='参数错误')
            return

        last_sql = QUERY_SQL
        day1 = int(time_to_day(start_time))
        day2 = int(time_to_day(end_time))

        # 数据库中时间戳作为索引, 所以首先加入时间戳判断
        last_sql +=  ' AND fdate BETWEEN {} AND {}'.format(day1, day2)


        # 以下的各种判断均排除缺省值
        if ip_src != '0.0.0.0':
            last_sql += ' AND s_ip = {}'.format(ip_src)
        if ip_dst != '0.0.0.0':
            last_sql += ' AND d_ip = {}'.format(ip_dst)

        if is_loop != -1:
            last_sql += ' AND is_loop = {}'.format(is_loop)
        if is_drop != -1:
            last_sql += ' AND is_drop = {}'.format(is_drop)
        if is_probe != -1:
            last_sql += ' AND is_probe = {}'.format(is_probe)

        access_log.debug(last_sql)

        rlts = []
        try:
            cur = self.db.cursor()
            cur.execute(last_sql)
            rlts = cur.fetchall()
        finally:
            cur.close()

        ret_rlts = []
        day1_ms = (start_time % (24 * 3600)) * 1000
        day2_ms = (end_time   % (24 * 3600)) * 1000

        for r in rlts:   # 简单过滤不符合时间的数据列
            if r['fdate'] == day1 and r['generate_time'] < day1_ms:
                continue
            if r['fdate'] == day2 and r['generate_time'] > day2_ms:
                continue

            ret_rlts.append(r)

        self.write_json(rlts)

