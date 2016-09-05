title: 最近搞 iOS 版遇到的一些问题和技巧 (三)
date: 2016-09-05 23:50:12
tags: iOS
description: "万一遇上了呢?"
---


# 一. iOS 内购返回商品无效 invalid product

我使用 quick-x 内置的 store 类请求商品信息时, 收到这样的错误:

> nvalidProductIdentifiers [CCStore_obj] 
[CCStore_obj] productsRequestDidReceiveResponse() invalid pid: com.xxx.xxx

首先, **检查你请求的商品 id 在 iTunes 后台是否创建**, 是否拼写错误. 如果没有问题, 那么就不太好办了, 会有很多原因导致这个问题.

ZZB_Amoy 博客的[这篇文章][1]总结了下可能的原因, 如下:

>+ 创建的App ID是否启用了IAP功能。
+ 商品信息是否配置到iTurn Connect，并到达“Ready to Submit”状态。
+ 在iTurn Connect中创建Test User，并收取邮件激活。之后登录到测试用手机的设置页面中（Store选项）。
+ App的Bundle Id是否和后台配置的App Id一致。
+ 是否创建相应的provisioning profile，并用此签名App。
+ iTurn Connect后台配置完商品信息后，是否等待若干小时生效。
+ SKProductsRequest请求的商品Id必须和iTurn Connect中配置的一致。（如：com.test.product.xxx）
+ iTunes Connect中配置的银行信息是否正确。
+ 是否先删除旧App，再重新编译生成新的。
+ 请不要使用越狱手机测试。

下面说说我两次遇到这个问题的解决方案:

1. 如果游戏发布区域中没有手机中 App Store 当前区的话, 需要先登陆下对应区域创建的测试账号, 将商店切换到对应区域.
2. 完善苹果开发者账号所能完善的信息, 如付款信息呀什么的, 然后莫名其妙就解决了.


# 二. iOS 运行崩溃 unrecognized selector sent to instance

运行游戏过程中收到如下错误:

> [1515:710439] -[AppController window]: unrecognized selector sent to instance 0x2c85c00
libc++abi.dylib: terminate_handler unexpectedly threw an exception

这个在接入某一个平台 sdk 时遇到的问题, 于是便问了下他们的技术, 很快解决了问题. 

#### 1. 修改 AppController.h 中 window 变量的声明形式

![][2]

#### 2. 修改 AppController.mm

![][3]

虽然问题解决了, 但是我并不明白各种缘由. Google 了下, 大概明白了, 原来如此. 从错误中我们可以看到这句 `[AppController window]` , 从语法来看, 这是要调用 AppController 的 window 函数, 但是在我们之前的写法中没有实现这个函数, 便出错了. 而使用 `@property` 这个东西, 会自动帮你实现一个 `window/setWindow` 函数, 这样就不会找不到这个函数了.



[1]: http://blog.sina.com.cn/s/blog_a6a46b330101dgju.html
[2]: http://ww2.sinaimg.cn/large/7f870d23gw1f7j62j0mfuj20dw02n3yy.jpg
[3]: http://ww2.sinaimg.cn/large/7f870d23gw1f7j665yyacj207901zq32.jpg

