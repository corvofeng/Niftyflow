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

## 程序结构

![程序结构图][8]

## 有关DPDK的使用

DPDK的使用, 可以帮助我们快速获得数据包. 但是也有一点不好, 需要专用的硬件支持,
而且DPDK程序的编译也很麻烦.

  作者手头上没有可以使用的大型服务器进行测试, 只是简单的利用`VirtualBox`搭建了
测试环境, 理论上`DPDK`的使用能够将接受效率提高8~10倍. 希望以后接手其他工作时,
能有机器来进行性能测试吧.

  DPDK测试时, 请在testpmd成功的情况下, 使用`sudo ./test/DPDKTest`.

  DPDK的环境配置, 请查看[测试环境搭建][10], 使用示例请查看[DPDK相关文档][9], 

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

## 性能测试(未使用DPDK)

  本程序的性能测试, 我将测试单台PC上的分析器性能, 分为如下几个部分:

服务器配置如下:

| Type| value|
| --- | --- |
| CPU |  Intel(R) Xeon(R) CPU E5-2690 v4 @ 2.60GHz|
| 内存 | 32G |
| 监听的网卡为lo | 采用tcpreplay指定测试数据以及发送速率.|



目前程序中有一个线程用于读取, 另外8个线程进行数据处理.

```bash
# 评测命令
sudo tcpreplay -K -p 3000000 -l 0 -i lo out.pcap   # 指定参数3000000pps
sudo ./bin/analyzer | tee  xx.log               # 使用tee同时输出到文件

# 检查日志
cat xx.log | grep -v 'Read a new' | grep -e  'packet' -e  'Now we process' | head -n 10

# 获取每隔20s的队列数据信息
cat xx.log | grep  'watch' | grep 'After 20s' | awk '{print $9}' > queue_20s.log

# 获取所有线程的处理信息, 每隔1s进行打印
cat xx.log |grep -v 'Read a new' |\
            grep 'Now we process' |\
            awk '{printf("%d %d\n"), $4, $8}' > process_1s.log

# 生成echarts图表数据, 可供js程序进行读取.
cat queue_20s.log  | awk '{printf("%d, "), $0}'            > queue_format.log
cat process_1s.log | awk '{printf("[%d, %d],\n"), $1, $2}' > process_format.log
```

首先, 我是在INFO模式下运行的, 即日志输出为INFO的状态, 以下两张为具体的运行
状态.

第一张为全部队列中的数据包变化情况, 第二张为所有的processor处理的数据包情况.

![队列处理情况][4]
![每个线程处理情况][5]

```bash
PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
14402 root      20   0 1455148  13672   2720 S 993.0  0.0  13:02.91 analyzer

25798 root      20   0 2842400 1.335g  10764 S 711.0  4.3  71:54.03 analyzer
```

通过观察第一张图片, 可以看得出, 使用lo进行数据接收时, 队列中的数据量根本不会上升.
这也就说明了一点, 当前程序的瓶颈是在`pcap_open_live`阶段. 这里我只测试了pacp库,
或许改用`dpdk`能达到更好的效果. 如果未来有硬件, 我会考虑测试的.

通过观察第二张图片, 所有process线程的处理速度基本是稳定的, 处理速度上的不同,
更多的是hash函数选择的问题. 某些线程被分到了比较多的数据包. 也可以看的出, 当前
的处理速率大概是880000pps, 也就是平均每个线程有10万pps. 这里并不是说单个线程的
处理只有10万pps, 只能说明, 我们每秒到达的数据包只有这么多.

对于top观察得到的结果, 当前的程序对CPU的利用率较高, 但是对内存利用率偏低,
归根到底, 我们队列的元素不够多, 导致内存占用率不够.


### 程序目前的瓶颈分析

  目前主要是网卡的读取速率不够, 造成了程序性能的损失. 而后, 我使用3个网卡进行
数据读入. 发现只要生产速度提高, processor达到百万pps是很容易的. 这里请看下图.

![多网卡读入][7]

在125s - 130s为处理峰值, 能达到400万pps这样的吞吐量,
平均每个processor能够有50万pps的处理能力.


[1]: https://stackoverflow.com/questions/5875318/is-there-anyway-to-pass-arguments-to-a-signal-handler
[2]: http://www.cplusplus.com/forum/unices/66284/
[3]: https://gist.github.com/corvofeng
[4]: ../doc/img/队列情况-faster.png
[5]: ../doc/img/每个线程处理情况-faster.png
[6]: ../doc/img/队列情况-faster-trpile.png
[7]: ../doc/img/每个线程处理情况-faster-triple.png
[8]: ../doc/img/analyzer_structure.png
[9]: ../doc/orig_md/DPDK使用说明.md
[10]: ../doc/orig_md/测试环境搭建.md