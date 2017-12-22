title: 用 python 撸一个 ccbwriter
date: 2017-12-17 12:50:25
tags: [Python, CocosBuilder]
---

之前可能和大家说过我们项目的 UI 编辑器是上古神器 [CocosBuilder][1], 这个编辑器诞生在那个 cocos2d-x 还需手写 ui 代码的年代, 是 cocos 发展史上一个重要转折点. 它标志着触屏手机游戏行业 ui 不再像之前那样只是简单的串联的作用, 而是像端游那样重交互, 可视化编辑的路线.

<!--more-->

尽管在今天, CocosBuilder 想比于已经死掉的 CocosStudio, 甚至于 Cocos 现在大力推广的 CocosCreator 都有一些难以替代的理由, 尤其是在开源和稳定性方面. 我们也不例外, 现在仍有多款项目在使用这个编辑器, 单个项目 CocosBuilder 的界面 ccb 文件数量最高在 500+, 这对于维护这些界面的美术同学是一个很大的挑战. 为了不让可爱的美术妹子疲于奔命, 我们程序组自然要贡献我们的一份力量, 写一些批处理的脚本, 解决那些无脑但是会大量重复的工作, 拯救妹子们与水火之中.

在写脚本的过程中, 不免要和 ccb 文件打交道, 这种文件的典型格式是这样的:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <string>CocosBuilder</string>
    <key>fileVersion</key>
    <integer>4</integer>
    <key>guides</key>
    <array>
        <dict>
            <key>position</key>
            <real>-2346</real>
        </dict>
    </array>
    <key>jsControlled</key>
    <false/>
    <key>nodeGraph</key>
    <dict>
        <key>baseClass</key>
        <string>CCNode</string>
        <key>children</key>
        </array>
        <key>customClass</key>
        <string></string>
        <key>displayName</key>
        <string>CCNode</string>
    </dict>
</dict>
</plist>
```


在搞 ccb2lua 项目时我曾写过一个 ccbreader 组件, 把 ccb 文件当做 xml 来解析, 一行是 `key`, 下一行是 `value`. 直到在 Mac 上工作的久了, 才知道这种特殊的 xml 是一种名为 [Property List][2] 的文件.

# 一. 使用 python plistlib 库解析

而作为我司第一语言的 Python 自然也有对应的解析库 [plistlib][3], 这是一个内置的库, 不需要安装直接就能使用, 用法也很简单.

```python
import plistlib
plist = plistlib.readPlist("/path/of/you/plistfile")
print plist
plist["somekey"] = "some value"
plistlib.writePlist(plist, "/path/of/you/another/plistfile")
```

上面就是一个最简单的读取修改写入 plist 文件的例子, 但是令人沮丧的是即便我什么都不做, 但是读取再写入整个文件也会发生很多的变动:

```diff
@@ -18,7 +18,7 @@
                        <key>position</key>
-                       <real>-2346</real>
+                       <real>-2346.0</real>
@@ -33,7 +33,8 @@
                                <key>children</key>
-                               <array/>
+                               <array>
+                               </array>
@@ -62,8 +63,8 @@
                                                <string>ScaleLock</string>
                                                <key>value</key>
                                                <array>
-                                                       <real>1</real>
-                                                       <real>1</real>
+                                                       <real>1.0</real>
+                                                       <real>1.0</real>
                                                </array>
```

主要发生在这么几个地方:

## 1. 浮点数精度问题

```xml
<!-- 原始 -->
<real>-2346</real>
<!-- 变为 -->
<real>-2346.0</real>

<!-- 原始 -->
<real>941.91412353515625</real>
<!-- 变为 -->
<real>941.9141235351562</real>
```

## 2. 空的 array 和 dict

```xml
<!-- 原始 -->
<array/>
<!-- 变为 -->
<array>
</array>

