title: 记一次 Quick-cocos2d-x 内存泄露排查
date: 2016-03-19 17:48:38
tags:
- Quick-Cocos2d-x
- MemoryLeak
description: "谁动了我的内存 ? "
---

这周 @bin 告诉我项目有比较严重的内存泄露, 任意一个界面不停的打开关闭, 内存占用会一直往上涨, 直到被系统 kill 掉.

# 一. 确定问题

收到问题后, 我简单写了一段测试代码, 加载/移除界面 100 次, 对比内存变化:

```lua
local index = 0
local handler = nil
handler = scheduler.scheduleGlobal(function( ... )
    -- 加载界面
    app.sceneManager:pushLayer(require('app.scenes.PageShop').new())
    self:performWithDelay(function( ... )
        -- 移除界面
        app.sceneManager:popLayer()
        index = index + 1
        if index >= 100 then
            scheduler.unscheduleGlobal(handler)
        end
    end, 0.5)
end, 0.5)
```


为了方便, 我没有进行真机测试, 而是使用 xcode 启动 player 来进行测试, 在 Xcode 的 `Debug Navigator/Memory Report` 窗口查看结果.

![][1]

在测试前的平稳内存为 194M , 测试后惊人的达到了 258M, 十分严重的内存泄露了!

# 二. 初步解决问题

由于项目是纯 lua 的, 所以不太可能是数据和逻辑的问题, 那么很有可能是视图(cocos2d-x 对象)存在内存泄露. 而每一个界面都存在问题, 那么很可能是某个通用组件存在问题.

经过一番努力, 最终成功找到了内存持续增加的原因, 一共两处:

## 1. lua 垃圾没有及时回收

lua 的垃圾是会自动回收的, 但我们有时候可能需要手动回收下, 比如切换场景时, 关闭界面时, 主动回收的代码很简单:

```
collectgarbage("collect")
```

我将这段代码加到了统一关闭界面的地方.

更多关于 lua 垃圾回收的具体问题大家可以参考这个两篇文章:

[http://luatut.com/collectgarbage.html][2]
[http://www.codingnow.com/2000/download/lua_manual.html][3]

## 2. 精灵变灰和高亮的 shader 创建后一直没有释放

```lua
function GameUtils.SetSpriteGrey(sprite,is_grey)
    if sprite and sprite.setGLProgramState then
        if is_grey then
            local pProgram = cc.GLProgram:createWithByteArrays(ShaderData.vertDefaultSourceGrey, ShaderData.pszFragSourceGrey)
            ...
            sprite:setGLProgram(pProgram) 
        else 
            local pProgram = cc.GLProgramState:getOrCreateWithGLProgram(cc.GLProgramCache:getInstance():getGLProgram("ShaderPositionTextureColor_noMVP"))
            sprite:setGLProgramState(pProgram)
        end 
    end
end
```

这段代码看起来没有任何问题, 我能想到只是没有用 `GLProgramCache` 缓存起来, 造成每次都会创建的效率问题, 应该不会导致泄露吗 ? 精灵被释放时难道不会自动释放所引用的 GLProgram 吗?

因为当时项目比较紧急, 我将这段代码使用 `GLProgramCache` 的形式修改了一下, 惊奇的发现内存泄露问题竟然解决了. 修改后的代码如下:

```lua
function GameUtils.SetSpriteGrey(sprite,is_grey)
    if sprite and sprite.setGLProgramState then
        if is_grey then
            local pProgram = cc.GLProgramCache:getInstance():getGLProgram("ShaderPositionTextureColor_Gray")
            if not pProgram then
                pProgram = cc.GLProgram:createWithByteArrays(ShaderData.vertDefaultSourceGrey, ShaderData.pszFragSourceGrey)
                pProgram:bindAttribLocation(cc.ATTRIBUTE_NAME_POSITION, cc.VERTEX_ATTRIB_POSITION)
                pProgram:bindAttribLocation(cc.ATTRIBUTE_NAME_COLOR, cc.VERTEX_ATTRIB_COLOR)
                pProgram:bindAttribLocation(cc.ATTRIBUTE_NAME_TEX_COORD, cc.VERTEX_ATTRIB_FLAG_TEX_COORDS)
                pProgram:link()
                pProgram:updateUniforms()
                cc.GLProgramCache:getInstance():addGLProgram(pProgram, "ShaderPositionTextureColor_Gray")
            end
            sprite:setGLProgram(pProgram) 
        else 
            sprite:setGLProgram(cc.GLProgramCache:getInstance():getGLProgram("ShaderPositionTextureColor_noMVP"))
        end 
    end
end
```


# 三. 隐藏在背后的秘密

问题解决了, 那么是不是可以结束了呢 ? 并不能, 晚上回到家我就一直在思考这个问题, 倒是是什么原因导致了 GLProgram 内存没有释放, 而使用 GLProgramCache 就没有问题.

莫不是 cocos2d-x 的 bug ? cocos 对象是基于引用计数去自动释放内存的. 我排查了几处可疑的地方:

1. GLProgram::createWithByteArrays 调用了 autorelease
2. Node::setGLProgram 调用了 retain
3. Node::~Node 调用了 release

貌似都没有问题. 那么我可以跟踪下引用计数的变化, 看看是哪一步出现的问题! 经过一番调试,最终定位到了问题, 大家请看:

![][4]

1.`Node::setGLProgram` 进入到 `GLProgramState::getOrCreateWithGLProgram`, 这一步没有什么问题.

![][5]

2.这里有一个新的缓存 `GLProgramStateCache` , 进入它到 `getGLProgramState` 函数.

![][6]

3.这一步是 GLProgramStateCache 的核心代码了, 判断有无在缓存中, 没有则 insert 到末尾. 这里的 `_glProgramStates` 是一个 `Map` , 而这个 map 竟然是以传递进来的 `glprogram` 做
 key , 而我们每次传递进来的 glprogram 都是新创建的, 所以在我们这个使用情况下缓存根本是无效的.

![][7]

4.让我看一下, `GLProgramState` 的 `init` 函数, 这下找到 retain 地方了. 


这样的话, 就讲得通了, GLProgram 会在 GLProgramState 析构的时候 release 掉. 而 GLProgramState 只会在 GLProgramStateCache:removeAllGLProgramState 释放掉. 而 removeAllGLProgramState 只有在手动或者游戏退出的时候才会调用.


OK, 这下定位到了问题, 虽然我们使用有些问题, 但 GLProgramStateCache 设计确实有不合理的地方, 大家记得正确用法就好了, 就是 shader 一定要使用 GLProgramCache !

---

后记

通过这次解决问题, 我有一个特别大的收获. 就是做优化工作时一定不能去猜, 要有数据和逻辑的支持.

[1]: http://ww3.sinaimg.cn/large/7f870d23gw1f23kr8dricj20rh0fstab.jpg
[2]: http://luatut.com/collectgarbage.html
[3]: http://www.codingnow.com/2000/download/lua_manual.html
[4]: http://ww1.sinaimg.cn/large/7f870d23gw1f2anef57p3j20u60370ts.jpg
[5]: http://ww4.sinaimg.cn/large/7f870d23gw1f2aneul8cej20u7021jrx.jpg
[6]: http://ww3.sinaimg.cn/large/7f870d23gw1f2anf4bv4tj20u906l402.jpg
[7]: http://ww1.sinaimg.cn/large/7f870d23gw1f2anfex5v1j20tx03zjs1.jpg