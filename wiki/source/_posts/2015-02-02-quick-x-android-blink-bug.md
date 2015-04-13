---
title: "quick-x android 闪屏 bug"
categories: quick-x
date: 2015-03-17 11:38:05
---

讨论贴在这里[这里][1],感谢[@偶尔e网事][2]同学.

在接入 sdk 的时候, 上层显示一个 sdk 的界面时, 整个屏幕就会快速闪烁, 十分影响体验.

解决方案,打开

`$QUICK-COCOS2D-X/lib/cocos2d-x/cocos2dx/platform/android/java/src/org/cocos2dx/lib/Cocos2dxActivity.java`:

1.将 `onResume` 的`this.mGLSurfaceView.onResume();`实现改为:

```java
	public void onResume() {
		super.onResume();

        this.setRenderMode(RENDERMODE_CONTINUOUSLY);
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleOnResume();
            }
        });
//		this.queueEvent(new Runnable() {
//			@Override
//			public void run() {
//				Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleOnResume();
//			}
//		});
	}
```

2.将 `onPause` 的实现改为:

```java
	@Override
	public void onPause() {
//		this.queueEvent(new Runnable() {
//			@Override
//			public void run() {
//				Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleOnPause();
//			}
//		});
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleOnPause();
            }
        });
        this.setRenderMode(RENDERMODE_WHEN_DIRTY);
		//super.onPause();
	}
```

[1]: http://www.cocoachina.com/bbs/read.php?tid-222173.html
[2]: http://weibo.com/GeekJacky