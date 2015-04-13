---
title: "AndroidStudio-下载组件慢"
categories: Android
date: 2015-02-02 10:33
---

第一次启动AndroidStudio回下载组件, 特别慢,在`/etc/hosts`文件添加下面几行即可:

```
#更新的内容从以下地址下载
203.208.46.146 dl.google.com
203.208.46.146 dl-ssl.google.com
```

![](/image/2015-01-30-4.12.10.png)