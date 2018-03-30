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
from util.time_util import time_reformat


QUERY_SQL = "SELECT * FROM tbl_trace_data WHERE 1"

class TraceFilterHandler(BaseHandler):

    def post(self):
        """
        这里是与用户交互的部分, 我们一定要做输入检验, 防止被注入
        其中, 程序里必须要求时间参数, 起止时间均需要, 其他参数为可选参数
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
            start_time = time_reformat(start_time)
            end_time = time_reformat(end_time)
        except ValueError as e:
            access_log.error('Get err time {}, {}'.format(start_time, end_time))
            self.write_json(None, status_code=400, msg='参数错误')
            return
        except TypeError as e:
            access_log.error('Get err time {}, {}'.format(start_time, end_time))
            self.write_json(None, status_code=400, msg='参数错误')
            return


        try: # 试图进行转换
            is_probe = int(is_probe)
            is_drop = int(is_drop)
            is_loop = int(is_loop)
            protocol = int(protocol)
        except ValueError as e:
            access_log.error('Get Err {}, {}, {}, {}' \
                              .format(is_probe, is_drop, is_loop, protocol))
            self.write_json(None, status_code=400, msg='参数错误')
            return
        except TypeError as e:
            access_log.error('Get Err {}, {}, {}, {}' \
                              .format(is_probe, is_drop, is_loop, protocol))
            self.write_json(None, status_code=400, msg='参数错误')
            return


        print(is_probe)
        import IPython
        IPython.embed()


        try:
            cur = self.db.cursor()

        finally:
            cur.close()


        self.write_json('get ok')

