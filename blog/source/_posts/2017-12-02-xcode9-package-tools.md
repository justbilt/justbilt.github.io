title: iOS 打包工具在 XCode9 下的改动
date: 2017-12-02 13:26:04
tags:
---

相信大家都会用到 iOS 的自动打包工具, 而不是每次都是通过 XCode 的 Archive 后手动导出的, 那样效率低下不说还增加出错的可能.

我们团队也是如此, 用 Python 搞了一套打包工具, 这套工具的核心是 XCode 附带的命令行工具 `xcodebuild`, 网上其他开源的打包工具也都大同小异. 这套工具自诞生以来一直运行的很稳定, 累计为我们打了数百个包, 为团队节省的时间估计也在数千分钟了.

<!--more-->

# 一. 无法导出 ipa

但是在更新 XCode9 之后, 工具挂掉了, 在导出包的时候会提示:

```sh
error: exportArchive: "wobwopr.app" requires a provisioning profile with the Push Notifications feature.
Error Domain=IDEProvisioningErrorDomain Code=9 ""wobwopr.app" requires a provisioning profile with the Push Notifications feature." UserInfo={NSLocalizedDescription="wobwopr.app" requires a provisioning profile with the Push Notifications feature., NSLocalizedRecoverySuggestion=Add a profile to the "provisioningProfiles" dictionary in your Export Options property list.}
```

简单 Google 了下, 也有其他打包工具也遇到了类似的错误:

