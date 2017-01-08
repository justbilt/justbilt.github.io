title: cocos2d-x 抖动效果 CCShake
date: 2013-09-11 15:37:58
categories:
- cocos2d-x
---

之前在网上找过一次,但是用起来有些问题,抖动完位置有偏移,就自己修改了下,原文应该是在这里:

<!--more-->

[http://blog.csdn.net/qq634416025/article/details/8630189][1]

感兴趣的同学可以去看一下,作者特意将抖动做成了CCAction,用起来十分方便,大谢!



效果图(GIF,画质压缩的有点狠,凑活着看吧!):

![667][2]



我把我修改过的版本放在了百度云盘上,下载地址:

[http://pan.baidu.com/share/link?shareid=2640816246&uk=2685725110][3]

使用时:

**1.先将CCShake.h中的**
```c++
#include "GameDefine.h"
```
 注释掉,改为下面这段话:
```c++
#include "cocos2d.h"
USING_NS_CC;
```


**2.用法:**
```c++
pSprite->runAction(CCShake::create(0.1f,10));
```
 pSprite:想抖动的物体

第一个参数是:抖动的时间

第一个参数是:抖动的幅度

**3.注意,这点曾经困扰了我好久!**

一个CCNode同时执行多个CCShake动作,或者一个CCShake没有完又执行一个CCShake的话就会出现问题,会出现偏移的现象!

解决方案:

1. 不要同时执行多个CCShake动作.
2. 自己外部记录这个CCNode的位置,执行完成后手动setPosition();




[1]: http://blog.csdn.net/qq634416025/article/details/8630189
[2]: /img/6527eb984895fca57fd5c7fad00e31b07b7966eb.gif
[3]: http://pan.baidu.com/share/link?shareid=2640816246&uk=2685725110