title: 在MacOS上搭建mafiagame版的quick-cocos2d-x
date: 2014-01-26 18:15:44
tags: [quick-cocos2d-x]
---

# 一.Why mafiagame?

为什么要用mafiagame版的quick-cocos2d-x(以下简称quick)呢?我在使用quick时候遇到了很多坑,改了quick好多代码,主要还是cocosbuilder(以下简称ccb)方面的,为了方便其他同事,就在quick的基础上做了一个分支,提交我的这些修改,**修改基于quick的最新2.2.1-rc**,关于具体的坑会单独开一篇文章来写,这里就不做赘述了.
<!--more-->

# 二.克隆仓库到本地
Ps:如果你比较熟悉git和github的话,这部分可以跳过了,没有什么技术含量.


### 1.工具选择

+ **Github For Mac**,好多git的新手会选择使用Github For Mac,我不太推荐这么做,因为Github的客户端隐藏了好多东西,非常容易误操作并且对git的使用一点帮助也没有,

+ **git command tool**,我也不推荐一上来就使用git的命令行工具,一来上手难度太大,二来效率太低.

+ **SourceTree**,在使用SourceTree之前我用过gitGUI,感觉差距太大了,SourceTree绝对是Mac上最好用的git GUI工具,没有之一.下面我详细讲述一下使用SourceTree获取最新代码

### 2.安装SourceTree
工具大家可以从[这里][1]获取,下载完成后安装打开,该同意的同意,能跳过的跳过,大家就能见到这个画面了:
![][2]

忽略我上面的东西,点击左上角有`` + ``号字样的第一个图标**添加仓库**,就会看到下面这个画面:

在地址栏填入[https://github.com/mafiagame/quick-cocos2d-x.git][3]这个地址,选择clone到``/Users/xxxx/Documents/quick-cocos2d-x``这个路径下,书签名设置为``quick-cocos2d-x``,推荐打开下面的高级选项,在brunch框中填入``mafiagame``,这样可以只迁出mafiagame分支,减少clone的时间:



漫长的等待时间结束后,双击书签``quick-cocos2d-x``就可以进入这个画面了,进到这里就结束了克隆代码的步骤就结束了.
![][5]

### 3.切换到mafiagame分支
如果你上步在brunch框中填入了``mafiagame``,这步就可略过了,但是如果没有,就需要切换分支了.

1. 点击工具栏的``Checkout``
2. 在弹出框中点击``Checkout New Branch``
3. 选择远端的``origin/mafiagame``分支
4. 点击``OK``,等待,切换成功,步骤图如下

![][12]


# 三.搭建Sublime开发环境
关于Sublime的简介大家可以自行搜索,我们用它作为开发环境的主要原因是[lonewolf][4]小伙伴为Sublime做了非常实用的插件**QuickXDev**,非常屌,大家可以慢慢体会.
### 1.下载安装
大家在[这里][6]可以很轻松的获取Sublime的最新版本,安装后打开,可以看到这个画面:
![][7]
### 2.安装package control组件
1. 按``Ctrl+` ``调出console
2. 粘贴以下代码到底部命令行并回车：
**
import urllib2,os;pf='Package Control.sublime-package';ipp=sublime.installed_packages_path();os.makedirs(ipp) if not os.path.exists(ipp) else None;open(os.path.join(ipp,pf),'wb').write(urllib2.urlopen('http://sublime.wbond.net/'+pf.replace(' ','%20')).read())
**
3. 等待一段时间
4. 重启Sublime Text 2。
5. 如果在Perferences->package settings中看到package control这一项，则安装成功。

### 3.安装QuickXDev
1. 按``⌘+Shift+P``键打开``package control``,在弹出的框中输入``Install Package``,选择对应条目回车.
2. 稍等片刻后会弹出另一个输入框,输入``QuickXDev``回车,等待安装成功.
3. 重启Sublime,在``Perferences->package settings``中看到QuickXDev这一项，则安装成功。
4. 打开``Perferences->package settings ->QuickXDev ->QuickXDev.sublime-settings``,在打开的文本中
输入``quick_cocos2dx_root``,如下图:
![][8]

### 4.运行示例项目
1. 按``⌘+k,b``键打开左侧Side Bar
2. 在Finder中找到``quick-cocos2d-x``目录,拖拽到Side Bar上
3. 在Side Bar上展开``quick-cocos2d-x -> samples -> coinflip``,在``scripts``文件夹上右键``Run With Player``,如下图:
![][9]
4. 然后就会看到官方的这个示例了,下面开始lua之旅吧

![][10]



### 四.后记
这里并没有讲Android环境的搭建,是因为quick官网上已经有了详细的教程了,大家去看[这篇][11]文章就OK,至于为什么不给官方提交``pull request``,有以下几个原因:

1. 我的代码习惯,提交规范和quick不知有无差异
2. 我做的更改多数在ccb方面,quick官方明显排斥ccb
3. 我添加了一些工具类,比较个人化,推广开来可能不适用

综上,我没有提交给quick仓库,但是如果quick有需要的话,我还是十分乐意合并过去的.


### (全文完)






[1]:http://sourcetreeapp.com/
[2]:http://ww1.sinaimg.cn/large/7f870d23jw1ecx5w2og63j20ho0jcgmf.jpg
[3]:https://github.com/mafiagame/quick-cocos2d-x.git
[4]:http://my.oschina.net/lonewolf/blog/176266
[5]:http://ww1.sinaimg.cn/large/7f870d23jw1ecx7e1oywgj21720rk7di.jpg
[6]:http://www.sublimetext.com/
[7]:http://ww2.sinaimg.cn/large/7f870d23jw1ecx7lzwmtxj20jg0gjdge.jpg
[8]:http://ww2.sinaimg.cn/large/7f870d23jw1ecx868doczj20re0gj402.jpg
[9]:http://ww1.sinaimg.cn/large/7f870d23jw1ecxu1qcduzj20c80dddh5.jpg
[10]:http://ww4.sinaimg.cn/large/7f870d23jw1ecxu86hshsj20kw0kljw9.jpg
[11]:http://cn.quick-x.com/?p=415
[12]:http://ww1.sinaimg.cn/large/7f870d23jw1ecxuv0vkj6j20ta08uju7.jpg


