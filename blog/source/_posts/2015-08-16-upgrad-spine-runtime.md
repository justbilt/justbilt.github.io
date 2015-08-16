title: "更新 spine 的 cocos2d-x runtimes"
date: 2015-08-16 19:17:03
tags:
---

又是2个多月没有更新博客了, 真是不应该, 惭愧啊！其实手上已经攒了一些素材了, 只是一直没有一个很强的动力去写出来, 今天总算是有一些想法了, 先用一篇没有什么养分的文章来 “探探路”,  唤醒下自己写作的欲望吧！

---

# 1. why ?

最近的项目中用到 spine, 但是美术在spine中做的效果却与程序中得效果有些出入。比如在spine中设置翻转, 在程序中却是没有的。看了下导出的json文件, 里面是有 `flipX` 字段的, 于是猜想是没有解析的原因.

在spine的解析代码中搜索了一下, 果然是没有找到解析`flip`字段的代码的, 看来是需要更新下解析的代码了! spine 的解析库是放在 github 上的, 地址在这里 [https://github.com/EsotericSoftware/spine-runtimes][1].

{% github EsotericSoftware spine-runtimes 7259586 true 25% %}

# 2. How ?

1. 首先我们 clone `spine-runtimes` 到本地的一个目录.
2. 复制 `spine-runtimes/spine-c/include/spine/` 目录下所有文件到 `cocos2d-x/cocos/editor-support/spine/`
3. 复制 `spine-runtimes/spine-c/src/spine/` 目录下所有文件到 `cocos2d-x/cocos/editor-support/spine/`
4. 复制 `spine-runtimes/spine-cocos2dx/3/src/spine/` 目录下所有文件到 `cocos2d-x/cocos/editor-support/spine/`
5. 编译, 运行, 然后就没有然后了!

---

好吧, 这个升级远我想象的简单好多, 没有编译错误, 没有运行崩溃, 还真是令人有些不习惯, 只能说 spine 他们还是蛮认真的!




[1]: https://github.com/EsotericSoftware/spine-runtimes