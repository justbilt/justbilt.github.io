title: "停止手打所有cpp文件到android.mk"
date: 2013-07-18 09:35:52
tags:
categories: [cocos2d-x]
---

<address>**前言:"懒"在这里当然不是贬义词,而是追求高效,拒绝重复劳动的代名词!做一个懒cocos2d-x程序猿的系列文章将教会大家在工作中如何偷懒,文章篇幅大多较短,有的甚至只是几行代码,争取把懒发挥到极致!**</address>
<!--more-->
### **一.懒人说书**

Android.mk中LOCAL_SRC_FILES需要罗列出所有参与编译的文件,这样在.cpp文件少的时候还可以一个一个添加,当有几百个文件的时候会十分的痛苦!

我们下看看TestCpp工程中的Android.mk文件:

[![QQ截图20130717200015](/images/6e98f48d235d657fc34a9c5ad87e0c71a419da1e.png)](http://blog.justbilt.com/wp-content/uploads/2013/07/QQ截图20130717200015.png)



这只是节选的一部分,大概只有50个左右吧,除数量多之外让我们看看下面的情况:

1改变了其中一个文件名就得修改mk文件,痛苦之处显而易见,而且会重新编译整个工程!

2.手工添加时因为失误多了空格,少了\之类事情很常见,得重新编译后才能发现问题!



### 二.进击的懒人

是不是不能忍受了!让我们试着改变下吧!

1.写个脚本自动变量里Classes文件夹下的所有.cpp文件,生成和上面类似的Android.mk文件.这个比手动添加要方便好多,但还是没有从根本上解决问题!



2.有木有办法在Android.mk中做手脚,不用罗列所有的.cpp文件呢?直到我看到了这篇文章:

[http://blog.csdn.net/qq634416025/article/details/8904466](http://blog.csdn.net/qq634416025/article/details/8904466)
```c++
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := hellocpp_shared

LOCAL_MODULE_FILENAME := libhellocpp

FILE_LIST := hellocpp/main.cpp
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../Classes/*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes

LOCAL_WHOLE_STATIC_LIBRARIES := cocos2dx_static

include $(BUILD_SHARED_LIBRARY)

$(call import-module,cocos2dx)
```
这样就OK了,不用手打所有的.cpp文件了!



但这这个做法还是有缺陷,就是如果Classes有子文件夹的话还是得在添加上去的,如下:
```c++
#FILE_LIST += $(wildcard $(LOCAL_PATH)/../../Classes/*.cpp)
#FILE_LIST += $(wildcard $(LOCAL_PATH)/../../Classes/datapacker/*.cpp)
#FILE_LIST += $(wildcard $(LOCAL_PATH)/../../Classes/platform/*.cpp)
```
这个样子的写法已经很高端了,有新的子文件的时候添加下就好,但是对于懒到极致的人还是不能接受!



3.终极解决方案

上边的解决方案用到了wildcard这个关键子,虽然不明白这是神马东西,但肯定可这个有关!于是经过一番google之后,便有了下面的这个:
```c++
# 遍历目录及子目录的函数
define walk
    $(wildcard $(1)) $(foreach e, $(wildcard $(1)/*), $(call walk, $(e)))
endef

# 遍历Classes目录
ALLFILES = $(call walk, $(LOCAL_PATH)/../../Classes)

FILE_LIST := hellocpp/main.cpp
# 从所有文件中提取出所有.cpp文件
FILE_LIST += $(filter %.cpp, $(ALLFILES))

LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes
```
哈哈哈,这样我们只用在项目最开始的时候改变Android.mk文件就以后再也不用碰啦!



附我的Android.mk文件,遇到问题可以参考下:

[Android](http://blog.justbilt.com/wp-content/uploads/2013/07/Android.txt)





7月23日更新:

感谢子龙大大^_^的补充:

如果classes目录下面有子目录的话，include路径是不对的。

可以再添加FILE_INCLUDES := $(shell find $(LOCAL_PATH)/../../Classes -type d)

然后LOCAL_C_INCLUDES := $(FILE_INCLUDES) 就可以了。

详细可见子龙大大的Android.mk脚本:

[https://gist.github.com/andyque/6060595](https://gist.github.com/andyque/6060595)



Ps:因为我都是在程序内部去加相对目录的,所以我的mk也是没有错的!如下:
```c++
#include "datapacker/Dpk.h"
```




(全文完)