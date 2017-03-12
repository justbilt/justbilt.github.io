
title: 在 quick-x 中使用 TableView
date: 2016-03-13 08:02:53
tags:
- quick-cocos2d-x
- TableView
description: "如何使用 TableView 这个效率提升利器? "
---

上篇文章说到使用 TableView 可以大幅提升界面的创建速度, 这篇文章我们来看看如何在 quick 中使用它.

# 一. 基本用法

## 1. 创建对象

首先, 创建一个 TableView 对象:

```lua
local view = cc.TableView:create(cc.size(480,320))
```

传入的那个 size 是 viewsize, 即可视区域的尺寸, 后期也可以通过 `setViewSize` 来调节.

## 2. 设置填充顺序

下面设置填充顺序, 默认是从下往上填充, 我们习惯了使用从上往下填充, 所以需要修改下:
```lua
view:setVerticalFillOrder(cc.TABLEVIEW_FILL_TOPDOWN)
```

其可选值如下:
```lua
cc.TABLEVIEW_FILL_TOPDOWN = 0  -- 从上自下
cc.TABLEVIEW_FILL_BOTTOMUP = 1 -- 从下自上
```

这个值最终会影响所有 cell 的顺序, 具体些是第 0 个元素在最上面还是在最下面.

## 3. 注册事件监听

TableView 提供了好多事件, 具体作用如下:

```lua
cc.SCROLLVIEW_SCRIPT_SCROLL = 0
cc.SCROLLVIEW_SCRIPT_ZOOM   = 1
cc.TABLECELL_TOUCHED        = 2
cc.TABLECELL_HIGH_LIGHT     = 3
cc.TABLECELL_UNHIGH_LIGHT   = 4 
cc.TABLECELL_WILL_RECYCLE   = 5
cc.TABLECELL_SIZE_FOR_INDEX = 6 -- 获取节点尺寸的回调
cc.TABLECELL_SIZE_AT_INDEX  = 7 -- 获取对应位置节点的回调
cc.NUMBER_OF_CELLS_IN_TABLEVIEW = 8 -- 获取节点数量的回调
```

但其实真正有意义的就三个, 6,7,8. 其他的**根本不会回调**, 注册了也没有什么用. 注册监听使用 `registerScriptHandler` 函数.


### 1). 注册节点数量回调

```lua
view:registerScriptHandler(function()
    return 10 -- 假如有10个节点
end, cc.NUMBER_OF_CELLS_IN_TABLEVIEW)  
```

返回节点数量.

### 2). 注册获取节点尺寸的回调

若是每个节点一致, 则可以返回一个固定尺寸; 若不一致, 可以根据 idx 去取出对应节点的尺寸.

```lua
view:registerScriptHandler(function(table, idx)
    -- 默认尺寸
    local size = self.defaultSize
    local cell = view:cellAtIndex(idx)
    if cell then
        -- cell.view 属性是在线面创建 cell 时赋值的.
        size = cell.view:getBoundingBox()
    end
    return size.height, size.width
end, cc.TABLECELL_SIZE_FOR_INDEX)
```

注意, 这里如果是纵向滚动的话,返回顺序是高,宽; 横向滚动的话则返回宽,高. 


### 3). 获取对应位置节点的回调

这个回调的名称和上面那个很像, 有时候会分不清楚.

```lua
view:registerScriptHandler(function(table, idx)
    local cell = table:dequeueCell()
    if nil == cell then
        cell = cc.TableViewCell:new()
        cell.view = self.class.new()
            :pos(self.class.designSize.width/2, self.class.designSize.height/2)
            :addTo(cell)
    end
    -- 如果需要刷新的话, 可能需要自己去处理, 如不需要, 就可以不用下面的这个调用
    cell.view:refresh()

    return cell
end, cc.TABLECELL_SIZE_AT_INDEX)
```


## 3. 获取/新增/修改/删除节点

这些操作分别对应 `cellAtIndex`/`updateCellAtIndex`/`insertCellAtIndex`/`removeCellAtIndex` . 这些接口很好理解, 但就多数情况而言, 他们可能需要和 `reloadData` 配合使用. 


好了, 以上就是基本用法了. 在使用时还有一些需要注意的细节:

1. 不能使用 setNodeEventEnable, 因为和 Node 的 registerScriptHandler 有冲突.
2. id 从 0 开始, 而大家在 lua 这边准备的数据多是以 1 开始, 这样可能需要 +1 和 -1, 需要细心一些.


---

# 二. 一些异常的解决方案

## 1. lua 出错导致 player 崩溃

TableView 回调 `tableCellAtIndex` 在lua这边的实现一旦出错, 就会在 c++ 那边收到一个 NULL 的 cell, 因为没有判空, 下面对 cell 的操作就会导致 Plaer 崩溃. 对应修改如下:

