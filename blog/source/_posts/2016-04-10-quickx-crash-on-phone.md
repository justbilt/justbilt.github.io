title: 最近遇到的几个 quick-x  真机崩溃
date: 2016-04-10 20:14:18
categories:
- quick-cocos2d-x
- crash
description: "卧槽, 又崩溃了!"
---

最近这段时间遇到了两次比较严重的真机崩溃问题, 都是之前所没有遇到过的, 特此记录一下, 希望能帮助到遇到类似问题的朋友.

之所以强调真机, 是因为这些问题在 player 上或者 debug 版无法出现, 只有真正运行在手机上才可能遇到, 因为最后我们 Archive 出来的包都是 release 版的.


# 1. removeFromParent

removeFromParent() 多次会导致崩溃并闪退, 很难以想象是吧, 在我的记忆中应该只是抛出一个 lua 错误. 事实上, debug 版确实如此, 让我们看一段测试代码:

```lua
self._btnLogin:removeFromParent()
self._btnLogin:removeFromParent()
```

player 上会收到如下的 lua 错误提示:

```lua
LUA ERROR: [string "xxx.lua"]:349: invalid 'cobj' in function 'lua_cocos2dx_Node_removeFromParentAndCleanup'
```

让我们看下抛出这个错误的地方 `lua_cocos2dx_Node_removeFromParentAndCleanup` :

```c++
#if COCOS2D_DEBUG >= 1
    if (!cobj)
    {
        tolua_error(tolua_S,"invalid 'cobj' in function 'lua_cocos2dx_Node_removeFromParentAndCleanup'", nullptr);
        return 0;
    }
#endif
```

第一次调用完 removeFromParent(), `_btnLogin` 的 c++ 对象就已经被释放了, 再次调用 `cobj` 就是 `NULL` 了, 所以就会进入到这个判断中, 从而抛出错误.

但是这段代码却是运行在 COCOS2D_DEBUG 宏中的, 就意味着这段代码在 release 版是不生效的, 所以就会接着往下执行, 从而导致崩溃.

虽然我们不太可能直接会用一个对象调用 removeFromParent 多次, 但是包含 removeFromParent 的函数被调用多次却是很有可能的, 所以我们一定要养成一个好的习惯, 在 `removeFromParent` 之前要做判空检测, 之后要立刻置空:

```lua
if xxx then
    xxx:removeFromParent()
    xxx = nil
end
```


# 2. Signal 13 was raised. SIGPIPE

当游戏在 iOS 上不切换到后台直接锁屏一段时间, 网络资源就会被系统回收掉, 这时候解锁屏幕, 游戏并不知道 tcp 连接已经断开, 发送消息就会触发这个崩溃.

表现出来的效果是手机解锁不了, 得等好久才可以.

关于具体技术解释, 大家可以看这篇文章, [Signal 13 was raised（SIGPIPE管道破裂）][2], 我这里重点讲解决方案.

网上说有两种解决方案([如何在 iOS 上避免 SIGPIPE 信号导致的 crash][3]):

## 1. 在全局范围内忽略这个信号
```c++
signal(SIGPIPE, SIG_IGN);
```

## 2. 在一开始的时候设置 socket 不要发送 SIGPIPE 信号
```c++
int value = 1;
setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));
```

就 1 来说, cocos 早已设置了忽略, 在 `socket_open` 函数中, 并且这的执行到了. 我在 `tcp.c` 中的 `tcp_create` 中加入 2 方案, 确实解决了这个问题. 但是 `SO_NOSIGPIPE` 并不是跨平台的, 我偷懒直接用宏判断了下:

```c++
static int tcp_create(lua_State *L, int family) {
    t_socket sock;
    const char *err = inet_trycreate(&sock, family, SOCK_STREAM);
    /* try to allocate a system socket */
    if (!err) {
        ...
        
#ifdef CC_TARGET_OS_IPHONE
        int val = 1;
        setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&val, sizeof(int));
#endif
        ...
}
```





[1]: http://ww2.sinaimg.cn/large/7f870d23gw1f2rxuvmowqj20wg0kmtjs.jpg
[2]: http://blog.csdn.net/jia12216/article/details/50844013
[3]: http://www.jianshu.com/p/1957d2b18d2c