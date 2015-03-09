title: "基于CCScrollView的quickx自动布局"
date: 2015-03-08 09:41:03
tags: quick-cocos2d-x
---

> UI的自动布局一定是很多人追求的目标, 这里和大家分享下我在这方面的尝试, 同时也是抛砖引玉, 希望能有更好更完美的解决方案.

一般自动布局需要整个UI系统的支持, 如果要改造, 工作量很大. 所以我打算从CCScrollView入手, 实现一个能自动布局的滚动视图. 让我们先看下效果:

![][1]

这是一个类似排行榜的演示, 让我们看一下核心代码:

```lua
local function createRankItem()
  self.index = self.index + 1
  local item = display.newScale9Sprite("diban.png")
    :align(display.CENTER)
    :size(display.width*0.7+math.random(0,100), 50+math.random(0,50))

  ui.newTTFLabel({text = string.format("第%d名 王五",self.index)})
    :pos(item:getContentSize().width/2,item:getContentSize().height/2)
    :addTo(item)
  return item
end

-- 创建自动布局
local rank = AutoLayout.new()
  :pos(display.cx, display.cy)
  :addTo(self,1)

-- 设置
rank:setDirection(kCCScrollViewDirectionVertical)
rank:setViewSize(CCSize(display.width*0.8, display.height*0.8))
rank:setIgnoreAnchorPointForPosition(false)

-- 添加节点
for i=1,5 do
  rank:push(self:createRankItem())
end

-- 自动布局
rank:layout()		

```

可以看到, 在创建好自动布局之后, 只需调用`push`和`layout`即可, 完全不用设关心每一个节点的位置.


[1]: /img/QQ20150308-1.jpg