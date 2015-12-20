title: "使用 Pyqtdeploy 发布你的 Pyqt 程序"
date: 2015-11-28 08:38:12
tags: [pyqt, pyqtdeploy]
---


在不久前我的另一篇博文[Pyqt5 MacOS 环境搭建][1]中介绍了如何在 Mac 上搭建 pyqt 的环境, 如果你恰巧看过那篇文章, 并且照做了, 那么非常不幸, 如果你打算使用 pyqtdeploy 发布的你的程序的话, 你可能得重新来一遍了!

我们需要准备几个几个东西:

1. 在[这里][2]下载 python 3.x 的源码包.
2. 在[这里][3]下载 sip 的源码包.
3. 在[这里][4]下载与你 qt 版本对应的 pyqt 的源码包.

注: 不要下载 3.5 , 因为后面某一步骤时会出错. -- 2015年11月29日


# 一. 安装

## 1. 安装 python3 :

因为 pyqtdeploy 要求 PyQt5 和 Python v3.2 或更高的版本, 因此我们需要安装 python3, 这里要记下你的 python 主要版本号 (e.g. 3.4), 后面会用到.

解压下载好的 python3 源码, 开启终端 cd 进这个目录:
```
./configure
```

等待结束后执行 `make`, 没有错误的话 `make install` .

##2. 安装 sip :

因为某些未知的原因, 我们需要先使用 brew 安装 sip:
```
brew install sip --with-python3
```

然后解压下载好的源码包, 打开终端 cd 进解压好的目录:

```
python3 configure.py --static --target-py-version=VERSION
```
替换 VERSION 为你的 python3 版本号, `--static` 是将 sip 编译为静态库的, 我们使用 brew 安装的 sip 没有生成静态库, 因此我们需要手动安装.

等待成功后, 执行 `make && make install`.

因为上一步会会覆盖 brew 安装的 sip 的执行文件, 所以我们需要重新 link 一下.

```
brew link --overwrite sip
```

然后我们`brew info sip`, 可以找到类似的一个路径:

```
/usr/local/Cellar/sip/4.16.9
```

在这个路径下有一个 `include` 文件夹, 这个文件夹的路径就是就是我们下面安装 `pyqt` 所需的路径.

##3. 安装 pyqt

这一步需要知道 `qmake` 的路径, 因此我们需将它所在目录加入到环境变量中:

```
export PATH=/path/of/your/Qt/clang_64/bin:$PATH
```

最终我们会输入这样的一行指令:
```
python3 configure.py --sip-incdir=PATH --target-py-version=VERSION --static
```

将上面的 `PATH` 修改为上面获取到的路径(e.g. `/usr/local/Cellar/sip/4.16.9/include`), 同时 `VERSION` 替换为你的 python 版本.

提示 `Do you accept the terms of the license?` 时输入 `yes`.

等待成功后,执行 `make`, 这一步需要编译好多东西, 可能会需要好久. 若是没有什么错误的话, 执行 `make install`.

##4. 安装 pyqtdeploy

安装好前面那些之后, 这一步变得十分简单:

```
pip3 install pyqtdeploy
```

到此为止, 我们 pyqtdeploy 的环境就搭建好了, 在任意目录输入 `pyqtdeploy`, 看到下面这个界面就算成功了:

![][5]


# 二. 使用

关于 pyqtdeploy 的使用还是建议大家看看[官方文档][6], 写的非常详细, 开始我寻求其他人写的教程, 但竟无一篇中文教程, 英文也寥寥无几, 最后还是回去啃文档. 下面介绍一下我的配置是什么, 至于期间踩坑无数, 自不足为外人道也.

首先我要创建一个 `pdy` 文件, pdy 是 pyqtdeploy 的工程配置文件. 需要注意的一点是, pdy 文件一定要保存在程序入口 py 文件的同级目录. 首先在你的项目源码目录中打开终端, 键入命令 `pyqtdeploy xxx.pdy`,(替换xxx为你想起的文件名) 然后就会弹出一个gui界面.


## 1. Application Source:

![][7]

Name: 填入你程序的名称
Main Script file: 程序的入口文件
Application Package Directory: 程序的其他 py 文件

## 2. qmake

这一页没有什么特别的设置

## 3. PyQt Modules

这一页中你的项目中使用了什么模块就勾选什么模块就可以了.

## 4. Standard Library

默认设置

## 5. Other Packages

默认设置

## 6. Other Extension Modules

默认设置

## 7. Locations

这一页的设置是重中之重, 曾在这里浪费了很多的时间.

![][8]

**Interpreter**: python 可执行文件路径
**Source directory**: 还记得我们之前下载的 python 源码, 解压到一个目录, 填写路径到这里就ok了 !
**Include directory**: python 的头文件路径
**Python library**: python 的静态库文件路径
**Standard python library**: python 标准库文件路径
**Build directory**: 构建用路径
**qmake**: qmake 文件路径

## 8. Build

![][9]

这一页必要重要的是勾选 `Verbose output` , 这样子出错了能够比较准确的定位. 另外勾选 `Additional Build Steps`, 可以帮你编译运行.


# 三. Q&A

1. ImportError: No module named xxx

如果这个模块是你项目的一个本地依赖模块, 那么请检查你的 pdy 文件是否保存在代码入口文件的同一级目录.

2. error: use of undeclared identifier '_Py_BEGIN_SUPPRESS_IPH'

是否使用了 python3.5 , 我切换为 python3.4 就 ok 了!


---

转眼间又过去了一个月多月, 每周一篇好不容易坚持了3周就这样断掉了. 主要原因是还是懒, 次要原因是公司项目即将上线, 所以工作比平时要紧张好多, 最近两周周六也开始加班了!  对目前这个项目还是比较看好的, 加油!

下次分享下我在最近这次热更新上的一些收获!


[1]: /2015/10/17/setup-pyqt5-on-mac
[2]: https://www.python.org/downloads/source/
[3]: https://riverbankcomputing.com/software/sip/download
[4]: http://sourceforge.net/projects/pyqt/files/PyQt5/
[5]: http://static.zybuluo.com/justbilt/0ud9c1sdxopu72gu02htefgb/%E5%B1%8F%E5%B9%95%E5%BF%AB%E7%85%A7%202015-12-05%20%E4%B8%8B%E5%8D%882.28.29.png
[6]: http://pyqt.sourceforge.net/Docs/pyqtdeploy/
[7]: http://ww4.sinaimg.cn/large/7f870d23gw1ez5zupeyb3j20rw0hs42h.jpg
[8]: http://ww4.sinaimg.cn/large/7f870d23gw1ez67jp69umj20rw0f7jut.jpg
[9]: http://ww4.sinaimg.cn/large/7f870d23gw1ez69ew90vbj20r40ef7b2.jpg