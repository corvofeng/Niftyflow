#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 17, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:
"""

from handlers.base_handler import BaseHandler


class TeshHandler(BaseHandler):
    def post(self):
        self.write_json('hello')
