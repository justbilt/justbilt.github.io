---
title: "iOS duplicate symbol静态库冲突"
categories: iOS
date: 2015-02-06 10:33
---


今天队友在接入i4(爱思)的sdk时遇到了duplicate symbol错误,原因是i4的sdk中含有SBJSON的静态库,而quick也含有一份,最终导致错误:


## 解决方案一:

使用[这篇][1]文章内的解决方案搞一下, 核心内容摘录如下:

本文的目标：合并`libDomobOfferWallSDK.a`、`libDomobAdWallCoreSDK+UI.a`、`libDomobAdSDK.a`


1.依次查看所有需要合并的lib库所支持的框架类型。i386表示支持iphone模拟器，armv6应用在iphone 3gs上，armv7应用在iphone4，armv7s应用在iphone4s以及iphone5，arm64应用在最新的iphone5s上。

```sh
lipo -info libDomobOfferWallSDK.a
输出是：Architectures in **the fat file**: libDomobOfferWallSDK.a are: armv7 armv7s i386
```

`fat file`表示这个库里面合并了多于1个框架。


2.将lib库拆分出armv7、armv7s、i386类型的.a：

```
lipo -extract_family armv7 -output libDomobOfferWallSDK_arm.a libDomobOfferWallSDK.a
```

> 这里要注意:libDomobOfferWallSDK_arm.a仍旧是fat的.

```
lipo -info libDomobOfferWallSDK_arm.a
打印出：Architectures in the fat file: libDomobOfferWallSDK_arm.a are: armv7 armv7s
```

接着用如下命令进一步分离出单独的armv7、armv7s版本：（非常重要的步骤）

```
lipo libDomobOfferWallSDK_arm.a -thin armv7 -output libDomobOfferWallSDK_armv7_final.a
lipo libDomobOfferWallSDK_arm.a -thin armv7s -output libDomobOfferWallSDK_armv7s_final.a
```

而对于i386则不用上述那么麻烦，直接用第一句-extract_family指令就能分离出只含i386版本的库：

```
lipo -extract_family i386 -output libDomobOfferWallSDK_i386.a libDomobOfferWallSDK.a
```

经过上面操作后，我们得到了第一个待合并库libDomobOfferWallSDK.a 的armv7、armv7s、i386三个版本的库。剩下的两个待合并库同理。

3.分离出目标文件(.o)

进行实质上的合并。xcode里提示duplicate symbol都是说***.o与***.o冲突。注意，只有非fat file，也就是只含有单种版本的lib库才能分离出.o文件

```
ar -x ../libDomobOfferWallSDK_armv7_final.a
ar -x ../libDomobAdWallCoreSDK+UI_armv7_final.a
ar -x ../libDomobAdSDK_armv7_final.a
```
这一步最好是在lib*_final.a的同级目录下新建一个文件夹，专门用来接收解压出来的.o文件。同种版本(如armv7)的都解压到这个文件夹中，相同的.o文件他会自己覆盖掉。

4.从.o文件中合并成静态库：

```
libtool -static -o ../libDomobUniversal-armv7.a *.o
armv7s、i386版本同理。
```

5.合并成最终的通用静态库：

```
lipo -create -output libDomobUniversal.a libDomobUniversal-i386.a libDomobUniversal-armv7.a libDomobUniversal-armv7s.a
```

## 解决方案二:

上边的方法略显繁琐, 我们可以考虑将quick-cocos2d-x的SBJSON给干掉,为了不影响其它平台的使用,我们可以新建一个target `quickcocos2dx_withoutSBJson`, 

在`Build Phases` -> `Compile Sources`中删除所有sbjson相关的文件:

```
$QUICK_COCOS2DX_ROOT/lib/cocos2d-x/external/extra/platform/ios/json/SBJSON.m
$QUICK_COCOS2DX_ROOT/lib/cocos2d-x/external/extra/platform/ios/json/SBJsonBase.m
$QUICK_COCOS2DX_ROOT/lib/cocos2d-x/external/extra/platform/ios/json/SBJsonWriter.m
$QUICK_COCOS2DX_ROOT/lib/cocos2d-x/external/extra/platform/ios/json/SBJsonParser.m
```

然后让i4链接`quickcocos2dx_withoutSBJson.a`即可!

[1]:http://www.cnblogs.com/wengzilin/p/3800856.html