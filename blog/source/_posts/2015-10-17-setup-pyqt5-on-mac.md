title: "Pyqt5 MacOS 环境搭建"
date: 2015-10-17 14:46:49
tags: [pyqt]
---

尝试过用各种 python 的 gui 库来写一些小工具, TkInter, wxPython, Pyfltk , PyQt等, 最终发现还是只有 `wxPython` 和 `PyQt` 能相对靠谱一些, 控件全, 文档丰富, 使用的人多. 因为曾经使用搞过 qt , 所以最终选择了 PyQt, 这次我们来说一下如何在 Mac 上安装.


# 1. 安装 qt

pyqt 其实就是 qt 的 python 绑定, 所以我们首先需要安装 qt, 版本可以自行选择, [最新版][1] 的下载地址, [历史版本][2]的地址. 如果追求最新版的话, 最好是去 pyqt 的[下载网站][3]看那一下最新版是什么, 因为 pyqt 的更新速度会落后于 qt.

![屏幕快照 2015-09-12 上午7.37.42.png-117.1kB][4]

当看到这个界面时, qt 就安装成功了.

# 2. 安装 sip

sip 是一个 python 调用 c 的工具, 官方网址在[这里][5], 我们可以按照官网上的指南去安装, 也可以选择另一种更简单的方式:

```
brew install sip
```

# 3. 安装 pyqt


----------


我们首先去[这里][6]下载适合自己版本, 解压, 并在终端 cd 进这个目录. 执行:

```
python configure.py
```

如果出现 `Error: Use the --qmake argument to explicitly specify a working Qt qmake.` 错误, 则是因为我们没有将 `qmake` 加入到环境变量中. 那么 qmake 在哪里呢? 根绝安装路径和版本,差不多是在这样的一个路径中:

![屏幕快照 2015-09-13 下午4.41.29.png-75.1kB][7]

知道了路径, 我们可以似乎可以通过 `--qmake` 参数指定 qmake 目录 ,但其实我们可以把它临时的加入到环境变量中, 在终端中键入:

```
export PATH=/path/your/qt/version/clang_64/bin:$PATH
```

再次执行`python configure.py`, 一切顺林的话会遇到一个选择 license 的提示, 我们输入 `yes` 即可.

等待 `configure` 完成, 我们执行:
```
make
```

这次可能并没有那么顺利, 我遇到这样的错误:

```
In file included from ../qpy/QtCore/qpycore_api.h:30:
../qpy/QtCore/qpycore_public_api.h:26:10: fatal error: 'sip.h' file not found
#include <sip.h>
         ^
1 error generated.
make[1]: *** [qpycore_post_init.o] Error 1
make: *** [sub-QtCore-make_first-ordered] Error 2
```

这时候我们需要传入 sip 的 include 路径, 如果是通过 brew 安装的(如果不是, 请看文章末尾的Q&A), 可以通过 `brew info sip`获得:

```
$ brew info sip
sip: stable 4.16.9 (bottled), HEAD
Tool to create Python bindings for C and C++ libraries
http://www.riverbankcomputing.co.uk/software/sip
/usr/local/Cellar/sip/4.16.9 (10 files, 908K) *
```

然后重新执行 

```
python configure.py --sip-incdir=/path/of/your/sip/4.16.9/include
```

等待成功后, 我们可以执行:

```
make && make install
```

到此为止, 我们安装就完成了. 下面让我们用 [zetcode][8] 上的例题测试一下:

```
#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""
ZetCode PyQt5 tutorial 

In this example, we create a simple
window in PyQt5.

author: Jan Bodnar
website: zetcode.com 
last edited: January 2015
"""

import sys
from PyQt5.QtWidgets import QApplication, QWidget


if __name__ == '__main__':
    
    app = QApplication(sys.argv)

    w = QWidget()
    w.resize(250, 150)
    w.move(300, 300)
    w.setWindowTitle('Simple')
    w.show()
    
    sys.exit(app.exec_())
```

把上面那段代码保存在一个python文件中, 然后执行:

```
python test.py
```

如果出现下面这个界面, 就说明你成功了!

![屏幕快照 2015-09-14 上午8.02.58.png-20kB][9]


----------


如果你没有通过 brew 安装 sip , 你可能遇到的问题:

## 1. 安装 pyqt , 执行 `python configure.py` 时找不到 sip

```
sh: sip: command not found
Error: 'sip -V' did not generate any output.
```

## 解决方案:

知道你安装 sip `make install` 时的一些信息, 比如:
```
$ make install
cp -f sip /usr/local/Cellar/python/2.7.10_2/Frameworks/Python.framework/Versions/2.7/bin/sip
```

然后在终端中执行:

```
export PATH=/path/of/your/python/2.7.10_2/Frameworks/Python.framework/Versions/2.7/bin/:$PATH
```

## 2. fatal error: 'sip.h' file not found
错误:
```
In file included from qpycore_post_init.cpp:25:
In file included from ../qpy/QtCore/qpycore_api.h:30:
../qpy/QtCore/qpycore_public_api.h:26:10: fatal error: 'sip.h' file not found
#include <sip.h>
         ^
1 error generated.
make[1]: *** [qpycore_post_init.o] Error 1
make: *** [sub-QtCore-make_first-ordered] Error 2
```

[解决方案][10]:

```
python configure.py --sip-incdir=/path/of/your/Downloads/sip-4.16.9/siplib
```


---

Update 2015年11月25日:

## 如何为 python3 安装 pyqt5

### 1. 安装 sip 时附加额外参数 `--with-python3`

```
brew install sip --with-python3
```

### 2. 安装 pyqt 时附加额外参数 `--target-py-version=VERSION` (e.g. 3.4)

```
python3 configure.py --sip-incdir=/path/of/your/sip/include --target-py-version=VERSION
```








  [1]: http://www.qt.io/download-open-source/#section-2
  [2]: http://download.qt.io/archive/qt/
  [3]: http://sourceforge.net/projects/pyqt/files/PyQt5/
  [4]: http://static.zybuluo.com/justbilt/krc3gqufsqx99msip1jw9fqy/%E5%B1%8F%E5%B9%95%E5%BF%AB%E7%85%A7%202015-09-12%20%E4%B8%8A%E5%8D%887.37.42.png
  [5]: http://pyqt.sourceforge.net/Docs/sip4/installation.html
  [6]: http://sourceforge.net/projects/pyqt/files/PyQt5/
  [7]: http://static.zybuluo.com/justbilt/6tbyfo13vr6hf4cuztepeja9/%E5%B1%8F%E5%B9%95%E5%BF%AB%E7%85%A7%202015-09-13%20%E4%B8%8B%E5%8D%884.41.29.png
  [8]: http://zetcode.com/gui/pyqt5/
  [9]: http://static.zybuluo.com/justbilt/qg63w41bobru04w0oki6iu8g/%E5%B1%8F%E5%B9%95%E5%BF%AB%E7%85%A7%202015-09-14%20%E4%B8%8A%E5%8D%888.02.58.png
  [10]: http://python.6.x6.nabble.com/PyQt-5-1-1-Compilation-Error-td5037071.html