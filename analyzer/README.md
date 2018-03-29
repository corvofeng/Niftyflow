# Analyzer

> 项目使用CMake进行构建, 使用到以下几个库
> * MySQL    : MySQL 连接
> * HiResdis : Redis 连接
> * LibPacp  : 解析pcap文件
> * cJSON    : JSON的读写


## 项目编译

编译前, 首先要安装上面几个库, 我只在Linux上测试通过, 不保证window可用. 建议使用
QtCreater打开, 它对CMake支持良好, 尤其是cmake到了3.8之后, 可以使用cmake server.

```bash
> cp conf/conf-example.json conf/conf.json   # 添加配置文件
> mkdir build
> cd build
> cmake ..
> make -j4  # 尽量多线程, 我的组织有问题, 编译其实还是挺慢的
> ./bin/analyer # 程序放在bin目录中
```


## 单元测试

目前的单元测试只是简单的 测试程序的完善 与 库的使用是否正常.


### 单元测试的其他使用

直接运行main, 很难知道程序如何进行, 简单写几个测试跑一下, 你会发现程序并不难理解.

如果你能看懂`test/CMakeLists.txt`, 那么请直接添加, 否则, 请尽快联系作者, 毕业前
提供支持. 毕业后不保证及时提供支持.

测试用例文件以List的方式进行组织,

以`MySQLTest`为例, 使用`MySQLTest.cpp`编译成为`./test/MySQLTest`
而后建立一条测试`MySQLTest_tester`.

除了使用`make test`之外, 也可以直接执行`./test/MySQLTest`.


## 数据库问题

  分析器中的数据库连接采用**长连接**的方式, 因为它对写入数据库很频繁,短连接会
影响性能.

### 数据表结构

经过强烈的思想斗争, 我决定不使用protobuf, 直接使用json作为trace存储工具.

### Triger 设置

> 建立trigger, 由于Trace数据本身只携带了当日时间戳的偏移.
> 最终我决定在存入数据库时, 直接用数据库存入当天日期作为fdate.
>
>
> 如果出现了快到第二天0点时存入了数据, 而设置fdate时成为了第二天
>   例如: 2018-03-22 23:59:55 时准备存入数据库,
>     之后数据库中的fdate被设置为了2018-03-23, 这是一种不匹配,
>
> 有这么一种解决方式: 可以通过判断time_start的偏移, 如果偏移过大, 则认为它是
> 第一天的数据, 也就是03-22的数据. 下面只是列了最简单的Trigger设置.

```sql
CREATE TRIGGER `fdate_set` BEFORE INSERT ON `tbl_trace_data`
FOR EACH ROW BEGIN
  SET NEW.fdate = CAST( DATE_FORMAT(NOW(),'%Y%m%d') AS UNSIGNED);
END
```


### 程序的启动过程

  主要逻辑请查看`main.cpp`中的代码.

```
+--------------+
|  Init logger |
+------+-------+
       |
       v
+------+-------+
|  Init conf   |
+------+-------+
       |
       v
+------+-------+
| Watcher Init |
+------+-------+
       |
       v
+------+-------+
| Send  Init   |
+------+-------+
       |
       v
+------+-------+
| Get Command  |
|From Controler|
+------+-------+
       |
       v
+------+-------+
| Reader Start |
|    Reading   |
+------+-------+
```


##  与控制器交互

与控制器的交互分为两个部分, 发送和接受. 这两个部分均在watcher中编写.

**发送**: 我们向控制器监听的消息队列中发送消息

使用消息队列主要是因为可能有多个分析器, 队列是一个比较简单的方式.


**接受**: 从频道中读取, 如果是属于自己的命令信息, 则进行相应操作.

如果控制器采取一对多的连接方式, 其实是没多少必要的(经常进行交互的操作主要就是
增加删除规则, 增加删除出口交换机ID), 这样的操作基本是对所有分析器进行的. 使用
PubSub可以简化控制器的逻辑.

与控制器的交互部分全部采用`Redis`解耦, 其中使用了`Redis`的`消息队列`和`Pubsub`
两种接口. 两种接口的使用如下

**消息队列**: 由 分析器 => 控制器

```bash
> 127.0.0.1:6379> LPUSH foo 1234 # 分析器PUSH
> (integer) 1

> 127.0.0.1:6379> LPOP foo  # 控制器接受
> "1234"
```

**Pubsub**: 由控制器发送广播, 许多分析器监听相同的频道

```bash
> 127.0.0.1:6379> SUBSCRIBE foo        # 首先启动监听foo频道 
> Reading messages... (press Ctrl-C to quit)
> 1) "subscribe"
> 2) "foo"
> 3) (integer) 1
> 1) "message"
> 2) "foo"
> 3) "12334"

> 127.0.0.1:6379> PUBLISH foo 12334   # 控制器向foo频道中发送
> (integer) 1
```



