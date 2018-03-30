#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 17, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:
"""

from tornado.log import access_log
from handlers.base_handler import BaseHandler


class CounterFilterHandler(BaseHandler):
    def post(self):
        access_log.info('Get an access')
        self.write_json('hello')
