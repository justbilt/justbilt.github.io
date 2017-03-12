title: quick-x 使用静态库加速 iOS 打包
date: 2016-05-29 21:20:58
tags:
- Quick-Cocos2d-x
- iOS
description: "亲爱的, 打完这个包就回家"
---

quick-x 项目的 iOS 工程使用 `Tgarget Dependencise` 依赖 `cocos2d_lib` 和 `cocos_lua_bindings` 工程.

![QQ20160529-0.png-27kB][1]

这样子在 iOS Archive 时会重新编译这两个项目, 十分痛苦, 尤其是一次出七八个渠道的包, 好几个小时就耗在里面了.

为什么不用静态库, 编译出 .a , 使用时直接链接就可以了嘛.

# 一. 编译静态库

找了一下, 原来早已经有小伙伴想到了这点, 这篇文章 [Build cocos2d-x fat static library][2] 就特别棒. 从中我们可以发现一个特别有用的脚本 [buildstaticlib.sh][3], 可以直接使用 xcode 工程编译出静态库.

不过这个脚本只能编译出 `Release` 版, 我修改下可以传入 `configuration`, 这样我们可以分别编译出 Debug 和 Release 版的静态库啦, 我修改后的[文件在这里][4].

因为我们要编译出多个静态库, 所有又写了另一个脚本 `build.sh` 调用 `buildstaticlib.sh` , 内容如下:

```sh
./buildstaticlib.sh $QUICK_V3_ROOT/cocos/scripting/lua-bindings/proj.ios_mac/cocos2d_lua_bindings.xcodeproj "libluacocos2d iOS" "Release"

./buildstaticlib.sh $QUICK_V3_ROOT/build/cocos2d_libs.xcodeproj "libcocos2d iOS" "Release"

./buildstaticlib.sh $QUICK_V3_ROOT/cocos/scripting/lua-bindings/proj.ios_mac/cocos2d_lua_bindings.xcodeproj "libluacocos2d iOS" "Debug"

./buildstaticlib.sh $QUICK_V3_ROOT/build/cocos2d_libs.xcodeproj "libcocos2d iOS" "Debug"
```

运行成功后会在当前目录生成 4 个 .a 文件, 下一步中将会用到.
```sh
.
├── build.sh
├── buildstaticlib.sh
├── libcocos2d\ iOS-debug.a
├── libcocos2d\ iOS.a
├── libluacocos2d\ iOS-debug.a
└── libluacocos2d\ iOS.a
```


# 二. 使用静态库
使用 XCode 打开 `proj.ios_mac` 目录下的 `xxx.xcodeproj` 工程.

## 1. 移除 Tgarget Dependencise

首先移除对 `cocos2d_lib` 和 `cocos_lua_bindings` 工程的依赖, 右键点击 `Delete` 然后选择 `Remove reference` 就可以.

![QQ20160529-1.png-49.2kB][5]

## 2. 添加 Other Linker Flags

我们静态库的依赖是在这里添加的, 在 Debug 和 Release 选项中分别加入对应的静态库.
![QQ20160529-3.png-117.8kB][6]


这样就完成啦, 尝试一下 Archive 的速度吧 !

# 三. 其他

## 1. 调试环境与生产环境

我们改成静态库后, 调试 cocos 引擎的代码会多有不便, 而且一旦修改了 cocos 的代码, 就得重新生成静态库, 对于开发阶段太不友好了.

我们的解决方案, 就是再建立一个 debug 工程, 这个工程依旧使用依赖项目的方式编译 cocos , 调试流程和以前一致. 上线打包时则使用我们的静态库版本, 多渠道也做在这个工程中, 享受静态库带来的编译加速.

最终我们的目录结构是这个样子的:

```sh
.
├── runtime-src
│   ├── Classes
│   ├── proj.android
│   ├── proj.android_no_anysdk
│   ├── proj.android_studio
│   ├── proj.ios_mac
│   ├── proj.win32
│   └── proj.wp8-xaml
└── runtime-src-debug
    ├── Classes
    └── proj.ios_mac
```

## 2. 生产环境工程瘦身

这一步可有可无, 我的代码洁癖又犯了, 所以顺手改了一下. 

这时的生产环境除静态库外的内容和调试环境几乎一致, 然而有一些东西是我们用不到的:

1. mac 平台对应的内容
2. Classes/runtime 下的内容

删除这些时改动了 AppDelegate 中的东西, 这也上一步为什么从 `runtimes-src` 目录复制了一份.

## 3. 进一步加速编译

这一步我们目前还没有做, 只是一个想法.

修改完使用静态库后, 编译速度得到了很大的提升, 但还没有达到极致, 因为 quick 特有的 c++ 文件还是以文件形式存在于工程中的. 所有 Archive 的时候还是有一百多个源文件需要编译.

如果我们能进一步拆分, 新建一个 lib 工程, 将 quick 的源文件添加和依赖项目添加进去, 我们的游戏只依赖这样的一个静态库, 是否可以达到一个极致的编译速度 ?

## 4. 静态库文件的版本管理

在编译出 debug 版的静态库之前, 我还有想法将这几个静态库压缩上传到 git 上, 编译出 debug 版之后, 我就一个想法, ignore them !

所以我最终的策略 将这几个 .a 在 git 上忽略掉, 同时在那个目录保留了一个编译脚本, 谁要用到 iOS 项目的时候, 发现没有 .a , 自己运行脚本编译一份就可以啦 !

## 5. 编译脚本优化 ?

现在那个编译脚本会编译出一个 `fat`(armv7 armv7s arm64 i386 x86_64) 版的静态库, 内部实现其实是编译了好多次, 导致现在编译时间非常长.

思考:
1. 是否有必要编出 `i386 x86_64` 版本 ?
2. 看到虾神的一篇文章貌似说可以以 armv7+arm64, i386+x86_64 组合两次打出所有版本.

## 6. 最终 Archive 出的包会比使用源文件大 ?

看到网上有过这个说法, 我没有在修改前后分别 Archive 对比包体, 不太严谨. 

但和我之前某一次的包相比, 只大了几百KB, 还不太确定是不是与使用静态库有关系, 大家在修改时可以注意对比一下.


  [1]: http://static.zybuluo.com/justbilt/i0vtatej9k1wx2d9ck8fmqhy/QQ20160529-0.png
  [2]: http://www.nicnocquee.com/2016/01/20/build-cocos2d-x-fat-static-library.html
  [3]: https://gist.github.com/nicnocquee/9dc4c4a128d7c0bafe23#file-buildstaticlib-sh
  [4]: https://gist.github.com/justbilt/903ef34b568527d57c9bd9bf4069ed72
  [5]: http://static.zybuluo.com/justbilt/3d9eiua1tind8aycakjwymgl/QQ20160529-1.png
  [6]: http://static.zybuluo.com/justbilt/5g1o6ozh1j4dlybcjftfybnr/QQ20160529-3.png