<!-- 原始 -->
<dict/>
<!-- 变为 -->
<dict>
</dict>
```

虽然这些变动不会影响该文件的再次读取, 并且当在 CocosBuilder 再次保存时又会还原回去, 但是这会造成两个很严重的问题:

1. 影响其他成员 review 更改, 无法识别出真正的变动
2. 增加合并时产生冲突的可能

尤其是第二点, ccb 文件一旦冲突, 基本很难解决, 只能选择放弃一方的修改, 这个损失是无法容忍的. 要解决这个问题, 我们可以有多种解决方案:

1. 像当时写 ccbreader 一样完全手撸一个 ccbwriter
2. 看下 plistlib 是否可以定制下解析器和写入代码


# 二. 改造 plistlib

经过一番考虑, 我选择了第二种方案, 官方文档上对 plistlib 说明很少, 我们只能从它的源码入手. 我电脑上它位于这个位置:

```
/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/plistlib.py
```

我们重点看 `readPlist` 和 `writePlist`:

```python
def readPlist(pathOrFile):
    didOpen = 0
    if isinstance(pathOrFile, (str, unicode)):
        pathOrFile = open(pathOrFile)
        didOpen = 1
    p = PlistParser()
    rootObject = p.parse(pathOrFile)
    if didOpen:
        pathOrFile.close()
    return rootObject

def writePlist(rootObject, pathOrFile):
    didOpen = 0
    if isinstance(pathOrFile, (str, unicode)):
        pathOrFile = open(pathOrFile, "w")
        didOpen = 1
    writer = PlistWriter(pathOrFile)
    writer.writeln("<plist version=\"1.0\">")
    writer.writeValue(rootObject)
    writer.writeln("</plist>")
    if didOpen:
        pathOrFile.close()
```

这里面分别用到了 `PlistParser` 和 `PlistWriter` 作为解析和写入工具类, 我们完全继承自这两个类写一个自定义 `Parser` 和 `Writer`, 处理掉之前的特殊情况.

```python
class CCBReal:
    def __init__(self, value):
        self.value_raw = value
        self.value = float(value)

    def float(self):
        return self.value

    def raw(self):
        return self.value_raw


class CCBParser(plistlib.PlistParser):
    def end_real(self):
        value = self.getData()
        self.addObject(CCBReal(value))

class CCBWriter(plistlib.PlistWriter):
    def writeValue(self, value):
        if isinstance(value, CCBReal):
            self.simpleElement("real", value.raw())
        else:
            plistlib.PlistWriter.writeValue(self, value)

    def writeArray(self, array):
        if len(array) <= 0:
            self.writeln("<array/>")
        else:
            plistlib.PlistWriter.writeArray(self, array)

    def writeDict(self, d):
        if len(d) <= 0:
            self.writeln("<dict/>")
        else:
            plistlib.PlistWriter.writeDict(self, d)
```

对空的 `dict` 和 `array` 处理很简单, 判断下长度就可以啦. 但是对 `real` 的处理就费了一番周折.

我一开始想着用 python 中的高精度类型来保存 real 的值, 但是过程十分曲折, 数值完全对不上, 各种修改测试依然不行. 绝望临近放弃之际, 突然灵光一现, 可以用一个复杂的类型同时记录下原始的值 (string 类型) 和转化为 float 中的值, 这样如果在整个程序运行的过程中没有改变这个值的话就可以把原始值写入进去, 这样就不会有任何意料之外的更改了.

全部代码已经放到[我的 gist 上][4]了, 有需要可以自取.


```python
plist = readPlist("/path/of/you/ccb")
print plist.guides[0].position
print plist.guides[0].position.float()
plist.guides[0].position = 5.5
print plist.guides[0].position
writePlist(plist, ccb)
```

测试代码当然少不了啦, 感觉自己棒棒哒, 晚饭申请加个🍗. 

[1]: https://github.com/cocos2d/CocosBuilder
[2]: https://zh.wikipedia.org/wiki/%E5%B1%9E%E6%80%A7%E5%88%97%E8%A1%A8
[3]: https://docs.python.org/2/library/plistlib.html
[4]: https://gist.github.com/justbilt/cc21b7c6573bac7668282faa56dfdd9b