```c++
     cell = _dataSource->tableCellAtIndex(this, idx);
-    this->_setIndexForCell(idx, cell);
-    this->_addCellIfNecessary(cell);
+    if(cell)
+    {
+        this->_setIndexForCell(idx, cell);
+        this->_addCellIfNecessary(cell);
+    }
```

这时候, 不会崩溃了, 但是也看不到 lua 那边出的什么错误, 经过一番追踪, 定位到了 `LuaStack::executeFunction` 函数:

```c++
        if (error)
        {
            if (traceCallback == 0)
            {
                CCLOG("[LUA ERROR] %s", lua_tostring(_state, - 1));        /* L: ... error */
                lua_pop(_state, 1);                                        // remove error message from stack
            }
            else                                                           /* L: ... G error */
            {
+               CCLOG("[LUA ERROR] %s", lua_tostring(_state, - 2));        /* L: ... error */
                lua_pop(_state, 2);                                        // remove __G__TRACKBACK__ and error message from stack
            }
            return 0;
        }
```

虽然出错了, 但是 `traceCallback` 的值并不等于 0, 所以没有进入输出错误的逻辑, 具体用意并不明白. 我的添加了带 `+` 号的哪一行, 输出了下就可以看到 lua 的错误了.

## 2. 滚动后导致节点上按钮触摸失效

大家使用时可能会遇到这样的问题, 节点上有一个利用 quick 触摸机制实现的按钮, 在 TableView 滚动后触摸事件都会失效, 按钮无法被点击.

这个实际上是 quick 触摸机制的一个bug, 复现是很容易的. 大家创建一个按钮并调用 `retain`, 然后将这个按钮添加到父节点上, 在某个时候将按钮从父节点上移除 (`removeFromParent`) 并再次添加(`addChild`)上到父节点. 这时候按钮还在, 但是触摸事件已经没有了.

究其原因是我们一般会在对象的 `ctor` 函数中 `setTouchEnable`, 然后 quick 在收到 `cleanup` 事件后移除了对象触摸事件, 具体逻辑大家可以看 `Node:EventDispatcher` 函数. 而一个节点从父节点上移除时恰好会发送 cleanup 事件.

对应到 TableView 中来, TableViewCell 为了做到复用在 `dequeueCell` 时会调用 `retain` 函数, 并且在移出屏幕时会被从 Container 上移除掉的. 这样 TableViewCell 的所有子节点都会收到对应的 cleanup 事件.

这个问题的解决方案恰好是我之前写的一篇文章, [为 quick-cocos2d-x 添加析构事件][1], 在这篇文章中, 已经修改为收到 destroy 事件后才去移除节点的触摸事件, 非常完美的解决了这个问题.

> Update: 2016-04-24

## 3. 遮挡上层按钮触摸事件

经过这几天的使用, 我发现了一个十分严重的问题. 如下图所示, TableView 中的某个按钮拖出 ViewRect 范围后会不可见, 但如果其位置恰好在 TableView 外的另一个按钮范围内, 就会优先收到点击事件. 这就会引发十分奇怪的现象, TableView 外的按钮看得见点不着, TableView 内的按钮看不见却可以响应到, 十分影响体验.

![][2]

因为有着以往 cocos 2.x 的悲惨经历, 我非常武断的认为这肯定是 TableView 的 bug, 开始着手阅读 TableView 的代码实现, 却一直未果. 然而团队中另外一位成员 @小齐同学 的意外发现, 让这个问题的谜底在无意中就被揭开了.

我们在项目中大量使用了 quick-x 提供的一个控件 `UIScrollView`, 它是一个用 `ClippingRegionNode` 纯 lua 实现的 `CCScrollView`, 一直以来工作的十分良好. 但是在某一天 测试中, @齐少 意外的发现, 某个界面的 UIScrollView 出现了和 TableView 一模一样的问题, 导致上层按钮无法点击. 经过排查, 发现是因为 UIScrollView 的创建顺序被延后的原因, 如果一个按钮先于 UIScrollView 添加到父节点, 就会被 UIScrollView 中的按钮所屏蔽, 后添加则不会.

当 @齐少 告诉我这个结论后, 我立刻意识到, TableView 遇到的问题肯定也是这个原因, 查看代码后果然如此, TableView 是最后被创建的. 解决方案也十分简单, 将 TableView 外部按钮放到 TableView 之后去创建就可以啦.


---

恩, 以上就是我在使用 TableView 时遇到的所有问题了, 虽然解决了这些问题, 但是在使用上还是十分的繁琐. 若是在一两个界面上使用可能还可以接受, 但若是想大规模推广就很有些困难了, 代码会变得十分冗长.

在此基础上, 我们又对 TableView 进行了一次封装, 数据驱动, 使用时不用过分关注界面的逻辑, 中心更多的落在了数据的组织上, 真正的做到了 "开箱即用" , 等我们内部进行推广并稳定后可以再和大家分享下心得.

[1]: /2015/05/17/onDestroy
[2]: http://ww4.sinaimg.cn/large/7f870d23gw1f381i7v2adj206c045glr.jpg