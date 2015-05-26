---
title: "ipa build 失败"
categories: iOS
date: 2015-05-26 18:12
---

# 1. Check dependencies

构建失败, 提示如下:
```
** BUILD FAILED **


The following build commands failed:
	Check dependencies
(1 failure)
```

> 解决方案:

查看 provisioning prefiles 是否正确, 存在问题则 点击 `Fix Issue`:
![][1]

# 2. Ld /Users/xxx/XXX.app/xxx normal armv7

构建失败, 提示如下:
```
** BUILD FAILED **


The following build commands failed:
	Ld /Users/zxsk/Library/Developer/Xcode/DerivedData/dota-bvifnumrevheojggvqgtmxhudglx/Build/Products/Release-iphoneos/真三国OL.app/真三国OL normal armv7
(1 failure)
** ARCHIVE FAILED **
```

> 解决方案:

编译失败, 在 XCode 中尝试下吧!


[1]: /image/QQ20150526-1.jpg