[fastlane: https://github.com/fastlane/fastlane/issues/9589][1]

还有这些文章:

[New export options Plist in Xcode 9][2]
[Xcode9命令行新打包方法][3]


总结了下, 错误的原因是这个样子的, 通过 `xcodebuild` 导出 ipa 包时需要传入参数 `exportOptionsPlist` 指定导出配置, XCode9 之前这个最简单的配置是这个样子的:


```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>method</key>
    <string>{method}</string>
</dict>
</plist>
```

method 可选的值有: 

+ app-store
+ ad-hoc
+ package
+ enterprise
+ development
+ developer-id
+ mac-application

而 XCode9 之后最小化的配置变成了:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>method</key>
    <string>{method}</string>
    <key>provisioningProfiles</key>
    <dict>
        <key>{bundleid}</key>
        <string>{provision}</string>
    </dict> 
</dict>
</plist>
```

新增了 `provisioningProfiles` 字段, 它是一个由 `包名:配置文件名字或udid` 组成的字典. 那么我们改如何获取这些参数呢 ?


## 1. 使用 XCode 导出后保存

这个是目前网上提供的最多的解决方案, 具体步骤如下:

1. 使用 XCode 打开你的项目, `Projuct/Archive` 归档工程. 或者直接用 XCode 打开你的 `xcarchive` 文件.
2. 点击 `Export` 选择 `App Store/Development` 后下一步, 选择证书和配置文件后导出.
3. 在导出文件夹中找到 `ExportOptions.plist`, 这个里面的有你想要的参数.


这个方法相对简单, 却不是完美的解决方案, 它存在这么两个缺点:

1. 没法自动来做这件事情, 测试/发布得搞两次, 每个新的项目都要做一遍.
2. 项目证书发生变化后又得重新来一遍.

想到以后每个项目都要手动来搞, 那打包工具的意义有何在呢 ?


## 2. 从 xcodeproj 中读取

![][4]

既然我们已经在 XCode 中配置过了这些参数, 那我们能否从 XCode 工程文件中读取到这写参数呢 ? 让我们先打开一个工程寻找一下:

```
PRODUCT_BUNDLE_IDENTIFIER = com.xxx.xxx;
PROVISIONING_PROFILE = "74c78746-9aa9-4a37-9533-a991f42b7f58";
PROVISIONING_PROFILE_SPECIFIER = "xxx-dev";
```

哈哈, 不费吹灰之力便找到了. 下一步就是如何在代码中读取这些参数了? 我们的打包脚本是使用 python 写的, 因此我选择 [pbxproj][6] 这个库来解析 pbxproj 文件:

```python
from pbxproj import XcodeProject
project = XcodeProject.load("path/of/your/pbxproj")
for configuration in project.objects.get_configurations_on_targets("{target}", "{configuration}"):
    bundleid = configuration.buildSettings.PRODUCT_BUNDLE_IDENTIFIER
    specifier = configuration.buildSettings.PROVISIONING_PROFILE_SPECIFIER
    break
```

`target` 是 XCode 中 target 列表中的名字, `configuration` 值可选 `Debug/Release` . 我们很轻易的便获取到了包名和配置文件的名字, 对打包脚本稍作修改便导出成功了.

在使用 `pbxproj` 库的过程中我还遇到了一个小问题, 在给作者[提出 Issue ][5]后很快便收到了解决方案, 原来是我的测试文件 `test.py` 触发了一个 py3 兼容库的 bug. 

# 二. XCode9 不再生成 .xcscheme 文件

使用 `xcodebuild` 生成 xcarchive 文件时需要指定 `-scheme` 参数, 而不是 `-target`, 不然会提示:

```sh
xcodebuild: error: The flag -scheme is required when specifying -archivePath but not -exportArchive.
```

`scheme` 和 `target` 从名字上有一定的联系, 第一次打开一个 XCode 工程时, 会在 `.xcodeproj` 文件夹下生成多个和 target 一一对应的 `xcscheme` 文件:

```
xcuserdata/*.xcuserdatad/xcschemes/xxx.xcscheme
```

在 XCode9 前如果没有这个文件是无法生成 `xcarchive` 文件的, 为了能够做到持续集成, 我们的做法是: 

> 如果检测到这个文件不存在的话, 就用 `open xxx.xcodeproj` 命令打开这个工程, 这样就会自动生成这个文件了

而在 XCode9 中, 已经不再生成 `xcscheme` 文件了, 取而代之的是一个 `xcschememanagement.plist` 文件了. **而且重要的是即时这个文件不存在, 也是能够正常生成 xcarchive 文件的**.

那么在 XCode9 中我们之前检测 xcscheme 文件是否存在并生成的逻辑就不需要了, 我们判断一下 XCode 版本就行啦.

```python
from distutils.version import LooseVersion
def is_xcode9():
    out, err = subprocess.Popen("xcodebuild -version", shell=True, stdout=subprocess.PIPE).communicate()
    verison = out.strip().replace("\n", " ").split(" ")[1]
    return LooseVersion(verison) >= LooseVersion("9.0.0")
```


# 三. allowProvisioningUpdates 自动更新配置文件

让我们先来看看它的描述:

> Allow xcodebuild to communicate with the Apple Developer website. For automatically signed targets, xcodebuild will create and update profiles, app IDs, and certificates. For manually signed targets, xcodebuild will download missing or updated provisioning profiles. Requires a developer account to have been added in Xcode's Accounts preference pane.

XCode 管理证书和配置文件有两中方式: `automatically` 和 `manually`, 如过在 XCode 中勾选了 `Automatically manager signing` 的话, 归档出来的包就是 automatically 的, 反之则是 manually 的.

只有 automatically 类型的配置指定 `allowProvisioningUpdates` 才有用. 这个参数在归档和导出阶段都可以用到, 前提是你得在 XCode 的账号设置中登录开发者账号.

在归档阶段, 比较可能是缺少 `Development` 证书, 如果没有这个参数, 你会收到这个错误:

```
Code Signing Error: No signing certificate "iOS Development" found:  No "iOS Development" signing certificate matching team ID "G76Y748JJ2" with a private key was found.
Code Signing Error: Code signing is required for product type 'Application' in SDK 'iOS 11.1'
** ARCHIVE FAILED **
```

而指定后, 理想情况下是会自动帮你创建或下载证书, **但现在可能不够完善, 没能做到这点**, 现在会遇到如下错误:

```sh
2017-12-03 16:08:22.837 xcodebuild[20785:1078389]  DVTAssertions: Warning in /Library/Caches/com.apple.xbs/Sources/IDEFrameworks/IDEFrameworks-13532/IDEFoundation/Provisioning/Logging/IDEProvisioningLedger.m:167
Details:  Unable to close provisioning ledger entry because not all of its subentries are closed
Object:   <IDEProvisioningLedgerEntry: 0x7fa9540a4a90>
Method:   -closeWithError:
Thread:   <NSThread: 0x7fa9540b8740>{number = 15, name = (null)}
Please file a bug at http://bugreport.apple.com with this warning message and any useful information you can provide.
=== BUILD TARGET wobwopr OF PROJECT newship_wobwopr WITH CONFIGURATION Release ===
Code Signing Error: Revoke certificate:  Your account already has a signing certificate for this machine but it is not present in your keychain. To create a new one, you must first revoke the existing certificate.
Code Signing Error: No signing certificate "iOS Development" found:  No "iOS Development" signing certificate matching team ID "G76Y748JJ2" with a private key was found.
Code Signing Error: Code signing is required for product type 'Application' in SDK 'iOS 11.1'
** ARCHIVE FAILED **
```

如果是在导出阶段, 签名阶段的, 最有可能的是缺少 `Distribution` 配置文件, 一般情况下这个文件是需要手动在开发者后台创建并下载的. 但是如果指定了这个参数的话 xcodebuild 会帮你自动创建一个, 很神奇的是这个 Profile 在开发者后台是看不到的, 也不会影响到你之前手动创建的配置文件.

总之, 这是一个很棒的特性, 期待苹果继续完善它.

---

以上就是我们目前为止遇到的 XCode9 的 xcodebuild 问题, 如果大家也遇到了其他的问题, 欢迎一起讨论.

[1]: https://github.com/fastlane/fastlane/issues/9589
[2]: https://blog.bitrise.io/new-export-options-plist-in-xcode-9
[3]: https://htge.github.io/osx/ios/2017/09/21/xcode9-export-ipa/
[4]: https://ws3.sinaimg.cn/large/006tKfTcly1fm3foq44n2j30jh0a2my7.jpg
[5]: https://github.com/kronenthaler/mod-pbxproj/issues/185
[6]: https://github.com/kronenthaler/mod-pbxproj