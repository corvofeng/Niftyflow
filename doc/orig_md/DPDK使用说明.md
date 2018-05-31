# 有关DPDK的使用介绍

## DPDK安装

请查看[测试环境搭建](./测试环境搭建.md)

[DPDK文档中所有实例][2]

本次设计参考了`Network Layer 2 Forwarding + variants`中的代码, 改写了其中的
接受数据包部分, 原始文档请查看[l2fwd][3]

## 程序中DPDK的使用位置

  在`analyzer/input/dpdk_adapter.cpp`中进行了DPDK的初始化操作:
* `dpdk_initer`中, 为每个核添加要监听的port
* `port_init`中, 将port与buf_pool绑定


在`analyzer/input/reader.cpp`中, 取得数据包并放置在对应的队列中:
*  `_inner_dpdk_read_and_push`, 取当前线程对应的核配置信息, 不断轮询,
    当前核对应的多个port中取得数据.

## 我所修改的示例程序


请查看[gist][1]的代码部分, 此程序可以直接编译运行(对DPDK版本无特殊要求), 截获数据包并打印.



[1]: https://gist.github.com/corvofeng/dd203ccd1f28130aef4ad41c80942b4e
[2]: https://software.intel.com/en-us/articles/introduction-to-the-dpdk-sample-applications
[3]:http://dpdk.org/doc/guides/sample_app_ug/l2_forward_real_virtual.html