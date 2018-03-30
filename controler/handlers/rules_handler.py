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
        access_log.info('Get an access')
        self.write_json('hello')
