# Analyzer

> 项目使用CMake进行构建, 使用到以下几个库
> * MySQL    : MySQL 连接
> * HiResdis : Redis 连接
> * LibPacp  : 解析pcap文件
> *.Protobuf : trace数据并不规则, 将其序列化为Protobuf进行保存, 便于控制器读取.


## 项目编译

编译前, 首先要安装上面几个库, 我只在Linux上测试通过, 不保证window可用. 建议使用
QtCreater打开, 它对CMake支持良好, 尤其是cmake到了3.8之后, 可以使用cmake server.

```bash
> mkdir build
> cd build
> cmake ..
> make -j4  # 尽量多线程, 我的组织有问题, 编译其实还是挺慢的
> ./bin/analyer # 程序放在bin目录中
```


## 单元测试

目前的单元测试只是简单的 测试程序的完善 与 库的使用是否正常.


## 单元测试的其他使用

直接运行main, 很难知道程序如何进行, 简单写几个测试跑一下, 你会发现程序并不难理解.

如果你能看懂`test/CMakeLists.txt`, 那么请直接添加, 否则, 请尽快联系作者, 毕业前
提供支持. 毕业后不保证及时提供支持.

测试用例文件以List的方式进行组织,

以`MySQLTest`为例, 使用`MySQLTest.cpp`编译成为`./test/MySQLTest`
而后建立一条测试`MySQLTest_tester`.

除了使用`make test`之外, 也可以直接执行`./test/MySQLTest`.


## 数据库问题

### 数据表结构



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
