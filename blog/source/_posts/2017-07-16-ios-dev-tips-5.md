title: 最近搞 iOS 版遇到的一些问题和技巧 (五)
date: 2017-07-16 11:36:20
tags:
- iOS
- iOS-Dev-Tips
description: "万一遇上了呢?"
---

这段时间又积攒下了不少的技巧和解决方案, 感觉 XCode 越来越牛逼了, 尤其是在应用的账号设置方面, 之前遇到的一些低级问题可能已经不复存在了.

<!--more-->

# 此应用在未来的 iOS 版本下将无法使用

![][1]

## 解决方案:

发生这个问题的原因就是应用没有支持 arm64 CPU 架构, 不同的CPU架构对应的设备如下:

>arm64：iPhone6s | iphone6s plus｜iPhone6｜ iPhone6 plus｜iPhone5S | iPad Air｜ iPad mini2(iPad mini with Retina Display)
armv7s：iPhone5｜iPhone5C｜iPad4(iPad with Retina Display)
armv7：iPhone4｜iPhone4S｜iPad｜iPad2｜iPad3(The New iPad)｜iPad mini｜iPod Touch 3G｜iPod Touch4

一般出现这个问题, 很有可能是 **XCode Architectures 设置或导出包的方式有问题**:

![][2]

这里面有两个地方需要关注一下: `Build Avtive Architecture Only` 和 `Vaild Architectures`, 前者是为了加速真机运行的速度, 一般 debug 会设置为 YES, release 版会设置为 NO; 后者是应用所支持的 CPU 架构.

所以我们必须得保证 Vaild Architectures 中有 arm64, 并且打包出 release 版才可以, 我们遇到这个问题的原因是打包同学使用了 debug 版的 app 做成 ipa 安装包的原因.

---

# clang: error: no such file or directory: 'XXX'

造成这个错误的原因有好多, 这次的报错极其诡异, 连系统库都会报找不到.

## 解决方案:

经过排查, 发现是 `Other Link Flag` 中有一个单行的 `-force_laod`, 估计是删除的时候少删了一行.

---

# XCode 真机调试安装失败, 提示证书过期

![][3]

## 解决方案:

真机调试的时候一般使用的是开发证书, 开发证书可以有很多个, 在钥匙串中找到过期的证书删掉就可以啦, XCode 会自动匹配一个没有过期的证书.

---

# 导出 dis 包出错, IDEDistributionErrorDomain error 3

```sh
error: exportArchive: The operation couldn’t be completed. (IDEDistributionErrorDomain error 3.)
Error Domain=IDEDistributionErrorDomain Code=3 "(null)" UserInfo={IDEDistributionErrorSigningIdentityToItemToUnderlyingErrorKey={}
```

## 解决方案:

其实就是证书和 Provisioning Profiles 没有匹配上, 很有可能是其他小伙伴 revoke 了你的 dis 证书.

![][5]

去 [苹果开发者中心][4] 查看对应的 Profiles 的 status 是否正常, 修改完后下载运行一下, 然后重新导出就好了.

---

# 沙盒测试: 此时没有权限在Sandbox购买此InApp

其实描述的很清晰, 这个沙盒账号不是你这个应用的, 请仔细检查一下.

## 解决方案:

更换正确的沙盒测试账号.

---

# QQ: 一键加群跳转过去后群不正确

检查 LSApplicationQueriesSchemes 中是否有 `mqqapi`:

![][6]

## 解决方案:

如果没有的话添加一个就好啦.

---

# 导出 dev 包出错, IDEDistributionErrorDomain Code=14

查看终端日志:

