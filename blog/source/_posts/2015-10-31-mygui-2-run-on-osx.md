title: MyGUI学习笔记(二) 在 MacOS 上运行MyGUI
date: 2015-10-31 22:52:44
tags: [MyGUI]
---


不知大家是否还记得我的这篇文章 [MyGUI学习笔记(一) 在Windows上运行MyGUI][1], 这篇文章讲述了在 Windows 上编译运行 MyGUI 的过程. 文中提到了在 Mac 遇到了很多坑, 确实如此, 当时的 MyGUI 虽然号称 Cross Platform , 但是在 Mac 却有好多东西没有实现, 编译起来困难重重, 以至于我不得不切换到 Windows 系统上, 因此便有了那篇文章.

<!-- more -->

那么今天为什么又要写这个了呢? 因为这几个月内, MyGUI 团队针对 Mac 做了好多修改, 使得脱离 Ogre 之后 Demo 也能够跨平台运行了!

那么我们就来看一下如何在 Mac 上运行 MyGUI 吧!

# 零. 准备工作

如同 Windows 一样, 我们需要做一些准备工作. 因为在这个阶段这些工作互不影响, 因此大家并行操作, 高效利用时间.

1. 从 github 上 clone 最新的 [MyGUI 源码][2].
2. 下载 [CMake Mac 版][3], 并安装.
3. 使用 Homebrew 安装 `sdl2_image`.
3. 下载已经编译好的 Mac 版 [Orge Dependencise][4], 并解压到 mygui 的根目录.

注: 这里我偷懒使用了已经编译好的依赖库, 毕竟那不是我们的主要目的. 当然你也可以自行编译, 源码位于[这里][5]. 

# 一. 生成 Xcode 工程

首先我们运行 CMake, 然后在 `Where is the source code:` 栏选择你的 clone 下来的 mygui 源码路径. 在 `Where to build the binaries:` 选择生成目录, 我是选择生成在 mygui 的 `bin` 路径下.

![QQ20151101-0.png-24.9kB][6]

然后点击 `Configure` 按钮, 如果弹出询问是否创建 `bin` 路径, 请同意. 在接下来的弹出界面的下拉列表中选择 `Xcode`, 点击确定.

等待片刻后, 会弹出一个错误提示:

> Error in configuration precess, project files may be invaild

在日志窗口查看错误信息可以得知我们需要修下 `MYGUI_RENDERSYSTEM` 的值. 为方便查找, 我们可以将 CMake 界面的 `Grouped` 复选框选中, 这样所有的配置都会分组显示. 

在中间红色区域的 `MYGUI` 分类下找到 `MYGUI_RENDERSYSTEM` 选项, 将其值修改为 `4`, 鼠标悬停可以查看数字做代表的意义.

![QQ20151101-1.png-61.6kB][7]

再次点击 `Configure` 按钮, 无任何错误提示即为成功. 接下来点击 `Generate` 生成 Xcode 工程.

# 二. 编译 MyGUI

打开 `bin` 目录下的 `MYGUI.xcodeproj` 工程, 开始编译.  编译过程中会遇到如下编译错误:

> /Users/.../Data.cpp:136:11: No viable conversion from 'std::__1::nullptr_t' to 'DataPtr' (aka 'shared_ptr<tools::Data>')

对于这个错误说明呢大家可以[看这里][11], 我的解决方案是把出错地方的 `nullptr` 都改为 `NULL`, 一共需要修改十几处吧.

![QQ20151101-2.png-82.3kB][8]

然后就编译成功了!

# 三. 运行 MyGUI 示例

当我们选中一个示例运行的时候, 会遇到这个崩溃错误:

> An exception has occured : MyGUI EXCEPTION : No root widget. ['ColourPanel.layout]
 in MyGUI at /Users/bilt/Documents/mygui/Common/BaseLayout/BaseLayout.h (line 126)libc++abi.dylib: terminating with uncaught exception of type MyGUI::Exception: MyGUI EXCEPTION : No root widget. ['ColourPanel.layout]
 in MyGUI at /Users/bilt/Documents/mygui/Common/BaseLayout/BaseLayout.h (line 126)
 
经过追踪, 是没有找到 `resources.xml` 没有找见的原因, 它位于 `path/of/mygui/bin/bin`目录, 而默认应用的工作目录不是这里, 因此我们需要设置下工作目录.

选中 Scheme 列表末尾的 `Edit Scheme` (或者按下`cmd+shift+,`), 然后再 `Options` 标签下将 `Working Directory` 选择为你的生成 `bin` 目录.

![QQ20151101-5.png-97.1kB][9]

再次运行就可以啦!

上一张截图纪念一下:

![QQ20151101-3.png-371.3kB][10]

---

MyGUI 从07年发展到现在已有8年的历史了, 仍然有这么多人在开发使用, 真的让人敬佩不已. 同时 MyGUI 的代码简洁, 优美, 十分适合学习研究, 下面我将从浅入深的和大家分享我的学习经验!


  [1]: http://blog.justbilt.com/2015/05/29/mygui-1-run-on-windows
  [2]: https://github.com/MyGUI/mygui
  [3]: https://cmake.org/download/
  [4]: http://jaist.dl.sourceforge.net/project/ogre/ogre-dependencies-mac/1.8/OgreDependencies_OSX_20120525.zip
  [5]: https://bitbucket.org/cabalistic/ogredeps
  [6]: http://static.zybuluo.com/justbilt/ultxdpj801k222om0tyr205m/QQ20151101-0.png
  [7]: http://static.zybuluo.com/justbilt/xiit07e4x4z8khyymr3d0a8z/QQ20151101-1.png
  [8]: http://static.zybuluo.com/justbilt/tfk0iznreipqw8m02ge20d3r/QQ20151101-2.png
  [9]: http://static.zybuluo.com/justbilt/6smjifuob0cc0g24yis212t1/QQ20151101-5.png
  [10]: http://static.zybuluo.com/justbilt/qeeokj32fvk2qgkufmwk0ki9/QQ20151101-3.png
  [11]: https://github.com/MyGUI/mygui/pull/82

