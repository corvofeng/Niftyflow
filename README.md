## Niftyflow


本程序为商业程序, 如果简单的部署使用, 可以随意.
如果需要技术支持, 将按照部署的服务器数量和咨询时间收费.
需要新的功能请单独咨询.

## 文件结构

```
.
├── doc         # 项目中的文档: 交互接口以及环境配置
├── analyzer    # 分析器
├── controler   # 控制器
├── frontend    # 前端页面
└── README.md

4 directories, 1 files
```

## 使用apidoc生成文档

> 使用方式参见[apidoc](http://apidocjs.com)

```
> npm install apidoc -g
> apidoc -i ./controler/ -o ./doc/apidoc # 生成的文档保存在doc/apidoc中
```

所有文档采用`Markdown`编写, 使用`pandoc`转换为`tex`文件后编译, 模板会在之后附上.

## 致谢

- 感谢提出`Everflow`的微软团队
- 感谢各位老师的指导
- 感谢实验室师兄师姐的配合
- 感谢许多开源工具GCC, GDB, CMake, VIM, NodeJS...

## LICENSE

Copyright © 2016 corvo.

This project is licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
