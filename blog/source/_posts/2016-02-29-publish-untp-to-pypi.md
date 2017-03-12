title: 将 untp 发布到 Pypi 上
date: 2016-02-29 23:33:24
tags:
- Tool
- untp
description: "年轻人的第一个 pip 包"
---

上一篇年结的时候有提到 [untp][1] 这个小工具, 它是我在 github 收获 star 数最多的一个项目. 这个项目本是无心之举, 既然受大家欢迎, 那么一定要好好维护下去 ! 对于它后续的发展, 我打算从两个方面入手:

1. 更便捷的安装
2. 支持更多的格式

之前唯一一次发布时用 PyInstaller 打包成可执行文件来发布的, 很方便, 出来的安装包也很小! 但是我时常会收到在 Mac 上崩溃的反馈:

```
illegal instruction 4
```

在这个错误上我花费了特别大的精力, 因为在推荐给朋友的时候吹得天花乱坠, 实际却完全运行不起来, 这也太不给我面子了吧.

Google 上对于这个问题也是众说纷纭, 没有一个能帮到我. 我尝试过各种解决方案, 升降级 PyInstaller, 用 Python3 重写, 用 PyQtDeploy 打包. 但却总是无功而返. PyQtDeploy 倒是可以解决问题, 但是打的包足足有 30~40MB , 想想还是算了吧.

最后我仔细想了想, 在 Mac 上完全没有必要打包成一个可执行文件呀, 系统自带 python 环境, 直接通过 pip 安装一下就可以了嘛 ~

关于如何制作,上传,发布自己的项目, 推荐大家阅读这篇文章 [发布python的包至pypi服务器][2] , 文章写的很好, 里面已经提到内容我在下面就不赘述了.


# 1. 制作 setup.py

我的 setup.py 文件如下:

```python
from setuptools import setup, find_packages

setup(
    name = 'untp',
    version = '1.0.2',
    keywords = ('untp', 'texturepacker'),
    description = 'A command line tool to split TexturePacker publish file.',
    license = 'MIT License',
    install_requires = [
        'Pillow',
        'parse'
    ],
    url = 'https://github.com/justbilt/untp',
    author = 'justbilt',
    author_email = 'wangbilt@gmail.com',
    scripts=['untp.py'],
    entry_points={
        'console_scripts': [
            'untp = untp:main',
        ],
    }
)
```


## entry_points

使用 entry_points 字段, 可以为项目添加可执行命令, 在我上面的配置中, 我就添加了 `untp` 这个名命令, 会执行 untp.py 的 main 函数.

关于 entry_points 的更多用法, 可以参考这篇文章 [Automatic Script Creation][3] .


## scripts

这里面值得一提的是 `scripts` 字段, 这个字段好多文章都没有提, 但是在我这里却引发了一个大问题. 我的目录结构如下:

```sh
untp
├── README.md
├── setup.py
└── untp.py
```

因为项目比较小, 我就直接将 untp.py 直接裸露在项目根目录, 开始没有添加 scripts 字段, 打包安装 `import` 都没有问题, 但是运行 `untp` 时却会报最经典的 `ImportError` 错误:

```sh
wanghaitaodeMacBook-Air:untp bilt$ untp
Traceback (most recent call last):
  File "/usr/local/bin/untp", line 9, in <module>
    load_entry_point('untp==1.0.2', 'console_scripts', 'untp')()
  ...
  File "/usr/local/lib/python2.7/site-packages/pkg_resources/__init__.py", line 2361, in resolve
    module = __import__(self.module_name, fromlist=['__name__'], level=0)
ImportError: No module named untp
```

然后我就要疯了, 这里什么个鬼情况 !!! 直到我看到了这个问答: [module-not-found-during-load-entry-point-in-python][4] . 我才意识到, 原来我一直都没有安装对, 之所以能够 import 成功, 是因为我就在当前 (untp.py) 目录运行的 python 终端.

然后去啃官方文档, 经过一番尝试, 其他人使用的 `packages = find_packages()`, 在我这里并不好使, 只有手动指定 scripts 才可以 !


# 2. 测试

我还是推荐大家先在 `testpypi.python.org` 上测试正确了再上传到正式的 `pypi.python.org`, 因为一但在正式服务器提交了, 就必须以版本号迭代的代价来更正上次提交. 而在测试服务器上发现问题的话, 登录删除一下有问题的版本就可以了!

testpypi 也是需要注册的, 和 pypi 的注册方法一致, 注册完别忘了去邮箱验证一下.

注意上传的时候不一定会成功, 说在 `.pypirc` 中找不到 testpypi 的配置, 解决方案可以看这里: [Test PyPI Server][5] .

```sh
python setup.py register -r https://testpypi.python.org/pypi
python setup.py upload -r https://testpypi.python.org/pypi
pip install -i https://testpypi.python.org/pypi <package name>
```

然后使用上面几条命令进行测试注册,发布,安装.


# 3. 发布

这边没有什么说的, 测试没有问题之后, 发布也不会出什么问题, 只是网址不一样而已. 写这篇文章的时候, 大家已经可以使用 `pip` 来安装最新版的 untp 了:

```
pip install untp
```



--EOF--

[1]: https://github.com/justbilt/untp
[2]: http://yejinxin.github.io/distribute-python-packages-to-pypi-server/
[3]: https://pythonhosted.org/setuptools/setuptools.html#automatic-script-creation
[4]: http://stackoverflow.com/questions/19718813/module-not-found-during-load-entry-point-in-python
[5]: https://wiki.python.org/moin/TestPyPI