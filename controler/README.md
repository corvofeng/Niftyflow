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


## tornado 学习

这是一个单线程的服务器框架, 在Linux上使用epoll来实现高并发.
入门可以查看[Tornado服务器的结构][1]


[1]: http://www.tornadoweb.org/en/stable/guide/structure.html
