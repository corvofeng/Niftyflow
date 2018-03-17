#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 17, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:
"""

import json
import tornado

class BaseHandler(tornado.web.RequestHandler):
    def post(self):
        pass

    def write_json(self, data, status_code=200, msg='success.'):
        self.set_header('Content-Type', 'application/json')
        self.finish(json.dumps({
            'code': status_code,
            'msg': msg,
            'data': data
        }, ensure_ascii=False, indent=2))

