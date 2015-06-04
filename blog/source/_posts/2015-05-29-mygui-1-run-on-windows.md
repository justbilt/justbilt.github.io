title: "MyGUI学习笔记(一) 在Windows上运行MyGUI"
date: 2015-05-29 23:30:11
categories: [MyGUI]
---

最近突然对 MyGUI 燃起了一些兴趣, 打算学习一番. 第一步肯定是跑起来 Demo 吧, 我主力系统是 MacOS , 首先肯定是考虑在 Mac 上搞起. 虽然 MyGUI 号称是 Corss Platform 的, 然而脱离了 Ogre 之后的 MyGUI 就只能跑在 windows 上了. 额, 在 Mac 上的趟坑之旅我们暂且不提, 让我们来曲线救国吧 -- 在Windows上运行MyGUI.

<!-- more -->

---

# 零. 准备工作

在正式开搞前, 我们需要准备几样东西, 这几样东西可能在墙外, 大家自备梯子.

1. 从 MyGUI 的 [github][1] 上下载最新代码, 解压到一个目录备用.
2. 从 CMake 的官网下载 [CMake][2] 的最新安装包, 安装之.
3. 从 bitbucket 上下载 [ogredeps][3].

# 一. 编译 ogredeps

因为 MyGUI 一开始就是为 Ogre 设计的, 所以会用到 Ogre 的一些依赖库, 我们这一步就需要编译这些依赖库.

## 1. 运行CMake

在上方输入框中选择 ogredeps 的源码路径, 下面选择 build 路径. 如下图:

![][4]

build 路径可以随意设置, 一般会选择和source目录一致.

## 2. 生成 IDE 工程 

点击`Configure`按钮, 在弹出的框中选择自己的 VS 版本, 然后点击`finish`按钮.

![][5]

这一步一般不会出什么问题, 成功后会刷新出一坨红色的东西, 不要担心, 那不是错误. 点击`Generate`按钮, 生成VS的工程文件.

## 3. 编译并拷贝生成文件

去 ogredeps 的根目录, 运行生成的`OGREDEPS.sln`文件打开VS, 在`Solution Explorer`窗口中找到`INSTALL`工程,右键`Build`:

![][6]

等待编译成功后, 去 ogredeps 根目录下找到刚生成的 `ogredeps` 文件夹, 里面包含有头文件和库文件, 把它拷贝到 MyGUI 的根目录下, 并重命名为 `Dependencies`.

好了, 这一步就算完成了.

# 二. 编译 MyGUI

## 1. 生成 VS 工程

第一步还是运行 CMake, 不过这次要先`Delete Cach` 下, 可以在`File`菜单下找到它:

![][7]

然后选择 source 路径 和 build 路径, 完成之后, 点击`Configure`按钮, 在弹出的框中选择自己的 VS 版本, 然后点击`finish`按钮.

一番等待之后, 弹出了错误提示! 这次是确实有错误了:

![][8]

从红框部分,可以看到错误细节:

```
+ ogre: Support for the Ogre render system <>
```

MyGUI 的设计理念确实是按照 Cross Platform 设计来搞的, 所以他的 render system 是有多套实现的, 默认是用 Ogre 的渲染. 往上看一下就能找到解决方案:

> -- Also check that you buildind with RenderSystem that you need or set
  another with -DMYGUI_RENDERSYSTEM=<1 2 or 3 for Direct3D_9 OGRE or OpenGL>

好的, 那我们找到 `MYGUI_RENDERSYSTEM` 这个宏, 给他改成 `3 <OpenGL>` 就可以了! 在 CMake 界面的一坨红色的东东中找到 `MYGUI_RENDERSYSTEM`, 如下图:

![][9]

鼠标悬停的话会有详细介绍.

再次点击`Configure`按钮, 这次就不会有错误了! 完成后点击`Generate`按钮, 生成VS的工程文件.

## 2. 编译 MyGUI

在 MyGUI 的根目录下找到 `MYGUI.sln` ,运行它! 还是在`Solution Explorer`窗口中找到`INSTALL`工程,右键`Build`. 这次编译会略微久一些, 不过正常情况下是不会有意外滴!

![][10]

编译完成后, 会在 `bin/debug` 目录下看到所有的 Demo 的运行文件, 可以挨个运行看一下都是什么.

## 3. 调试项目

如果我们想调试某个 Demo, 需要在这个Demo的工程文件上右键设为启动项`Set as StartUp Project`, 然后 `F5` 启动调试, 不过在运行时会因为找不到资源崩溃掉. 估计原因是工作目录不正确的, 我们需要修改下, 项目右键`Properties`:

![][11]

将`$(ProjectDir)`改为`$(OutDir)`, 确定, 再次启动调试, 成功!



---

至此为止, windows 上的运行调试已无问题, 可以开始学习了!

--EOF--


[1]: https://github.com/MyGUI/mygui/releases
[2]: http://www.cmake.org/
[3]: https://bitbucket.org/cabalistic/ogredeps
[4]: /img/QQ20150530-1.jpg
[5]: /img/QQ20150530-2.jpg
[6]: /img/QQ20150530-3.jpg
[7]: /img/QQ20150530-4.jpg
[8]: /img/QQ20150530-5.jpg
[9]: /img/QQ20150530-6.jpg
[10]: /img/QQ20150530-7.jpg
[11]: /img/QQ20150530-8.jpg