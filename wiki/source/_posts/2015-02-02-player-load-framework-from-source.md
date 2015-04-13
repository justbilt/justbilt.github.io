---
title: "Player从源码加载framework"
categories: quick-x
date: 2015-02-02 10:33
---

默认player是从预编译`framework_precompiled.zip`包加载framework的, 这样做会提高运行效率,但是也会带来一些不便:

#### 1. framework的错误无法命中行号:

```
stack traceback:
	[C]: in function 'addChild'
	[string ".../quick-cocos2d-x-2.2.5//framework/shortcodes.lua"]:0: in function 'add'
```

如上, 无论错误在哪里, 都会显示`0` 行.

#### 2. 修改framework后得重新打预编译包, 无法做到实时, 出错后又无法准确定位.

这就需要我们从源码加载framework.

打开`$QUICK-COCOS2D-X/player/sources/AppDelegate.cpp`, 找到:

```c++
if (projectConfig.isLoadPrecompiledFramework())
{
    const string precompiledFrameworkPath = SimulatorConfig::sharedDefaults()->getPrecompiledFrameworkPath();
    pStack->loadChunksFromZIP(precompiledFrameworkPath.c_str());
}
```

修改为:

```c++
if (projectConfig.isLoadPrecompiledFramework())
{
    const string quickCocos2dxRootpath = SimulatorConfig::sharedDefaults()->getQuickCocos2dxRootPath();
    pStack->addSearchPath(quickCocos2dxRootpath.c_str());
}
```