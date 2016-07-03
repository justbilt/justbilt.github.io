title: 最近搞 iOS 版遇到的一些问题和技巧 (二)
date: 2016-06-26 11:38:44
tags: iOS
description: "万一遇上了呢?"
---


## 一. XCode: Could not find Developer Disk Image
![101138-1ab6ab96d37904bd.jpeg-4.5kB][1]
**解决方案:**
[http://www.jianshu.com/p/3930df903a44][2]
这个问题可能是因为你 XCode 没有下载对应 iOS 的 SDK 导致, 一般情况需要同步更新 XCode.


## 二. XCode: 无法导出 Archive 的项目
这个问题有可能是你项目 Team 选择的是一个没有开发者资格的账号导致的, 虽然可以正常开发, 真机调试, 但是是不能发布的, 所以也无法 Export .
**解决方案:**
1. 更换一个有开发者资格的账号重新 Archive 导出, 但是 Bundle ID 就得换一个了.
2. 通过命令行工具导出.
在 Organizer 中找到你想导出的 Archive, 右键选择在文件夹中显示, 复制路径, 打开终端:
```
xcodebuild -exportArchive -exportFormat ipa -archivePath your-archive-file-name.xcarchive -exportPath ~/Desktop/test.ipa
```

## 三. XCode: App Installation failed
![Unknown.png-45.1kB][3]
很可能是之前手机已经装过一个同 Bundle ID 的应用, 但是现在换了签名.
**解决方案:**
删掉手机已经安装的那个应用就可以啦.

## 四. XCode: Failed to code sign "xxxx"
![QQ20160626-0.png-437.7kB][4]
签名失败了, 这种情况一般发生在使用别人给的证书打包时. 这时候我们项目 `Build Setting > Code Signing Identity` 就不能选择 `iOS Developer`, 而是要选择导入的签名文件.

## 五. 内购: 无法连接到 iTunes Store
如果没有发布应用的话, 需要用沙盒测试账号来测试. 我们需要先在 `设置>iTunes Store 和 App Store`中 **注销账号**, 然后打开游戏, 开始购买, 这时候输入你的测试账号. 成功后如果有跳转 App Store 的话或者绑定付款方式的话, 不同理会, 再返回应用购买就可以了.

## 六. 崩溃: showAlert 崩溃
某一次突然, 一旦调用 quick-x 提供的 `device.showAlert` 就会崩溃, 断点调试无果, 崩溃时提示的内容也不尽相同.
**解决方案:**
删除 `RootViewController.mm` 中所有和屏幕方向代码, 就解决啦.

```objc
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
- (NSUInteger) supportedInterfaceOrientations
- (BOOL) shouldAutorotate
- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
```

## 七. 崩溃: iOS 9.2+ 播放视频崩溃

感谢 @子龙山人 大神提供的解决方案, [点击这里查看][5].

将 `UIVideoPlayer-ios.mm` 文件 `~VideoPlayer()` 函数中的 `dealloc` 修改为 `release` 即可.

```objc
-        [((UIVideoViewWrapperIos*)_videoView) dealloc];
+        [((UIVideoViewWrapperIos*)_videoView) release];
```

  [1]: http://static.zybuluo.com/justbilt/84lvyg8n11lmtefi02pirqup/101138-1ab6ab96d37904bd.jpeg
  [2]: http://www.jianshu.com/p/3930df903a44
  [3]: http://static.zybuluo.com/justbilt/kl9sahl74sqfnxgjem57f390/Unknown.png
  [4]: http://static.zybuluo.com/justbilt/eb80q3re93fflmiuhygk4f1m/QQ20160626-0.png
  [5]: https://github.com/cocos2d/cocos2d-x/issues/14855