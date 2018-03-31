#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 17, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:

2018-03-31: 修正计数器规则请求时的bug.
"""

from util.time_util import *
from tornado.log import access_log
from handlers.base_handler import BaseHandler

QUERY_SQL = "SELECT * FROM tbl_counter WHERE 1"

class CounterFilterHandler(BaseHandler):

    def post(self):
        """ 对于程序来说, 必须提供时间以及rule_id
        """
        start_time = self.get_argument('start_time', None)
        end_time = self.get_argument('end_time', None)

        rule_id = self.get_argument('rule_id', 0)

        try: # 时间检验
            start_time = to_time_stamp(start_time)
            end_time = to_time_stamp(end_time)
        except (ValueError, TypeError) as e:
            access_log.error('Get err time {}, {}'.format(start_time, end_time))
            self.write_json(None, status_code=400, msg='参数错误')
            return
        except Exception as e:
            access_log.error('Get error {}'.format(e))

        try: # 试图进行转换
            rule_id = int(rule_id)
        except (ValueError, TypeError) as e:
            access_log.error('Get Err {}' \
                              .format(rule_id))
            self.write_json(None, status_code=400, msg='参数错误')
            return
        except Exception as e:
            access_log.error('Get error {}'.format(e))

        last_sql = QUERY_SQL
        day1 = int(time_to_day(start_time))
        day2 = int(time_to_day(end_time))

        last_sql +=  ' AND fdate BETWEEN {} AND {}'.format(day1, day2)
        last_sql +=  ' AND generate_time > \'{}\' AND generate_time < \'{}\'' \
                    .format(time_print(start_time), time_print(end_time))

        if rule_id != 0:
            last_sql += ' AND rule_id = {}'.format(rule_id)

        access_log.debug(last_sql)

        rlts = []
        try:
            cur = self.db.cursor()
            cur.execute(last_sql)
            rlts = cur.fetchall()
        finally:
            cur.close()

        for r in rlts:
            r['generate_time'] = time_print((date_to_stamp(r['generate_time'])))

        self.write_json(rlts)
