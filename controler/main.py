#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 17, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:
"""


import settings
import tornado.ioloop
import tornado.web
from  tornado import web
from tornado.log import app_log
from handlers.base_handler import My404Handler
from tornado.options import define, options
from handlers.test_handler import TeshHandler
from handlers.trace_filter_handler import TraceFilterHandler
from handlers.counter_filter_handler import CounterFilterHandler
from handlers.probe_construct_handler import ProbeConstructHandler
from handlers.rules_handler import RulesHandler

from q_listen import q_listen


class Application(tornado.web.Application):
    def __init__(self):
        handlers = [
            (r'/v1/test', TeshHandler, dict(with_db=True)),
            (r'/v1/rules', RulesHandler, dict(with_db=True)),
            (r'/v1/tracce_filter', TraceFilterHandler, dict(with_db=True)),
            (r'/v1/counter_filter', CounterFilterHandler, dict(with_db=True)),
            (r'/v1/probe_construct', ProbeConstructHandler, dict(with_db=False)),
            (r"/api/(.*)", web.StaticFileHandler, {"path":"../doc/apidoc", "default_filename": "index.html"}),
        ]
        settings = dict(
            debug=options.debug,
            autoreload=options.autoreload,
            default_handler_class=My404Handler,
        )

        super(Application, self).__init__(handlers, **settings)


def main():
    app = Application()
    app.listen(options.port)

    # 监听redis队列中的预警
    q_listen()

    # app_log.debug('debug')
    # app_log.warning('warn')
    # app_log.error('error')
    app_log.info('App is listening: {}'.format(options.port))
    tornado.ioloop.IOLoop.current().start()


if __name__ == '__main__':
    main()
