---
title: "quick-x windwos 字体抗锯齿"
categories: quick-x
date: 2015-02-02 10:33
---

同样的字体在windows下边缘都是锯齿, 非常难看.

可以使用[这篇][1]文章中的解决方案.

> 近几天在玩弄ccx的时候发现在win下的字体显示有严重的锯齿问题，拜读了下win平台下的字体解决方案，结果让我有些失望。ccx是采用win32 api将font画到DC上然后GetDIBits取出位图数据经过处理放到位图数组中，锯齿问题是因为它简化了边缘alpha的处理。


改完了文件在[这里][2],大家可以下载[^注1].


[^注1]: 当时cocos2d-x版本为2.1.2


[1]: http://my.oschina.net/gal/blog/141431
[2]: /image/CCImage.cpp