#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 17, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:
"""

import json
import tornado
from tornado.log import app_log
from settings import mysql_conn
from settings import mysql_close

class BaseHandler(tornado.web.RequestHandler):

    def write_error(self, status_code, **kwargs):
        if status_code == 404:
            self.render("404.html")
        else:
            self.render("error.html")

    def set_default_headers(self):
        """ 目前允许跨域访问
        """
        app_log.info("setting headers!!!")
        self.set_header("Access-Control-Allow-Origin", "*") # 这个地方可以写域名
        self.set_header("Access-Control-Allow-Headers", "x-requested-with")
        self.set_header('Access-Control-Allow-Methods', 'POST, GET, OPTIONS')

    def initialize(self, with_db=False):
        """ 初始化
            @parm with_db 置位表示此Handler需要访问数据库
        """
        self.db = None
        if with_db:
            app_log.debug('connect db')
            self.db = mysql_conn()

    def post(self):
        pass

    def write_json(self, data, status_code=200, msg='success.'):
        self.set_header('Content-Type', 'application/json')
        self.finish(json.dumps({
            'code': status_code,
            'msg': msg,
            'data': data
        }, ensure_ascii=False, indent=2))

    def on_finish(self):
        if self.db:
            app_log.debug('free db')
            mysql_close(self.db)

class My404Handler(BaseHandler):
    def prepare(self):
        raise tornado.web.HTTPError(404)
