#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Date   : March 28, 2018
@Author : corvo

vim: set ts=4 sw=4 tw=99 et:
"""

import tornadoredis

redis_client_queue = tornadoredis.Client()
redis_client_queue.connect()

redis_client_pubsub = tornadoredis.Client()
redis_client_pubsub.connect()
