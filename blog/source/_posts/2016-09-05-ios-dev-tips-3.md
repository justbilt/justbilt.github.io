title: 最近搞 iOS 版遇到的一些问题和技巧 (三)
date: 2016-09-05 23:50:12
categories:
- quick-cocos2d-x
- iOS
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

# 三. 游戏在低于 ios9 的系统启动崩溃

这个也是在接入第三方 sdk 时遇到的问题, 游戏一启动就会崩溃, 收到错误如下:

> dyld: Symbol not found: _OBJC_CLASS_$_SFSafariViewController
  Referenced from: /var/mobile/Applications/CF4146B4-3F79-4644-86CA-F19E52E64BAA/superarmoreddivision.app/superarmoreddivision
  Expected in: /System/Library/Frameworks/SafariServices.framework/SafariServices
 in /var/mobile/Applications/CF4146B4-3F79-4644-86CA-F19E52E64BAA/superarmoreddivision.app/superarmoreddivision

Google 了一下, 没有任何人遇到过这样的问题, 这就十分棘手了, 完全不知从何入手. 经过一番探索, 找到了几个有用的线索:

1. SFSafariViewController 这个类是 ios 9 才引入的, 这和我们已知的信息相符.
2. 所幸的是我们的游戏有多个 Scheme , 每个 Scheme 接入不同的 sdk . 其他的 Scheme 的都可以正常运行.

这就可以肯定是某个 sdk 中使用了 `SFSafariViewController` 这个类, 但是还是没有办法定位是那个 sdk . 我不知道是否有一个命令查找符号引用, 因此只能采用最笨的排除法了, 我将引入的 sdk 依次删除, 看是否能够运行.

最终定位到了某个广告统计 sdk , 在询问其 ios 技术人员后得到了解决方案. 原来他们 sdk 需要**以 `optional` 的形式引入 `SafariServices.framework`**.

![][4]

都怪我没有仔细阅读文档, 白白耽误了一段时间, 下次一定要注意!

# 四. Facebook 登录崩溃

集成 Facebook sdk 时, 调用登录接口游戏就会崩溃, 这个问题 Google 一下就能解决, 解决方案也很简单, 在 Info.plist 中加入下面几行代码即可:

```xml
<key>LSApplicationQueriesSchemes</key>
<array>
        <string>fbapi</string>
        <string>fb-messenger-api</string>
        <string>fbauth2</string>
        <string>fbshareextension</string>
</array>
```

Stackoverflow 上的答案可以[移步这里][5], Facebook 官网上也[给了解答][6].

# 五. ios9 状态栏无法隐藏

隐藏状态栏在 ios9 上换了一种方式, 还是需要在 Info.plist 中进行配置:

```xml
<key>UIStatusBarHidden</key>
<true/>
<key>UIViewControllerBasedStatusBarAppearance</key>
<false/>
```

Stackoverflow 上的答案可以[移步这里][7].

# 六. showAlert 诡异崩溃

游戏内的一些弹框为了保证在游戏的最上层显示, 偷懒使用了 quick-x 提供的 `device.showAlert` 接口. showAlert 内部使用 `UIAlertView` 实现, 运行一直良好, 有一天突然就不行了, 一调用就崩溃. 

各种办法都试过了, 网上都说是线程安全问题, 我试了一下各种处理都不行, 打断点跟踪到最底层也无济于事. 几近绝望之时, @bin 的一句话提醒了我:

> 会不是屏幕方向的问题 ? 

最终一番尝试, 删除了 `RootViewController.mm` 中几个屏幕方向相关的函数:

```objc
*/
// Override to allow orientations other than the default portrait orientation.
// This method is deprecated on ios6
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    
    if (ConfigParser::getInstance()->isLanscape()) {
        return UIInterfaceOrientationIsLandscape( interfaceOrientation );
    }else{
        return UIInterfaceOrientationIsPortrait( interfaceOrientation );
    }
    
}

// For ios6, use supportedInterfaceOrientations & shouldAutorotate instead
- (NSUInteger) supportedInterfaceOrientations{
#ifdef __IPHONE_6_0
    if (ConfigParser::getInstance()->isLanscape()) {
        return UIInterfaceOrientationMaskLandscape;
    }else{
        return UIInterfaceOrientationMaskPortraitUpsideDown;
    }
#endif
}

- (BOOL) shouldAutorotate {
    if (ConfigParser::getInstance()->isLanscape()) {
        return YES;
    }else{
        return NO;
    }
}
```

这个 bug 出现之诡异, 解决方案之诡异, 在我遇到的 bug 中也算是很少见了.


[1]: http://blog.sina.com.cn/s/blog_a6a46b330101dgju.html
[2]: http://ww2.sinaimg.cn/large/7f870d23gw1f7j62j0mfuj20dw02n3yy.jpg
[3]: http://ww2.sinaimg.cn/large/7f870d23gw1f7j665yyacj207901zq32.jpg
[4]: http://ww1.sinaimg.cn/large/7f870d23gw1f7kanujs5rj20ol018jrf.jpg
[5]: http://stackoverflow.com/a/33489214
[6]: https://developers.facebook.com/docs/ios/ios9
[7]: http://stackoverflow.com/a/32965748

