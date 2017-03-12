title: quickx addTo导致tag无效坑
date: 2014-09-11 19:59:57
tags:
- Quick-Cocos2d-x
---

前几天遇到了一个十分诡异的bug,分享出来大家看看,代码是这个样子的:

```lua
local a = display.newNode()
local b = display.newNode()
b:setTag(10)
b:addTo(a)
print(b:getTag())
```
<!--more-->

这个程序正常的结果应该是`10`,但实际却是`0`.这个问题十分诡异,如果是`-1`的话还说的过去,但`0`是个什么情况嘛?


仔细想想,大家应该很快能把异常的位置定位到`b:addTo(a)`这行中,让我们看下下`addTo`函数的实现:

```lua
function CCNodeExtend:addTo(target, zorder, tag)
    target:addChild(self, zorder or 0, tag or 0)
    return self
end
```

不难看出,当你调用addTo函数不传入`zorder, tag`时会自动帮你设置为`0`!

知道了原因,解决起来十分容易:

1. 使用`addChild`函数代替`addTo`
2. 在`addTo`调用时传入`tag`
3. 在`addTo`调用之后调用`setTag`函数


是不是很简单呢? 这主要是测试代码比较简单的原因,我当时的 setTag 和 addTo 都没有在一个文件里!!!找死我了!!

平复下心情,我们来想想造成这个bug的原因:

首先, `shortcodes`系列的函数设计的初衷可能是为了连续的调用函数,不建议分开单独使用! 但是我觉得最主要的原因是addTo函数的实现是不够健壮的,如果参考C++ addChild函数的话应该是这样子:

```lua
function CCNodeExtend:addTo(target, zorder, tag)
    target:addChild(self, zorder or self:getZOrder(), tag or self:getTag())
    return self
end
```


以上