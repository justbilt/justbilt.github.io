---
title: "quick-x 纹理重复"
categories: quick-x
date: 2015-02-02 10:33
---


代码:

```lua
	local sprite = display.newSprite("img/test.png")
	local tp = ccTexParams()
    tp.minFilter = 9729
    tp.magFilter = 9729
    tp.wrapS = 10497
    tp.wrapT = 10497
    sprite:getTexture():setTexParameters(tp)
	self.sprite:setTextureRect(CCRect(0, 0, dis, size.height))
```

效果:

![img](/image/QQ20150129-1.jpg)