```
/Users/justbilt/.rvm/bin/rvm: line 66: shell_session_update: command not found
+ xcodebuild -exportArchive -exportOptionsPlist /Users/justbilt/Documents/work/easy_slg_client/frameworks/runtime-src/proj.ios_mac/ios_export_dev.plist -archivePath /Users/justbilt/Documents/work/easy_slg_client/.build/yile.guopan-1.4.0.f592c19533-0607170734.xcarchive -exportPath /Users/justbilt/Documents/work/easy_slg_client/.build/0607192643399129
2017-06-07 19:26:43.906 xcodebuild[4724:25032817] [MT] PluginLoading: Required plug-in compatibility UUID E0A62D1F-3C18-4D74-BFE5-A4167D643966 for plug-in at path '~/Library/Application Support/Developer/Shared/Xcode/Plug-ins/XAlign.xcplugin' not present in DVTPlugInCompatibilityUUIDs
2017-06-07 19:26:44.068 xcodebuild[4724:25032817] [MT] IDEDistribution: -[IDEDistributionLogging _createLoggingBundleAtPath:]: Created bundle at path '/var/folders/ff/wpytbgmd3rl1pn9dd02nbjc80000gn/T/guopan_2017-06-07_19-26-44.068.xcdistributionlogs'.
2017-06-07 19:26:58.844 xcodebuild[4724:25032817] [MT] IDEDistribution: Step failed: <IDEDistributionThinningStep: 0x7fb80e6cb860>: Error Domain=IDEDistributionErrorDomain Code=14 "No applicable devices found." UserInfo={NSLocalizedDescription=No applicable devices found.}
error: exportArchive: No applicable devices found.
```

找到其中的 xcdistributionlogs 日志文件路径:

```
/var/folders/ff/wpytbgmd3rl1pn9dd02nbjc80000gn/T/guopan_2017-06-07_19-26-44.068.xcdistributionlogs
```

在 IDEDistribution.standard.log 文件中可以找到真正的错误原因:

```
guopan.app/guopan arm64 ->
error: Info.plist of “guopan.app/heepayImage.bundle” specifies a non-existent file for the CFBundleExecutable key
error: Info.plist of “guopan.app/walletResources.bundle/Contents” specifies a non-existent file for the CFBundleExecutable key
2017-06-07 11:08:24 +0000  Validating IPA structure...
2017-06-07 11:08:24 +0000 [MT] /Applications/Xcode.app/Contents/Developer/usr/bin/ipatool exited with 1
2017-06-07 11:08:24 +0000 [MT] ipatool JSON: {
    alerts =     (
                {
            code = 0;
            description = "Info.plist of \U201cguopan.app/heepayImage.bundle\U201d specifies a non-existent file for the CFBundleExecutable key";
            info =             {
            };
            level = ERROR;
            type = "malformed-payload";
        },
                {
            code = 0;
            description = "Info.plist of \U201cguopan.app/walletResources.bundle/Contents\U201d specifies a non-existent file for the CFBundleExecutable key";
            info =             {
            };
            level = ERROR;
            type = "malformed-payload";
        }
    );
}
```

## 解决方案:

可以看到这里有两个 Error, 意思就是这两个 .bundle 中的 `Info.plist` 中的 key `CFBundleExecutable` 字段指定了一个不存在的文件, 这个指定其实没有啥用, 我们删掉它就可以了. 

用 Sublime 打开这个 plist, 删除下面这两行:

```
    <key>CFBundleExecutable</key>
    <string>xxx</string>
```
如果你打开的文件是一团乱码, 那么这个文件应 2进制 形式的 plist, 我们可以用一个命令行工具把它转化为 xml 形式的:

```
plutil -convert xml1 xxx/heepayImage.bundle/Info.plist
```

然后删除掉.


[1]: https://ws1.sinaimg.cn/large/006tKfTcgy1fhlkn3of4wj316y0p4aku.jpg
[2]: https://ws2.sinaimg.cn/large/006tKfTcgy1fhlkyqgvv0j30jy02odg2.jpg
[3]: https://ws4.sinaimg.cn/large/006tKfTcgy1fhllu9g2m4j30ad03r3yo.jpg
[4]: https://developer.apple.com/account/ios/profile/
[5]: https://ws3.sinaimg.cn/large/006tKfTcgy1fhlmmvhb27j30ud0jajth.jpg
[6]: https://ws1.sinaimg.cn/large/006tKfTcgy1fhlmr1lvfkj30y00bcgno.jpg