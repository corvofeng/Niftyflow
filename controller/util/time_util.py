#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : November 24, 2017
@Author : corvo

 时间相关的函数

vim: set ts=4 sw=4 tw=99 et:
"""

import time
import datetime

time_format = '%Y-%m-%d %H:%M:%S'


def time_print(t):
    """将时间戳 => 字符串"""
    return time.strftime(time_format, time.localtime(t))


def time_to_day(t):
    """ 时间戳 => 某一天"""
    return time.strftime('%Y%m%d', time.localtime(t))


def time_to_month(t):
    """时间戳 => 月份"""
    return time.strftime('%Y%m', time.localtime(t))


def time_reformat(time_str):
    """ 字符串 => 字符串 有助于防止SQL注入 """
    timeArray = time.strptime(time_str, time_format)
    unix_stamp = int(time.mktime(timeArray))

    return time.strftime(time_format, time.localtime(unix_stamp))


def to_time_stamp(time_str):
    """字符串 => unix事件戳
     为了逻辑简单, 程序中只使用time_format形式进行输入
    """
    timeArray = time.strptime(time_str, time_format)
    return int(time.mktime(timeArray))


def date_to_stamp(d_time):
    """datetime => unix时间戳
    主要用于数据库中取出的时间的解析
    """
    return time.mktime(d_time.timetuple())
