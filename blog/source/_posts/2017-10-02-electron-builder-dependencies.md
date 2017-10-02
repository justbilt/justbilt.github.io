title: 解决 electron-builder 打包依赖问题
date: 2017-10-02 13:21:04
tags: [Electron, convert2fnt]
---

大概是三个月前, 写了 [使用 Electron 重写 convert2fnt][1] 这篇文章, 文章大概讲了我用 Electron 重写之前的一个 BMFont 字体生成工具的事情.

开发的过程还是比较顺利的, Node.js 的生态非常给力, 加上 Chrome, VSCode 加持, 开发和调试都很爽. 但由于我没有 WEB 开发的基础, 整过过程只能摸索加网上查资料, 中间之曲折, 自不足为外人道也, 但最终磕磕绊绊的也算搞出来了.

呃, 其实也没有那么完美. 

> Windows 版打包后发现运行不太正常, 但 Win 开发版以及 Mac 开发/发布版都运行正常.

有些诡异, 不过我自己只在 Mac 上使用, 加上工作比较忙, 便搁置了起来. 但谁愿意在自己头上悬在一把利剑呢, 万一哪天着急用 Win 版的怎么办.

趁着国庆放假还没有什么安排的时候, 赶紧搞一下.

<!--more-->

---

首先要搞明白无法正常运行的原因是什么, 打开 Chrom 开发者工具后发现了错误内容. 项目的一个依赖项 `jimp` 没有加载成功, 在调用这个模块的一个函数后抛出错误:

```javescript
const Jimp = require("jimp");
Jimp.read(imagePath, function (err, lenna) {}
---
Uncaught TypeError: Jimp.read is not a function
```

试着断点跟踪了下 `require("jimp")` 的过程, 由于自己对 js 的细节知之甚少, 也没有发现什么异常. 各种关键字组合 Google 也没有结果, 一度陷入了毫无头绪的困境.

在搜索的过程中发现 `electron` 和 `electron-builder` 都有了新的版本, 于是抱着侥幸的心态试着升级了下这两个库.

```diff
- "electron-builder": "^19.11.1",
- "electron": "^1.6.11"
+ "electron-builder": "^19.33.0",
+ "electron": "^1.7.8",
```

没想到, 竟然 ... ... 还是没有解决这个问题. 好吧, 其实也还在预料之中. 但是也有 "好消息", 出错的地方和之前不一样了.

```
module.js:472 Uncaught Error: Cannot find module 'bmp-js'
```

又是一番折腾, 尝试各种解决方案, 各种猜想, 各种组合. 这些尝试多没有什么变化, 有的尝试反而将我带到沟里, 离真相越来越远. 

最后我坚定了一个想法:

> 我的代码是没有任何问题的, 一定是打包的配置有问题.

果然, 在仔细阅读官方文档的时候发现了这个 [Two package.json Structure][2] , 原来官方已经推荐使用两个配置文件来配置各种参数, 一个配置开发时的参数, 一个配置打包时的参数. 于是便有了这样的目录结构:

```
├── app
│   ├── index.html
│   ├── index.js
│   ├── package-lock.json
│   └── package.json
└── package.json
```

同时, 官方还建议在开发的 `config.json` 中增加一个 `"postinstall": "electron-builder install-app-deps"` 来自动触发安装 `app` 目录中的依赖.

---

哈哈, 问题已经解决, 都是没有仔细看官方文档惹的祸, 但从此 Win 下同学也可以用上新版的工具啦.

[1]: /2017/07/09/convert2fnt-electron-rewrite
[2]: https://www.electron.build/tutorials/two-package-structure