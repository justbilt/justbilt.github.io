title: Quick-cocos2d-x 小米 Mix 全面屏适配
date: 2017-11-17 11:58:46
tags: [Quick-Cocos2d-x, Android]
---

有玩家反馈我们的游戏在 Mix 上面无法全屏显示, 上下会有黑边. 搜索了下, 网上还没有绍如何适配的相关文章, 这里介绍下我们是如何做的.

我们可以在看看小米官方[对全面屏适配工作的介绍][1]:

> 这些变化也影响了手机软件的设计，最值得开发者关注的，是以下两点：
> 1. 更大的屏幕高宽比
> 2. 虚拟导航键

<!--more-->

好的, 我们先按照官方的方法修改下 `max_aspect` 的值为 `2` 以上

```html
<meta-data android:name="android.max_aspect" android:value="2.1" />
```

运行下, 发现顶部的黑条不见了, 但是底部的虚拟导航键还在. 我们接着看官方的文档:

![][2]

然而这几种效果都不是我们想要的, 我们想要是隐藏`虚拟导航键`. Google 一下, 很容易搜索到这段代码:

```java
 /**
     * 隐藏虚拟按键，并且全屏
     */
    protected void hideBottomUIMenu() {
        //隐藏虚拟按键，并且全屏
        if (Build.VERSION.SDK_INT > 11 && Build.VERSION.SDK_INT < 19) { // lower api
            View v = this.getWindow().getDecorView();
            v.setSystemUiVisibility(View.GONE);
        } else if (Build.VERSION.SDK_INT >= 19) {
            //for new api versions.
            View decorView = getWindow().getDecorView();
            int uiOptions = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_FULLSCREEN;
            decorView.setSystemUiVisibility(uiOptions);
        }
    }
```

加上之后发现导航键真的没有了, 但是当你点击屏幕时, 它又出现了. 继续搜索发现了[这篇文章][3], 按照文章中的提示进行修改你的 AppActivity :

```java
    private Cocos2dxGLSurfaceView glSurfaceView;
    protected void onCreate(Bundle savedInstanceState, SplashEnum splash, long duration, boolean debug) {
        ...
        View decorView = getWindow().getDecorView();
// Hide both the navigation bar and the status bar.
// SYSTEM_UI_FLAG_FULLSCREEN is only available on Android 4.1 and higher, but as
// a general rule, you should design your app to hide the status bar whenever you
// hide the navigation bar.
        int uiOptions = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN;
        decorView.setSystemUiVisibility(uiOptions);
        ...
    }
    @Override
    public Cocos2dxGLSurfaceView onCreateView() {
        glSurfaceView = super.onCreateView();
        this.hideSystemUI();
        return glSurfaceView;
    }
    @Override
    public void onWindowFocusChanged(boolean hasFocus)
    {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus)
        {
            this.hideSystemUI();
        }
    }
    private void hideSystemUI()
    {
        // Set the IMMERSIVE flag.
        // Set the content to appear under the system bars so that the content
        // doesn't resize when the system bars hide and show.
        glSurfaceView.setSystemUiVisibility(
                Cocos2dxGLSurfaceView.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | Cocos2dxGLSurfaceView.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | Cocos2dxGLSurfaceView.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        | Cocos2dxGLSurfaceView.SYSTEM_UI_FLAG_HIDE_NAVIGATION // hide nav bar
                        | Cocos2dxGLSurfaceView.SYSTEM_UI_FLAG_FULLSCREEN // hide status bar
                        | Cocos2dxGLSurfaceView.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
    }
```


搞定. 看下最终的效果:

![][4]

[1]: https://dev.mi.com/doc/p=10083/index.html
[2]: https://dev.mi.com/doc/wp-content/uploads/2017/06/%E8%99%9A%E6%8B%9F%E9%94%AE%E7%9A%84%E6%A0%B7%E5%BC%8F.png
[3]: https://segmentfault.com/a/1190000002936799
[4]: https://ws2.sinaimg.cn/large/006tNc79gy1flncnl0u3tj30ku11278w.jpg
