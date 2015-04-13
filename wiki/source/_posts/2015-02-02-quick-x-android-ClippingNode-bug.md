---
title: "quick-x android 开启深度缓冲"
categories: quick-x
date: 2015-02-02 10:33
---

ClippingNode在一些Android平台上显示会出现显示不正确,该问题的解决方案如下：

打开`quick-cocos2d-x-2.2.5/lib/cocos2d-x/cocos2dx/platform/android/java/src/org/cocos2dx/lib/Cocos2dxGLSurfaceView.java`文件:


修改`Cocos2dxGLSurfaceView`的构造函数为:

```java
	public Cocos2dxGLSurfaceView(final Context context) {
		super(context);

		this.initView();

		this.setEGLConfigChooser(5, 6, 5, 0, 16, 8); // add this
	}
```