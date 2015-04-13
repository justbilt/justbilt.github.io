---
title: "ios-去掉icon上部高光"
categories: iOS
date: 2015-02-02 10:33
---

1.以source形式打开项目中的info.plist,在里面添加上这段代码:

```xml
<key>icon already includes gloss and bevel effects</key>
<true/>
```


2.勾选xcassets的`iOS icon is pre-rendered`

![](/image/QQ20150130-1.jpg)

3.删除iphone/itouch上已经存在的app

4.项目做一次build clean ，或者直接点，删除项目所在文件夹中的build文件夹