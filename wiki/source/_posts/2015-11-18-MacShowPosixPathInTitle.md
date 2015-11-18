---
title: "Mac 显示当前浏览的文件夹名称"
categories: MacOS
date: 2015-11-18 16:11:46
---

我们在使用MAC时，Finder栏默认只显示当前浏览的文件夹名称，而没有显示访问路径，这个问题该怎么解决呢？

![][1]


## 开启:

```
defaults write com.apple.finder _FXShowPosixPathInTitle -bool TRUE;killall Finder
```


## 关闭:

```
defaults delete com.apple.finder _FXShowPosixPathInTitle;killall Finder
```


[1]: /image/52-1501291II6142.jpg