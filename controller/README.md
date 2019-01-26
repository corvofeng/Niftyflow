# 控制器相关

**Attention**: 整个程序是在单线程中运行的, 请不要执行过于耗时的操作, 尽量使
                程序的瓶颈在数据库访问上.

## 启动过程

```bash
> cp server.conf-example server.conf # 创建配置文件
> pip install -r requirement.txt  # 安装依赖
> redis-server      # 启动redis服务端
> python main.py    # 启动程序
```

## 程序结构

![程序结构][3]

## API文档

* 有关前端的API请查看[README][4]中文档生成部分

* 与分析器相关的API接口, 请查看[Analyzer初始化及通信过程][5]

## tornado 学习

这是一个单线程的服务器框架, 在Linux上使用epoll来实现高并发.
入门可以查看[Tornado服务器的结构][1]

[PyMySQL的使用][2]

## 有关程序性能的问题

首先明确一点, 本程序是一个单线程的服务器程序, 所有用户均是在一个线程中进行轮转
如果某个用户请求时间过久, 将会严重影响其他用户的查询性能.

其中可以优化的点:
   1. 减少Python中字符串拼接的操作
   2. 如果查询频繁且要求较高, 可以改为长连接, 这里全部使用短连接查询


[1]: http://www.tornadoweb.org/en/stable/guide/structure.html
[2]: https://pypi.python.org/pypi/PyMySQL
[3]: ../doc/img/controller_arch.png
[4]: ../README.md
[5]: ../doc/orig_md/Analyzer初始化及通信过程.md
