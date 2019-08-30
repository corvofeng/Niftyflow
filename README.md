## Niftyflow

原论文见此: https://www.microsoft.com/en-us/research/wp-content/uploads/2016/08/everflow-sigcomm15.pdf

本程序为商业程序, 如果简单的部署使用, 可以随意.
如果需要技术支持, 将按照部署的服务器数量和咨询时间收费.
需要新的功能请单独咨询.

  2018-6-2, 答辩结束, 我将文档补充完善, 未来有任何问题, 欢迎联系我
`corvofeng#gmail.com`

## 文件结构

```
.
├── doc         # 项目中的文档: 交互接口以及环境配置
├── analyzer    # 分析器
├── controller   # 控制器
├── frontend    # 前端页面
└── README.md

4 directories, 1 files
```

工作流程如下图所示
![数据流][3]

## 文档生成

### 使用apidoc生成接口文档

> 使用方式参见[apidoc](http://apidocjs.com)

```
> npm install apidoc -g
> apidoc -i ./controller/ -o ./doc/apidoc # 生成的文档保存在doc/apidoc中
```

### 使用pandoc生成项目文档

所有项目文档采用`Markdown`编写, 使用`pandoc`转换为`tex`文件后编译, 模板会在之后附上.


## 持续集成

为了之后开发以及部署的方便, 不出意外将采用`Travis CI`进行持续集成. 持续集成
最主要的是针对分析器进行, 如果控制器增加单元测试之后, 也会加入进来.


## 致谢

- 感谢提出`Everflow`的微软团队
- 感谢各位老师的指导
- 感谢实验室师兄师姐的配合
- 在程序写作时, 也看到了DPDK之父患癌症去世的消息, 希望生活中的大家好好珍惜现在.
- 感谢许多开源工具GCC, GDB, CMake, VIM, NodeJS...


## LICENSE

Copyright © 2016 corvo.

This project is licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

[3]: ./doc/img/user_flow.png
