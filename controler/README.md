# 控制器相关

**Attention**: 整个程序是在单线程中运行的, 请不要执行过于耗时的操作, 尽量使
                程序的瓶颈在数据库访问上.

## 启动过程

```bash
> pip install -r requirement.txt  # 安装依赖
> redis-server      # 启动redis服务端
> python main.py    # 启动程序
```
