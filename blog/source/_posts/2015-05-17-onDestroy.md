title: 为 quick-cocos2d-x 添加析构事件
date: 2015-05-17 14:04:37
tags: [quick-cocos2d-x]
---

# 一. 为什么会需要析构事件 ?

quick 的 class 实现提供了类似c++构造函数的`ctor` , **却没有提供类似 c++ 的析构函数**. 我们确实需要这样的一个回调,去写一些retain对象的release调用, 移除监听的事件等. 那么该怎么做呢?

<!-- more -->

## 1. onEnter 与 onExit

onEnter 与 onExit 确实是一个蛮不错的选择, 总是成对出现. 但是, 确实存在这样的问题:

> 问题1: 可能会被调用多次

确实如此, 当你以 pushScene 的形式加入一个新的场景, 原先场景的所有节点都会被调用 onExit, popScene 时又会调用 onEnter ; 或者我们想暂停一个 Node, 可以通过手动调用 onExit 来实现, 恢复时调用 onEnter, 等等. 这样子在 onExit 中写一写释放的代码似乎又不太合适.

> 问题2: 只有加入到渲染树的节点才会被调用 onEnter/onExit

存在这样的需求, 我们 new 一个 Node 对象, 但是却不急于将他 addChild 到另一个节点上, 而是先retain下, 这样的话就不会调用的onEnter/onExit. 或者我们有一个局部的 Node 对象, 不添加到节点树, 这样下一个回收周期会被回收掉, 但是整个过程却不会调用的onEnter/onExit.


## 2. ctor 与 onCleanup

当我看到 onCleanup 整个名字, 我一度为我找到了最终的解决方案. 事实上, **它在多数情况下确实是能够正常工作的**, 它会在 node 被 remove 时调用. 说到这里你可能会敏锐发现它会遇到与 onExit 相同的问题2, 也确实如此. **只有加入到渲染树的节点才会被调用 onCleanup**.

但是如果你没有上述的要求的话, ctor 与 onCleanup 配合使用还是很不错的. 需要注意的一点是, 调用 `removeFromParentAndCleanup(bool cleanup)` 时如果 cleanup 不传入 true 的话是不会触发 onCleanup 事件的.

---

综上所述, 现在并没有一个完美的方案, 只能自己搞一个.

![][4]

# 二. 实现

要实现一个析构消息, 其实特别简单, 只用在 Node 的析构函数中分发一个消息就可以了. 这里我用 `quick-3.5` 实现一下.

## 1. 添加 kNodeOnDestroy 事件并分法

打开 CCNode.h , 找到 `kNodeOnCleanup` 枚举, 并在下方添加 `kNodeOnDestroy` :

```c++
enum {
    kNodeOnEnter,
    kNodeOnExit,
    kNodeOnEnterTransitionDidFinish,
    kNodeOnExitTransitionDidStart,
    kNodeOnCleanup,
    kNodeOnDestroy
};
```
打开 CCNode.cpp, 找到`Node::~Node()`函数, 在函数开始处添加:
```c++
#if CC_ENABLE_SCRIPT_BINDING
    if ( _scriptType != kScriptTypeNone)
    {
        int action = kNodeOnDestroy;
        BasicScriptData data(this,(void*)&action);
        ScriptEvent scriptEvent(kNodeEvent,(void*)&data);
        ScriptEngineManager::getInstance()->getScriptEngine()->sendEvent(&scriptEvent);
    }
#endif // #if CC_ENABLE_SCRIPT_BINDING
```
打开 `CCLuaEngine.cpp`, 修改 `handleNodeEvent` 函数, 添加:
```
case kNodeOnDestroy:
    _stack->pushString("destroy");
    break;
```

编译 player, c++ 这边的修改就结束了.

## 2. Lua 添加事件处理

打开 NodeEx.lua , 在`onCleanup`函数下方添加:
```lua
function Node:onDestroy()
end
```

修改`setNodeEventEnabled`函数, 添加调用:
```lua
...
elseif name == "cleanup" then
    self:onCleanup()
elseif name == "destroy" then
    self:onDestroy()
end
...
```

---
下来我们要修改一下lua这边移除事件监听的条件, 之前是收到`cleanup`消息, 现在应当改为`destroy`消息.

还是 NodeEx.lua , 找到`EventDispatcher`函数, 修改判断条件如下:
```
if idx==c.NODE_EVENT then
    event = { name=data }
    if data=="destroy" then
        flagNodeCleanup = true
    end
```

## 3. 测试

> 1.我们新建一个scene, 然后添加如下代码:

```lua
local nodeEventTest = class("nodeEventTest",function()
    return display.newScene("nodeEventTest")
end)

function nodeEventTest:ctor()
    cc.FileUtils:getInstance():addSearchPath("src/")

    self:setNodeEventEnabled(true)
end

function nodeEventTest:onDestroy()
    print("--nodeEventTest:onDestroy")
end
```

启动这个scene后,然后再进入另一个scene后,会在控制台观察到输出.


> 2.我们在一个Layer中添加下面这几行代码:

```lua
local node = display.newNode()
node:setNodeEventEnabled(true, function(event)
    print("node:",event.name)
end)
```

可以在控制台观察到:
```lua
[LUA-print] node: destroy
```

因此我们的做法是成功的, 一个临时cocos对象也可以收到`onDestroy`事件.

--EOF--

[4]: /img/22015-04-19-001.jpg


