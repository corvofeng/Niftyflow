#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 17, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:
"""


import tornado.ioloop
import tornado.web
import tornado.log
import logging as SLOG
from tornado.options import define, options
from handlers.test_handler import TeshHandler
from q_listen import q_listen

define('port', default=9999, help='run on the given port', type=int)
define('debug', default=False, help="debug or relase", type=bool)
define('autoreload', default=True, help="when modify auto reolad", type=bool)


class Application(tornado.web.Application):
    def __init__(self):
        handlers = [
            (r"/v1/test", TeshHandler),
        ]
        settings = dict(
            debug=options.debug,
            autoreload=options.autoreload
        )

        super(Application, self).__init__(handlers, **settings)


def main():
    app = Application()
    app.listen(options.port)

    # 监听redis队列中的预警
    q_listen()

    SLOG.info("App is listening: %d" % options.port)
    tornado.ioloop.IOLoop.current().start()


if __name__ == "__main__":
    tornado.options.parse_config_file('./server.conf')
    main()
