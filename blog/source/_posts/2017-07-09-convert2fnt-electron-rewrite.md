title: 使用 Electron 重写 convert2fnt
date: 2017-07-09 12:54:42
tags: [Electron, convert2fnt]
---


大概是 14 年 2 月份的时候, 我使用刚学会 Python 写了一个小工具: [convert2fnt][2], 为此还写了一篇文章 [将一堆图片转化为BMFont工具][1] 介绍这个工具. 它的主要应用场景是这个样子的:

> 美术妹子出了一堆图片字, 但是在程序中使用 BMFont 是更加方便的, 这个时候你可以强硬的要求美术妹子重新用 Glyphdesigner 制作一份字体. 但是也可以很温柔的告诉她: "你先去忙吧, 剩下的交给我了.", 然后在妹子崇拜的目光下, 转身离去, 深藏功与名. 

<!--more-->

恩, 最初版的工具确实能够达到这个目的, 只是过程可能略微麻烦一下. 

在第一版的工具中, 因为用到了 ImageMagick 来拼接图片, 所以你需要先安装这个工具, 然后下载脚本, 安装依赖库, 开始使用. 当妹子满心期待看着你放大招的时候, 你TM还在配环境? 

![][3]

第二版中重点优化了配置环境复杂的问题. 使用 Pillow 替换了 ImageMagick, 还使用 PyInstaller 打包了可执行程序. 如果顺利的话, 下载一个可执行程序, 把图片按照规则命名好, 执行下就好了, 很大的进步有木有?

但是这个工具的使用范围还是局限在了技术人员手, 你不会指望美术策划同学来搭建一个 `Python + pip` 的环境吧? 我们确实很愿意帮助妹子, 但是策划同学来找你怎么办? 

![][4]

一个带界面的工具在这时候看起来确实会是一个更好的选择, 这也是我一直所努力的方向. 我尝试过 Python 的各种 GUI 方案: Tkinter, PyWx, PyQt 等等, 这些方案良莠不济, 有的看起来只是一个 Demo, 有的光搭建环境就会让你吐.

为此我各种尝试,徘徊在各种方案间挣扎了好几年, 直到 `Electron` 横空出世. 它似乎是一个含着金钥匙出生的项目, 有着 Github 这个全球最大同性交友网站的加成, 一出生便备受瞩目. 当然它也不负众望, 干翻它的前身 Node-Webkit, 使得越来越多的 App 选择使用它来制作:

![][5]

这里含金量最高的便是: Atom, VSCode 以及 Cocos 的最新产品: Cocos Creater. 其中 Creater 是真正让我下定决定使用 Electron 的项目, 前两个都只是个编辑器, 而 Creater 则是一个解决方案, 一个 2D 游戏的制作工具, 而且从目前的发展来看, 十分的健康. 

以目前 Electron 的火爆, 网上可以找到一大堆的教程, 相信大家可以很轻易的入门这个框架. 这里我和大家分享下自己的心得和踩过的一些坑.

## 1. Electron 多个组件的作用

### electron/electron-prebuilt

根据官方的解释:

> Note As of version 1.3.1, this package is published to npm under two names: electron and electron-prebuilt. You can currently use either name, but electron is recommended, as the electron-prebuilt name is deprecated, and will only be published until the end of 2016.

意思就是 `electron-prebuilt` 已经被废弃了, 建议直接使用:

```sh
npm install electron --save-dev
```

### electron-package/electron-builder

这两个都是 Electron 的打包工具, 

[electron-package][8] 只能打出可执行文件(Win:exe, Mac:app):

```json
"scripts": {
    "package": "electron-packager . --platform=win32 --arch=ia32 --electron-version=1.4.15 --overwrite --ignore=node_modules --ignore=.gitignore"
},
```

[electron-builder][9] 是一个更为先进, 简单的打包工具, 如果不想折腾的话可以直接选择它了.

```json
"scripts": {
    "pack": "electron-builder --dir",
    "dist": "electron-builder"
},
```

## 2. 一定要准备一个强有力的梯子...或者脑子

### npm install

安装 `Electron` 的途径之一就是通过 `npm`, 以国内的这个网络环境, 通过 npm 安装一些小的库还勉强可以, 对于 Electron 这种几十兆的库就显得捉襟见肘了, 时间长不说还很容易中断. 这时候就可以选择使用 [cnpm][6] 来做这些事情, 

![][12]

### npm run dist

electron-builder 第一次打包时会去下载 electron 的预编译文件, 这个文件很大, 它会默认去 github 上下载, 这时候如果没有翻墙工具就会很惨了.

我们可以使用 淘宝 提供的[镜像][13]来下载, 使用方法很简单, 在打包前先运行这个命令:

```sh
export ELECTRON_MIRROR="https://npm.taobao.org/mirrors/electron/"
```

原理可以看文章末尾的链接, 使用后真的是立竿见影, 大家可以对比下下载速度:

```sh
MacBook-Air:convert2fnt bilt$ npm run dist
Downloading tmp-6511-0-electron-v1.6.11-darwin-x64.zip
[==>                                          ] 8.8% of 46.08 MB (65.41 kB/s)
MacBook-Air:convert2fnt bilt$ export ELECTRON_MIRROR="https://npm.taobao.org/mirrors/electron/"
MacBook-Air:convert2fnt bilt$ npm run dist
Downloading tmp-6579-0-electron-v1.6.11-darwin-x64.zip
[============================>                ] 66.1% of 46.08 MB (951.77 kB/s)
```

# 最后

最后上一下新版 convert2fnt 截图, 现在还有一些收尾的工作再做, 很快你就会见到它:

![][10]

当策划再有类似的需求时, 可以直接扔给对方一个下载地址. 至于妹子, 当然是选择帮助她啦.

---

参考资料:

+ [常用Electron App打包工具][7]
+ [加速electron在国内的下载速度][11]

[1]: /2014/02/01/images_to_bmfont/
[2]: https://github.com/justbilt/convert2fnt
[3]: /face/yilianmengbi3.jpg
[4]: /face/ruhua.jpg
[5]: https://ww1.sinaimg.cn/large/006tNc79ly1fhdnp3klegj30ln0vcjza.jpg
[6]: https://npm.taobao.org/
[7]: http://www.jianshu.com/p/1c2ad78df208
[8]: https://github.com/electron-userland/electron-packager
[9]: https://github.com/electron-userland/electron-builder
[10]: https://ws2.sinaimg.cn/large/006tNc79gy1fhkaqhm6dnj30n80ixq5f.jpg
[11]: http://blog.tomyail.com/install-electron-slow-in-china/
[12]: https://zos.alipayobjects.com/rmsportal/UQvFKvLLWPPmxTM.png
[13]: https://npm.taobao.org/mirrors/electron/