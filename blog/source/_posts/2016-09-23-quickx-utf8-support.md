title: quick-x utf8 支持
date: 2016-09-23 06:58:40
tags: [quick-cocos2d-x, utf8, emoji]
description: "唉, 什么都得自己搞."
---

# 一. 需求

## 1. 计算玩家名字字符数

对于这个需求一般情况下 `string.len` 或 quick 自带的 `string.utf8len` 就能满足, 但是如果需求是:

> 对于像 中文/日文/韩文 这样的方块字一个占 2 个长度, 其他字符占 1 个长度.

该如何满足呢 ? 

## 2. 屏蔽 emoji 表情

我们游戏的聊天/起名都是不允许输入 emoji 表情的, 那么该如何判断玩家输入的文字中包含 emoji 表情呢 ? 

在我之前的文章([quick-x EditBox 几个小技巧][1])中有提到过这个需求, 当时分析了下有两个解决方案:

1. 无法输入, 弹出键盘点击表情没有反应
2. 输入完成后, 游戏内点击提交时提示非法

本来打算是使用**方法2**的, 苦于无法在 lua 这边识别出 emoji , 所以只能曲线救国的使用的**方法1**, 每个平台得单独实现不说, 还容易出 bug, 出了 bug 亦无法热更新修复.

最近还真是遇到了 bug , 会导致在 ios 上无法使用**九宫格**输入法.

---

以上两个问题, 如果支持 utf8 的话, 我们可以遍历整个字符串, 判断每个字符的 `codepoint` 是否在某个码区中. 所以我们需要实现这么几个接口:

1. utf8 长度计算
2. 遍历 utf8
3. utf8 char/byte 实现

# 二. 实现

## 1. 使用 clib 实现

Google 搜索 `lua utf8` 很快发现了 [luautf8][1] 这个项目, 100 多个 star , 对于一个 lua 项目, 已经算很多了. 实现也很简单, 就 `unidata.h` 和 `lutf8lib.c` 两个文件.

下面我们在 quick 中集成这个项目, 我们在 `quick-cocos2d-x/external/lua` 目录先新建一个 `utf8` 目录, 将上面提到的那两个文件下载下来放进去, 修改 `lutf8lib.c:1303` 行:

```c
//  lua_createtable(L, 0, sizeof(libs)/sizeof(libs[0]));
//  luaL_register(L, NULL, libs);
  luaL_openlib(L, "utf8", libs, 0);
```

为了在别的文件中能够访问到 `luaopen_utf8` 函数, 我们还需要新建一个 `utf8lib.h` 头文件:

```c
#ifndef LUAUTF8_H
#define LUAUTF8_H

#include "lua.h"

#ifndef LUALIB_API
#define LUALIB_API extern
#endif

/*-------------------------------------------------------------------------*\
* Initializes the library.
\*-------------------------------------------------------------------------*/
LUALIB_API int luaopen_utf8(lua_State *L);

#endif /* LUAUTF8_H */
```

下面我们就可以注册这个库了, 修改 `cocos/scripting/lua-bindings/manual/network/lua_extensions.c` :

```c
#include "utf8/lutf8lib.h"
void luaopen_lua_extensions(lua_State *L)
{
    ...
    luaopen_utf8(L);
}
```

同时我们还需要修改编译脚本, 使得在不同平台上能够编译通过, Android 需要修改

 `cocos/scripting/lua-bindings/proj.android/Android.mk` 文件:

```mk
LOCAL_SRC_FILES += ../manual/network/lua_cocos2dx_network_manual.cpp \
                   ../../../../external/lua/luasocket/usocket.c \
                   ../../../../external/lua/utf8/lutf8lib.c
```

在 `LOCAL_SRC_FILES` 末尾加上 `lutf8lib.c` .

iOS/Mac 的话, 在 XCode 中将整个 `utf8` 目录加入进来就可以呀, 如下图所示:

![][3]

## 2. 纯 lua 实现

如果一切按计划走的话, 是不会有这么一步的, 然而天意难测, 说好的冷更新变成了变成了热更新. 若是还想保留这个功能的话, 只能寻找纯 lua 的解决方案了. 虽然走了不少弯路, 浪费了大量的时间, 最终还是让我找到了: 

[https://github.com/Stepets/utf8.lua][4]

将项目中的 `utf8.lua` 下载下来放到你工程中就可以啦, 就是这么简单.

## 3. 兼容

按说有了纯 lua 的实现后, 我们就可以放弃 c 代码的实现了, 但是想起做 python 的时候, 有好多库的实现为了提高效率, 都会有一份 c/c++ 的实现优先使用. 我们是不是也可以这样子搞, 优先使用 clib 的实现, 若是没有再考虑 lua 的实现 ? 首先, 我们要对比一下这两个库的效率对比.

设计了一个简单的测试案例, 遍历一个 utf8 的字符串, 计算耗时, 得出了这样一份数据:

| 字符数 	| 耗时clib            	| 耗时lua             	| 倍数            	|
|--------	|---------------------	|---------------------	|-----------------	|
| 10     	| 2.0999999999938e-05 	| 6.8000000000068e-05 	| 3.2380952381081 	|
| 138    	| 0.00018899999999999 	| 0.00095199999999995 	| 5.0370370370369 	|
| 2919   	| 0.001868            	| 0.01553             	| 8.3137044967881 	|

可以看到至少也有 3 倍的速度提升, 而且随着字符数越来越多, 速度差距会更大. 

这个方案是可行的, 我们可用如下代码做兼容:

```lua
if type(utf8) ~= "table" then
    utf8 = require("your/path/of.utf8")
end 
```

但是两种实现遍历字符串的 api 略有不同, 需要包装兼容一下:

```lua
utf8.foreach = function(_str, _func)
    local index = 1
    if utf8.next then
        for pos, code in utf8.next, _str do
            if _func(index, utf8.char(code), code, pos) then
                return
            end
            index = index + 1
        end
    elseif utf8.gensub then
        for char,pos in utf8.gensub(_str) do
            if _func(index, char, utf8.byte(char), pos) then
                return
            end
            index = index + 1
        end
    else
        assert(false, "no utf8 supports!")
    end
end
```

# 附录:

附上我们判断玩家姓名和 emoji 的代码, 比较简单, 若有不对之处, 欢迎指正.

## 玩家姓名长度判断

```lua
local length = 0

utf8.foreach(text, function(index, char, code, pos)
    -- 中日韩文字一个字符算两个长度
    if code >= 0x3040 and code <= 0x9fff then
        length = length + 2
    else
        length = length + 1
    end
end)
```

## emoji 表情判断

```lua
local function checkContainsEomji(text)
    local contain = false
    utf8.foreach(text, function(index, char, code, pos)
        -- [^\u0000-\u25ff\u27c0-\uD7FF\uE000-\uFFFF]
        if not ((code >= 0x0000 and code <= 0x25ff) or (code >= 0x27c0 and code <= 0xD7FF) or (code >= 0xE000 and code <= 0xFFFF)) then
            contain = true
            return true
        end
    end)
    return contain
end
```

[1]: /2016/05/01/quickx-editbox-util/#u4E8C-__u5C4F_u853D_Emoji__u8F93_u5165
[2]: https://github.com/starwing/luautf8
[3]: http://ww2.sinaimg.cn/large/7f870d23gw1f838r4uoz1j207e03u74f.jpg
[4]: https://github.com/Stepets/utf8.lua
