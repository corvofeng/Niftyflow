## 毕业设计


# 本次项目版本仅供实验室内部交流, 请不要针对这个版本索要任何形式的商业支持.


```
.
├── doc
├── analyzer
├── frontend
├── controler
└── README.md

2 directories, 3 files
```

## 使用apidoc生成文档

> 使用方式参见[apidoc](http://apidocjs.com)

```
> npm install apidoc -g
> apidoc -i ./controler/ -o ./doc/apidoc # 生成的文档保存在doc/apidoc中
```


所有文档采用`Markdown`编写, 使用`pandoc`转换为`tex`文件后编译, 模板会在之后附上.

## LICENSE

Copyright © 2016 corvo.

This project is licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
