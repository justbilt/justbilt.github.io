title: 友盟渠道包工具
date: 2014-03-19 10:18:22
tags: [umeng,apk]
categories: [Tool]
---

游戏做完了,在国内发android包是一件很痛苦的事情,应用市场多达100多家,不说都上吧,就是上10来个也够咱们喝一壶的了,而且每次更新版本都得这么做:(,想想还是应该有个批量打包的工具!

<!--more-->

## 1.问题
之前在微博上看到王峰的一张表,里面大概有百八十个渠道,后来找不到了,只找到了下面这张图:

![国内部分应用商店][1]

实现自动化的思路的大概有两个:

1. 使用ant工具,写编译脚本
2. 打出一个Apk包,使用拆包工具分解Apk,修改``AndroidManifest.xml`` 文件后，再重新打包

## 2.下载安装

友盟渠道打包工具就是按照第二个思路实现的,官方github仓库在[这里][2],windows版的下载地址在[这里][3],现在貌似只有windows7版的(偷乐)!

如果你恰好接入了友盟的统计,那么就可以爽了!

下载完成后,解压,运行``UmengTools.exe``后会检测Java环境,需要安装JDK,可以去[这里][4]下,然后把类似这个路径``C:\Program Files\Java\jdk1.7.0_45\bin``加入到环境变量中去.

## 3.使用

启动**UmengTools**后你就会看到一个非常高大上的界面:

![][5]

然后点击1位置的扳手图标,会弹出配置界面,如图:

![][6]


1. KeyStore中填入你签名文件的路径,password栏填入创建签名文件时的密码
2. Alias栏填入这个应用的名称,password栏填入密码
3. 要添加,删除渠道在渠道栏配置
4. 在配置文件栏填入配置文件名称
5. 点击保存


然后将你通过eclipse导出的签名包拖拽到2的位置,点击一键打包可以冲杯咖啡等待成功了!



[1]:http://ww2.sinaimg.cn/large/7f870d23jw1eekv0hmikaj20dw06vaam.jpg
[2]:https://github.com/umeng/umeng-muti-channel-build-tool
[3]:https://github.com/umeng/umeng-muti-channel-build-tool/blob/master/Downloads/UmengTools(Green)V2.1.zip
[4]:http://www.oracle.com/technetwork/java/javase/downloads/jdk7-downloads-1880260.html
[5]:http://ww1.sinaimg.cn/large/7f870d23jw1eekxijxt9tj20m80go0u2.jpg
[6]:http://ww1.sinaimg.cn/large/7f870d23jw1eekxppu1iej20iw0egmyn.jpg