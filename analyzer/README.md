# Analyzer

> 项目使用CMake进行构建, 使用到以下几个库
> * MySQL    : MySQL 连接 sudo apt-get install libmysqlclient-dev
> * HiResdis : Redis 连接 sudo apt-get install libhiredis-dev
> * LibPacp  : 解析pcap文件 sudo apt-get install libpcap-dev
> * cJSON    : JSON的读写 随项目附带
> * Libevent : 仅在测试时使用 sudo apt-get install libevent-dev

程序虽为C++写成, 但其中并没有包含任何的继承关系. 仅利用C++中的一些简单特性,
是`C With Class`.

本程序中用到一些代码片段[gist][3], 可以直接运行. 有关详细的开发文档, 其实大家看
代码基本就很全了, 另外, 分析器的架构请查看doc文件夹.

## 项目编译

编译前, 首先要安装上面几个库, 我只在Linux上测试通过, 不保证window可用. 建议使用
QtCreater打开, 它对CMake支持良好, 尤其是cmake到了3.8之后, 可以使用cmake server.

```bash
> export DPDK_DIR=/home/corvo/dpdk/x86_64-native-linuxapp-gcc # 指定DPDK的目录
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

如果你能看懂`test/CMakeLists.txt`, 那么请直接添加. 其中, 测试用例文件以List的方式进行组织.

以`MySQLTest`为例, 使用`MySQLTest.cpp`编译成为`./test/MySQLTest`
而后建立一条测试`MySQLTest_tester`.

除了使用`make test`之外, 也可以直接执行`./test/MySQLTest`.


## 有关DPDK的使用

  对于分析器程序来说, 网卡输入的速度, 极有可能成为瓶颈. 如果使用`libpcap`进行抓包,
整个过程需要Linux网络协议栈的支持, 需要从内核空间进行一次拷贝到用户空间. 而这些
GRE数据包中的内容, 我们根本不需要Linux系统来关心, 只是想要更快的得到更多的数据包.
DPDK的使用, 可以帮助我们快速获得数据包. 但是也有一点不好, 需要专用的硬件支持,
而且DPDK程序的编译也很麻烦.

  作者手头上没有可以使用的大型服务器进行测试, 只是简单的利用`VirtualBox`搭建了
测试环境, 理论上`DPDK`的使用能够将接受效率提高8~10倍. 希望以后接手其他工作时,
能有机器来进行性能测试吧.

  DPDK测试时, 请在testpmd成功的情况下, 使用`sudo ./test/DPDKTest`.

  DPDK的环境配置, 请查看整个项目`doc`文件夹中的相关文档.


## 数据库问题

  分析器中的数据库trace数据连接采用**长连接**的方式, 因为它对写入数据库很频繁,
短连接会影响性能. 而定时的规则书写采用**短连接**的形式, 10~20s才会写入一次.

**Attention**: 每个写入线程单独持有一个连接对象, 假如所有线程公用一个连接对象,
可能会造成数据库之间的竞争, 而且文档中也未提及mysql的连接是线程安全的. 我们也
不能做这样的假设.


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
DELIMITER $$
CREATE TRIGGER `fdate_set` BEFORE INSERT ON `tbl_trace_data`
FOR EACH ROW BEGIN
  SET NEW.fdate = CAST( DATE_FORMAT(NOW(),'%Y%m%d') AS UNSIGNED);
END$$
DELIMITER ;

DELIMITER $$
CREATE TRIGGER `fdate_set_counter` BEFORE INSERT ON  `tbl_counter` 
FOR EACH
ROW BEGIN 
SET NEW.fdate = CAST( DATE_FORMAT( NOW( ) ,  '%Y%m%d' ) AS UNSIGNED ) ;
END$$
DELIMITER ;

```

### Counter计数器表格的设计问题

目前的分析器中, 想要定时10s进行发送计数器信息, 所以一天的数据大约有
$$
\frac{24 \times 3600 s}{10s} = 8640(条)
$$

如果我们有3台交换机的话, 10个计数规则的话, 每天将会有250,000条记录,
每个月大约有7,500,000条.

MySQL主要支持百万级别的数据, 因此, 在正式投入使用时,
整个程序需要使用分区表(按月分区). 如果有可能我们将粒度设置为20s是不是可以呢?

我建议数据保存时间最好不要超过1个月, 有了问题需要尽快解决. 


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


## 与控制器交互

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

## 定时器与signal

在当前的程序中, 我不打算再添加一个线程来做定时上传计数器的功能. 这样的轮询
太过单调了, 而且定时上传功能可能只需要10s, 或是20s才上传一次, 单独开线程显得
很浪费.

最终, 我决定使用Linux中的定时器功能, 使用程序定时发送`SIGALRM`信号, 这样, 我们
将信号处理函数完善即可. 但是, 这样做会造成一个问题, 信号处理函数只接受一个参数
我们需要使用全局变量才可以在信号函数中访问.

关于能不能传递其他参数, 我也找了许多的相关材料[1][1], [2][2], 以失败而告终,
最后只得放弃, 开始着手全局变量的规划.

## 程序中的单例模式

程序中在几个位置使用了单例模式:

1. Logger
2. Conf
3. Watcher
4. EverflowMain

这几个单例全部在main函数中初始化, 这个时候没有创建任何线程,
不存在需要加锁来保证唯一性.


## 程序中内存泄露问题

  程序中主要使用`valgrind`进行内存问题检测, `valgrind`的功能远不止此, 如果未来
需要进行缓存优化, 也可以利用`valgrind`进行操作.

  在未使用DPDK时, 程序中的内存泄露问题已经完全解决. 但是之后针对DPDK程序进行
内存泄露检测时, `valgrind`需要打补丁进行编译, 所以DPDK程序的内存安全性不能保证.
没错, 我有点懒.

  其实我也想为自己辩解一下, `DPDK`只是作为入口使用. 并且`DPDK`所有的数据包会
经过一轮拷贝, 拷贝过之后, 立刻进行原数据包的释放. 这样, `DPDK`完全不会有
内存泄露.

  其实, `DPDK`数据包不进行释放也是可以的, 这样可以省一步拷贝的操作. 由于程序中
使用了智能指针, 可以在`trace`数据构造完成后进行释放. 是不是很美好.

  醒醒吧少年, 你很可能会忘记释放的. 如果想要减少这一次拷贝操作, 那么,
请一定要为`valgrind`打补丁编译后进行充分的测试. 图便宜也是要付出代价的.


[1]: https://stackoverflow.com/questions/5875318/is-there-anyway-to-pass-arguments-to-a-signal-handler
[2]: http://www.cplusplus.com/forum/unices/66284/
[3]: https://gist.github.com/corvofeng

