#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 30, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:
"""

from tornado.log import access_log
from handlers.base_handler import BaseHandler


RULE_QUERY = 'SELECT * FROM counter_rule'
class RulesHandler(BaseHandler):

    def get(self):
        """ 查询所有的规则
        """
        rlts = []
        try:
            cur = self.db.cursor()
            cur.execute(RULE_QUERY)
            rlts = cur.fetchall()
        finally:
            cur.close()

        self.write_json(rlts)

    def post(self):

        ip_src = self.get_argument('ip_src', '0.0.0.0')
        ip_dst = self.get_argument('ip_dst', '0.0.0.0')

        try:    # IP检验
            import socket
            socket.inet_aton(ip_src)
            socket.inet_aton(ip_dst)
        except OSError as e:
            access_log.error('Get wrong ip {}, {}'.format(ip_src, ip_dst))
            self.write_json(None, status_code=400, msg='参数错误')
            return
        except Exception as e:
            access_log.error('Get error {}'.format(e))


        access_log.info('Get an access')
        self.write_json('hello')
