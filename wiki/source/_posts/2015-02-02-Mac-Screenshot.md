---
title: "Mac 截图"
categories: MacOS
date: 2015-02-02 10:33
---

首先说一下两种截图
```
1. ⌘＋shift＋3:全屏截图，保存截图到桌面
2. ⌘＋shift＋4:鼠标选定区域截图，保存截图到桌面
```

一
如果你同时按住Ctrl，最后保存截图在剪切板里，你可以CMD+V来直接粘贴到编辑界面
二
使用Command＋shift＋4后，按下空格键，鼠标会变成一个小相机，这时候你使用鼠标对一个窗口点击一下鼠标左键，你已经对一个窗口进行了截图。
三
按Command＋shift＋4 后 ,画一个抓取的区域，不要松开鼠标，接着

1. 按住空格可以移动这个区域
2. 按住 Shift后，将锁定X 或者 Y轴进行拖动
3. 按住 Option后 将按照区域圆心进行放大.

最后所有截图将直接显示在桌面上。

设置截图格式:
```
defaults write com.apple.screencapture type jpg/png/gif
killall SystemUIServer
```

设置截图保存路径:
```
defaults write com.apple.screencapture location /XXX/
killall SystemUIServer
```

设置截图文件名称:
```
defaults write com.apple.screencapture name XXXXX
killall SystemUIServer
```

截图是否含有阴影:
```
defaults write com.apple.screencapture disable-shadow -bool true
killall SystemUIServer
```