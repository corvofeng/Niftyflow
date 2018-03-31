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
        """ 对规则的操作分为两种
                1: 添加规则, 需要提供规则所需的几个信息
                    规则名称, 源IP, 目的IP, 协议类型, 镜像时的交换机ID

                2: 删除规则, 提供rule_id即可
        """

        act = self.get_argument('act', '')
        rule_name = self.get_argument('rule_name', '')
        rule_id = self.get_argument('rule_id', -1)
        ip_src = self.get_argument('ip_src', '0.0.0.0')
        ip_dst = self.get_argument('ip_dst', '0.0.0.0')
        switch_id = self.get_argument('switch_id', -1)
        protocol = self.get_argument('protocol', -1)

        try:    # IP检验
            import socket
            socket.inet_aton(ip_src)
            socket.inet_aton(ip_dst)
        except OSError as e:
            access_log.error('Get wrong ip {}, {}'.format(ip_src, ip_dst))
            self.write_json(None, status_code=400, msg='参数错误')
            return
        except Exception as e:
            access_log.error('Get error {}'.format(e))

        try: # 试图进行转换
            rule_id = int(rule_id)
            switch_id = int(switch_id)
            protocol = int(protocol)
        except (ValueError, TypeError) as e:
            access_log.error('Get Err {} {} {}' \
                              .format(rule_id, switch_id, protocol))
            self.write_json(None, status_code=400, msg='参数错误')
            return
        except Exception as e:
            access_log.error('Get error {}'.format(e))


        if act == 'ADD':
            self.rule_add(rule_name, ip_src, ip_dst, switch_id, protocol)
        elif act == 'DEL':
            self.rule_del(rule_id)
        else:
            access_log.error('Not valid action: {}'.format(act))
            self.write_json(None, status_code=400, msg='参数错误')

        return


    def rule_add(self, rule_name, ip_src, ip_dst, switch_id, protocol):

        # rule_name = rule_name[:30] # 不允许超过30位
        import re
        if re.search('^\w*$', rule_name) and rule_name != '':
            pass
        else:
            access_log.error('Get wrong name {}'.format(rule_name))
            self.write_json(None, status_code=400,\
                    msg='rule_name 只能由字母数字下划线组成')

        insert_sql = """INSERT INTO `counter_rule`
            (`rule_name`, `ip_src`, `ip_dst`,
            `protocol`, `switch_id`, `is_valid`)
            VALUES ('{}', '{}', '{}', '{}', '{}', '1') """

        insert_sql = insert_sql.format(rule_name,ip_src, ip_dst, protocol, switch_id)
        access_log.debug(insert_sql)

        try:    # 首先插入数据库
            cur = self.db.cursor()
            cur.execute(insert_sql)
            self.db.commit()
        except Exception as e:
            access_log.error('Get error {}'.format(e))
            self.write_json(None, status_code=400, msg='插入错误')
            return
        finally:
            cur.close()


        from q_listen import get_counter_rules, generate_sub
        query_sql = """SELECT * FROM counter_rule where rule_name = '{}' """
        query_sql = query_sql.format(rule_name)
        access_log.debug(query_sql)

        rule_item = {}
        try:    # 查找刚刚的记录
            cur = self.db.cursor()
            cur.execute(query_sql.format(rule_name))
            r = cur.fetchone()
            rule_item['CNT_ID'] = r['id']
            rule_item['SRC_IP'] = r['ip_src']
            rule_item['DST_IP'] = r['ip_dst']
            rule_item['SWH_ID'] = r['switch_id']
            rule_item['PTL'] = r['protocol']

        except Exception as e:
            access_log.error('Get error {}'.format(e))
            self.write_json(None, status_code=400, msg='查找错误')
            return 
        finally:
            cur.close()

        pub_msg = {}
        pub_msg['ANALYZER_ID'] = 0
        msg = {}
        msg['COMMOND'] = 'ADD_RULE'
        msg['COUNTER'] = [rule_item]
        pub_msg['MESSAGE'] = msg
        generate_sub(pub_msg)

        self.write_json('success')

    def rule_del(self, rule_id):
        """ 删除操作并不是真的删除, 只是进行置位
            {
              "ANALYZER_ID": 0,         // ID为0, 表示这是一种广播操作.
              "MESSAGE": {              // 报文主体
                "COMMOND": "ADD_RULE",  // 重载过程, 这里就认为是热重启
                "SWH_ID": [
                    14, 23, 19, 40
                ],
                "COUNTER": [           // 计数器的filter
                    {
                        "CNT_ID" : 1
                        "SRC_IP": "192.118.0.2",
                        "DST_IP": "192.119.0.1"
                    }
                ]
              }
            }
        """
        if rule_id == -1:
            access_log.error('Get wrong id, del failed')

        del_sql = """UPDATE `counter_rule`
                SET `is_valid` = '0'
                WHERE `counter_rule`.`id` = {}"""

        del_sql = del_sql.format(rule_id)
        access_log.debug(del_sql)

        try:
            cur = self.db.cursor()
            cur.execute(del_sql)
            self.db.commit()
        except Exception as e:
            access_log.error('Get error {}'.format(e))
            self.write_json(None, status_code=400, msg='删除错误')
            return
        finally:
            cur.close()

        from q_listen import get_counter_rules, generate_sub
        pub_msg = {}
        pub_msg['ANALYZER_ID'] = 0
        msg = {}
        msg['COMMOND'] = 'DEL_RULE'
        msg['COUNTER'] = [rule_id]
        pub_msg['MESSAGE'] = msg
        generate_sub(pub_msg)

        self.write_json('success')


"""
@api {get} /rules 请求所有规则
@apiVersion 0.0.1
@apiName All rules
@apiGroup Query

@apiDescription 查询所有的规则, 所有的rule在初始化时就应该被获取到, 而后通过
rule_id查找counter数据.

@apiSuccessExample {json} Success-Response:
HTTP/1.1 200 OK
{
    "code": 200,
    "msg": "success.",
    "data": [
        {
            "id": 1,
            "rule_name": "flow_from_192_3_2_3",
            "ip_src": "192.3.2.3",
            "ip_dst": "0",
            "protocol": -1,
            "switch_id": 0,
            "is_valid": 0
        }
    ]
}
"""

"""

@api {post} /rules 添加删除规则
@apiVersion 0.0.1
@apiName Rules add or del
@apiGroup Modify
@apiDescription 对于操作者来说, 添加删除规则只是一个请求而已, 但是对于整个系统
来说, 添加删除代表的是所有的分析器要暂停一遍, 才能将规则更新.


@apiParam {String} act  只能使用 'ADD|DEL'
@apiParam {Number} rule_id     在DEL操作时必须填写
@apiParam {String} rule_name   在ADD操作时必须填写
@apiParam {String} [ip_src='0.0.0.0']
@apiParam {String} [ip_dst='0.0.0.0']
@apiParam {Number} [protocol=-1]    报文中的协议类型
@apiParam {Number} [switch_id=-1]   镜像报文的交换机ID

@apiSuccessExample {json} Success-Response:
HTTP/1.1 200 OK

{
    "code": 200,
    "msg": "success.",
    "data": "success"
}
"""



