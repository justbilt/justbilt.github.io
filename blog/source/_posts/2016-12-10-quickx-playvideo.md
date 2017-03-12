title: quick-x 视频播放
date: 2016-12-10 22:00:28
tags:
- quick-cocos2d-x
- videoplayer
description: "半成品, 别用!"
---

今天我们来聊聊 QuickX 中播放视频的那些事. 

这篇文章来自于日常的笔记, 年代可能会有些久远, 加上当时最开始视频播放不是我来做的, 所以有些地方我的理解也不是很深刻. 若是有什么不对的地方, 还望大家不吝赐教.

# 一. 基本用法

播放一段视频:

```lua
    local video = ccexp.VideoPlayer:create()
    video:setPosition(cc.p(display.cx, display.cy))
    video:setAnchorPoint(cc.p(0.5, 0.5))
    video:setContentSize(cc.size(display.width, display.height))
    video:setFileName("res/start_video.mp4")
    video:setKeepAspectRatioEnabled(true)
    video:setFullScreenEnabled(true)

    self:addChild(video)
```

监听视屏播放事件:

```lua
    local function onVideoEventCallback(sender, eventType)
        print("onVideoEventCallback:", eventType)
        if eventType == ccexp.VideoPlayerEvent.PLAYING then
        elseif eventType == ccexp.VideoPlayerEvent.PAUSED then
        elseif eventType == ccexp.VideoPlayerEvent.STOPPED then
        elseif eventType == ccexp.VideoPlayerEvent.COMPLETED then
        end
    end
    video:addEventListener(onVideoEventCallback)
    video:play()
```

> 然而你做完这一切还是可能会播放不出来视频, 没有关系, 多播放几次就出来啦.

# 二. 遇到的问题

## 问题 1: iOS 播放视频完后黑屏

cocos2dx 3.3 iOS端播放视频完后黑屏, 控制台中提示日志: OpenGL error 0x0506 in -[CCEAGLView swapBuffers] 324

解决方案:

http://www.cnblogs.com/cc4coco/p/4188347.html

```objc
-(void) viewDidAppear:(BOOL)animated{

    cocos2d::Director::getInstance()->resume();
    cocos2d::Director::getInstance()->startAnimation();
}

- (void)viewWillDisappear:(BOOL)animated{

    cocos2d::Director::getInstance()->pause();
    cocos2d::Director::getInstance()->stopAnimation();
}
```

## 问题 2: 在 iOS 9.2 以上会崩溃

解决方案:

https://github.com/cocos2d/cocos2d-x/issues/14855

修改 VideoPlayer 类的析构函数, 将 `dealloc` 改为 `release`.

```java
VideoPlayer::~VideoPlayer()
{
     if(_videoView)
     {
-        [((UIVideoViewWrapperIos*)_videoView) dealloc];
+        [((UIVideoViewWrapperIos*)_videoView) release];
     }
}
```

## 问题 3: iOS 上播放视频有进度条, 双击可以放大缩小, Android 点击可以暂停

解决方案:

这个问题挺让人啼笑皆非的, 不知道当时设计这个类的人是如何考虑的, 至少也应该提供一个关闭的接口吧. 好, 让我们看下如何解决它.

![](http://ww3.sinaimg.cn/large/006y8lVajw1fam4s9vv7ij30dj0m9q3f.jpg)

### 1. 隐藏 iOS 播放进度条

在 UIVideoPlayer-ios.mm 的 `-(void) setURL:(int)videoSource :(std::string &)videoUrl` 函数中, 修改 `MPMovieControlStyleEmbedded` 为 `MPMovieControlStyleNone`:

```objc
-    self.moviePlayer.controlStyle = MPMovieControlStyleEmbedded;
+    self.moviePlayer.controlStyle = MPMovieControlStyleNone;
```

进度条隐藏了, 但是视频播放时双击缩放的问题却无法解决.

### 2. 禁用 Android 点击暂停

修改 `Cocos2dxWebView.java` 中的 `onTouchEvent` 函数

```java
    @Override
    public boolean onTouchEvent(MotionEvent event) {
//        if((event.getAction() & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_UP)
//        {
//            if (isPlaying()) {
//                pause();
//            } else if(mCurrentState == STATE_PAUSED){
//                resume();
//            }
//        }
        
        return true;
    }
```


## 问题 4: vivo手机无法播放视频的bug

解决方案:

经过断点跟踪, 定位到了原因, 视频的尺寸获取的有问题, 我尝试修改了 `setVideoRect` 函数中的两行代码:

```java
-        mViewWidth = maxWidth;
-        mViewHeight = maxHeight;
+        mViewWidth = 1;
+        mViewHeight = 1;
```

很神奇的解决了这个问题.

## 问题 5: 多次调用可能会遮挡 SDK 的弹出界面

我们游戏中有一个重启的功能, 每次重启都会播放一个视频, 这样某些渠道的登录界面就看不到了, 另一个不播放视屏的项目就没有问题, 好像是每次播放都会提高游戏的层级.

解决方案:

遍历所有 window , 找到 SDK 的那个 window, 将它的 windowLevel 提高一级.

```objc
    for(UIWindow * w in [[UIApplication sharedApplication]windows]){
        if ([w isKindOfClass:NSClassFromString(@"XSDKOriginalWindow")]) {
            [w setWindowLevel:[[[UIApplication sharedApplication]keyWindow]windowLevel]+1];
            break;
        }
    }
```


## 问题 6: 还有什么问题, 都说了吧

如果播放时候是黑屏，把游戏切到后台，再进入游戏就能从头播放！
如果播放时候正常，切到后台再切回来就变成黑屏

http://www.cocoachina.com/bbs/read.php?tid-306892.html

Cocos2d-x V3.10版本中的videoplayer问题
http://www.voidcn.com/blog/sh15285118586/article/p-5989468.html

关于cocos2dx 3.x VideoPlayer的问题
http://blog.sina.com.cn/s/blog_93add5520102w6n9.html

cocos2d-x视频控件VideoPlayer的用户操作栏进度条去除
http://blog.csdn.net/pklll000pp44/article/details/51337577


---

还真的是问题一大堆呢! 大家最好还是回去和策划大大商量下, 别播放视频了.