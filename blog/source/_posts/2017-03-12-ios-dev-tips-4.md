title: 最近搞 iOS 版遇到的一些问题和技巧 (四)
date: 2017-03-12 10:25:47
tags:
- iOS
- iOS-Dev-Tips
description: "万一遇上了呢?"
---

这是这半年搞 iOS 时遇到的一些问题, 写下来, 希望会帮到大家. 因为时间精力有限, 有些问题, 并没有解决方案, 只是饶了过去.

## 一. iOS 沙盒测试 无法连接到APPSTORE

确定是测试机, 沙盒账号登录的也没有问题, 就是无法充值.

### 解决方案

换了一个测试机就好了


## 二. xcodebuilder -exportArchive dev 失败

这个问题发生在我们的 iOS 打包脚本中, 会报错:

> IDEDistribution: Step failed: <IDEDistributionThinningStep: 0x7fa72bbd50e0>: Error Domain=IDEDistributionErrorDomain Code=14 "No applicable devices found."

### 解决方案

一开始我使用了老的 ipa 导出方案 `xcrun -sdk iphoneos PackageApplication` , 这样虽然可以导出了, 但是经常无法安装到测试机, 还得用 XCode 打开归档文件手动导出 dev 版本. 就这样凑活了一段时间后, 我终于下定决心解决这个问题了. 原来每次 `exportArchive` 会有有一个日志文件, 查看 /var/folders/…/IDEDistribution.standard.log 得知是我的 ruby 环境有问题:

```md
[NOTE]
You may have encountered a bug in the Ruby interpreter or extension libraries.
Bug reports are welcome.
Don't forget to include the above Crash Report log file.
For details: http://www.ruby-lang.org/bugreport.html
```

在 StackOverflow 上找到了[这个解决方案][1], 使用 `xcbuild-safe.sh` 替换 `xcodebuilder` 就好了


## 三. You must supply a CFBundleIdentifier for this request

上传 dis 包时遇到这个错误:

![][2]

### 解决方案

在[这篇文章][3]中找到了解决方案:

>infor.plist表中Bundle OS Type code 栏为空或者非APPL
具体设置路径为，项目的TARGETS >> infor >> Custom iOS Target Properties >> Bundle OS Type code，检查是否为空或者其他。将该选项设置为 APPL。自此，重新上传成功。

虽然解决了, 但是比较奇怪的是, 我好多项目没有设置也可以上传成功.


## 四. 连接 VPN 后有的 http 请求会失败

这个很诡异呀, 不连接 VPN 没有任何问题.

### 解决方案

我在 Exception Domain 添加对应域名之后就可以访问了. 猜测是不是果爹对大陆 ip 放宽了要求 ? 哈哈哈

## 五. This action could not be completed.Try again.(-22421)

还是上传 dis 包到 iTunes 后台的时候, 会遇到这个错误:

![][4]

### 解决方案

在[这篇文章][5]中找到了解决方案:

> 暂时请使用 -Application Loader上传app程序

## 六. iOS 沙盒测试充值一次成功, 一次失败

在沙盒测试充值时, 这次失败了, 下次就会成功, 主要发生在小面额的充值项上.

### 解决方案

![][6]

苹果商店抽风, 没啥解决方案.

## 六. 苹果提交审核，因为应用名称不合法被拒，改完提交还是被拒

这个是苹果的回复:

>We still find that your app name to be displayed on the App Store includes descriptors, which are not appropriate for use in an app name.
Specifically, the following words in your app names are considered descriptors

### 解决方案

可以查看是否在iTunes connect 后台添加了多个语言，而只修改了一种语言。

## 七. XCode 安装应用出错

这个是错误内容:

> This application's application-identifier entitlement does not match that of the installed application. These values must match for an upgrade to be allowed.

### 解决方案

删除手机上已经安装的应用就好了

[1]: http://stackoverflow.com/questions/39634404/xcodebuild-exportarchive-no-applicable-devices-found/42027456#42027456
[2]: https://ww3.sinaimg.cn/large/006tNbRwly1fdjuz90b3cj30w40g2aar.jpg
[3]: http://www.jianshu.com/p/2d229dfb34a6
[4]: https://ww2.sinaimg.cn/large/006tNbRwly1fdjv6l9qzij30dh038wep.jpg
[5]: http://www.jianshu.com/p/a9f818ac1066
[6]: https://ww2.sinaimg.cn/large/006tNbRwly1fdjvcfmifbj31as0foq6c.jpg