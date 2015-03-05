title: 盘点 quick-cocos2d-x Player 粗线的问题
date: 2014-05-17 12:04:10
tags: CCClipingNde
categories: [quick-cocos2d-x]
---

## 一. Mac版日志显示不完整

不知道大家有没有遇到这样的问题,在输出大量log的时候,Player的日志窗口有时会只输出一部分内容,后面补上`...`, 而且之后的log就再也无法输出了, 对后面的调试造成很大的不便.

<!--more-->

打开Player的工程,搜索`...`,可以定位到 `ConsoleWindowController.m` 的 trace 函数,让我们看下这个代码片段:

```c++
if (traceCount >= SKIP_LINES_COUNT && [msg length] > MAX_LINE_LEN)
{
    msg = [NSString stringWithFormat:@"%@ ...", [msg substringToIndex:MAX_LINE_LEN - 4]];
}

traceCount++;
```

大家把这几行注释掉重新编译运行即可解决这个问题.

## 二. Mac版 CCClippingNode 无效

又到了做新手引导的时候了,之前发过两篇关于新手引导的文章:

[cocos2d-x-游戏实战经验三-多分辨率的自适应(上)][1]
[cocos2d-x-游戏实战经验三-多分辨率的自适应(下)][2]

看过的朋友应该知道,想实现屏幕中某一部分高亮需要用到CCClipingNde, 这是一个非常实用的功能类.这设置stencil之后会抠空stencil的内容

但是当我在**quick-cococs2d-x**中用player中运行**CCClipingNde**时,却发现没有任何的变化.


网上搜索 `CCClippingNode无效` 会找到几篇文章,都一直的指向OpenGL初始化的地方:

![][3]

试着用xocde启动项目工程,意外的发现**CCClipingNde**在ios的模拟器是好使的,这样看来问题是出在了Player的身上,打开quick的player工程,不难发现player实质上就是在cocos2d-x的Mac版的基础上做成的,这样的话造成这个问题的原因不外乎有两个:

1. cocos2d-x 的Mac不支持 CCClipingNde 
2. quick-cocos2d-x 的Playe 的openGL初始化有问题

运行官方2.2.1版cocos2d-x的mac项目,发现CCClippingNode是没有问题的,正常运行.

在对比两个项目的AppController.mm发现了不一致的地方:

```c++
// player
glView = [[EAGLView alloc] initWithFrame:rect];

//coco2d-x for mac
NSOpenGLPixelFormatAttribute attributes[] = {
	NSOpenGLPFADoubleBuffer,
	NSOpenGLPFADepthSize, 24,
	NSOpenGLPFAStencilSize, 8,
	0
};
NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
glView = [[EAGLView alloc] initWithFrame:rect pixelFormat:pixelFormat];
```

用官方的实现替换Player的实现,编译运行,一切正常,下面上张图!

![][4]

## 未完

暂时只遇到了这两个问题,都比较容易解决,以后遇到问题还会在这里补充!


[1]:http://post.justbilt.com/2013/07/12/cocos2d-x-%E6%B8%B8%E6%88%8F%E5%AE%9E%E6%88%98%E7%BB%8F%E9%AA%8C%E4%BA%8C-%E6%96%B0%E6%89%8B%E5%BC%95%E5%AF%BC%E4%B8%8A/
[2]:http://post.justbilt.com/2013/08/02/cocos2d-x-%E6%B8%B8%E6%88%8F%E5%AE%9E%E6%88%98%E7%BB%8F%E9%AA%8C%E4%B8%89-%E5%A4%9A%E5%88%86%E8%BE%A8%E7%8E%87%E7%9A%84%E8%87%AA%E9%80%82%E5%BA%94%E4%B8%8B/
[3]:http://ww2.sinaimg.cn/large/7f870d23jw1egh5qaaxw7j20i70gngnc.jpg
[4]:http://ww1.sinaimg.cn/large/7f870d23jw1egh68oa3ahj206q08iaa6.jpg