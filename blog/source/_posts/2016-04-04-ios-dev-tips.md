title: 最近搞 iOS 版遇到的一些问题和技巧
date: 2016-04-04 09:13:34
tags: iOS
description: "运行 iOS 版你需要知道的一些事情, 进来看看不吃亏!"
---

# 一. 编译运行

## 1. 运行时提示 identity 无效

![][1]

> The identity used to sign the executable is no longer valid.
Please verify that your device’s clock is properly set, and that your signing certificate is not expired. (0xE8008018).

解决方案:

1. 打开 Preferences > Accounts
2. 选中项目对应的 Apple ID > 点击右下角 `View Details...`
3. 点击弹出框左下角的 `Download All` > 等待完成后点击 `Done` 关闭
4. 再次运行项目, 等待弹出框, 点击 `reset`.

![][2]

## 2. iPhone5 上获得的设备尺寸为 960x640

表现出来是竖屏游戏上线有黑边, 跟踪发现获得的设备尺寸为 960x640, 而非 1136x640.

解决方案:

添加一张尺寸为 `1136x640` 名为 `Default-568h@2x.png` 的启动图即可.

参考资料:
[http://discuss.cocos2d-x.org/t/getframesize-get-wrong-screen-size/7657][4]

## 3. XCode issue 页面只显示错误, 不显示警告

cocos2d-x 在 XCode 中编译 warning 太多, 出错后 error 会被淹没在一大堆的警告中, 得拖动半天才能找得到.

解决方案:

![][5]

在页面最下方有一个感叹号型按钮, 点击选中即可.

# 二. 上传应用

## 1. 多任务支持

> XCode 7 error: “A launch storyboard or xib must be provided unless the app requires full screen”

解决方案:

我们勾上全屏即可:
![][3]


---

目前就这些, 后面会持续更新 !

[1]: http://ww2.sinaimg.cn/large/7f870d23gw1f2ker4f0hhj20nc08eabu.jpg
[2]: http://ww1.sinaimg.cn/large/7f870d23gw1f2khakkr86j20ja0a0go3.jpg
[3]: http://ww1.sinaimg.cn/large/7f870d23gw1f2khkqmob7j20b809it9f.jpg
[4]: http://discuss.cocos2d-x.org/t/getframesize-get-wrong-screen-size/7657
[5]: http://ww2.sinaimg.cn/large/7f870d23gw1f2kib70c4qj207500mt8h.jpg



