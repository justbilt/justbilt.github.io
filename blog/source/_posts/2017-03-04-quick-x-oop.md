title: Quick-Cocos2d-x 中的面向对象
date: 2017-03-04 10:04:44
tags:
- quick-cocos2d-x
description: "没有对象? 自己 new 一个就好了嘛!"
---

虽说现在一直提倡组合优于继承, 但是我们这帮深受 c++ 毒害的大好青年还是对继承有着深深的感情. Lua 这门语言本身并没有提供面向对象的机制, 不过我们可以很容易的通过 Lua 的 metatable 来实现一套面向对象的机制.

# 一. 实现机制 

实现的原理很简单, 如果一个 Lua 的 table1 通过 `setmetatable` 函数设置了元表之后, 如果试图访问一个不存在的 `属性`, 就会触发这个 metatable 的 `__index` 元方法, 这个 __index 可以是另一个 table2 , 这样它就回去这个 table2 中去找那个属性了, 如果 table2 中还没有的话, 就会触发 table2 的元方法, 就这样一层一层的往上找. 我们可以用一个十分简单的栗子来测试下这个功能:

```lua
local a = {aa = 1}
local b = {bb = 2}
local c = {cc = 3}

setmetatable(b, {__index = a})
setmetatable(c, {__index = b})

print("aa:", c.aa, "bb:", c.bb, "cc", c.cc) -- output: aa:  1   bb: 2   cc  3
```

如上, 只是加了两个 `setmetatable`, c 便可以访问到 a 和 b 的属性, 是不是很神奇.

# 二. Quick-x 中的实现

Quick-x 作为一个 framework 自然也实现了一套这样的机制, 因为函数实现比较长, 所以我就不粘贴代码了, 大家可以[跳转这里查看][1]. 纵观这段代码, 可以以最外层的 `if-else` 将这段逻辑分成两部分, 继承自 Cocos 的对象和继承自 Lua 的 Table, 为什么要这么分呢 ?

因为 Cocos 的对象在 Lua 中的 type 是 `userdata`, 是不能设置 metatable 的, 所以我们之前说的那套继承的方法就行不通了, Quick-x 在这里的选择是把所有的变量都复制一份, 做了一次一维的深拷贝. 

所以, 抛开继承的实现不同, 这两个分支的逻辑是一致的. 我们精简下, 可以分离出下面这段简短的代码:

```lua
function class(_name, _super)
    local cls = {__cname = _name, super = _super}
    if _super then
        setmetatable(cls, {__index = _super})
    else
        cls.ctor = function()end
    end
    function cls.new(...)
        local instance = setmetatable({}, {__index = cls})
        instance:ctor(...)
        return instance
    end
    return cls
end
```

这短短十几行代码几乎已经包含了面向对象的所有特性, 有实例函数 `new`, 有默认构造函数 `ctor`, 可以访问父类 `super`. 

大家可以看到这段代码有两次 `setmetatable`, 第一次是在创建 cls 的时候, 是为了让 cls 能够访问到 super 中的属性; 第二次是在产生实例 `instance` 的时候, 是为了让实例能够访问到 cls 中的属性. 

# 三. 要注意的地方

下面这段代码是一个典型的面向对象示例:

```lua
local Animal = class("Animal")

function Animal:ctor(_name)
    self.name = _name
end

function Animal:say()
    print("i couldn't say!")
end

local Dog = class("Dog", Animal)

function Dog:ctor(_name, _age)
    Dog.super.ctor(self, _name)
    self.age = _age
end

function Dog:say()
    print(string.format("I'm a %s, my name is %s, i'm %s years old.", self.__cname, self.name, self.age))
end

Dog.new("Jack", 5):say()
```

我们重点看下第 `14` 行代码, 这行代码有些奇怪.

## 1. 为什么用类名去调用 super ?

当继承到第三层的时候 self.super 从 C 跳转到 B 时 self 还是 C 的实例, 这时 B 中 self.super 其实还 B, 就会造成 stack overflow.

## 2. 为什么是类名 `.` super 而不是 `:` super ?

首先大家要明白 Lua 中 `.` 和 `:` 调用函数的区别是什么 ?

> 冒号调用是 Lua 提供的一个语法糖, 默认会将函数的调用者作为第一个参数传入.

区别在于父类的函数收到的 self 是 `self` (即 Dog 的实例) 还是 `Dog.super` (即 Animal) , 很明显应该是后者.

# 四. 有什么改进的地方

很早之前就拜读过[这篇文章][2], 这篇文章中列举了一些作者的疑惑, 很有收获, 大家可以看一下. 文章中提出了两条:

> 问题1：从父类做深度拷贝
> 问题4：创建实例时的深度拷贝

作者当时可能忽略了我们刚才做的解释: *无法向一个 userdata 设置 metatable* , 那么我们这里是否真的需要一次深拷贝呢 ? 让我们先思考一个问题:

> 既然 Cocos 的对象是一个 `userdata`, 那么我们为什么可以往这个 userdata 上添加新的 Lua 属性呢 ?

Quick-x 为 C++ 导出 Lua 接口的工具是 tolua++ , 其中有两个接口叫: `tolua.setpeer` 和 `tolua.gerpeer` , 这个 peer 又是一个什么东西呢 ?

![][4]

这张图是六月大大在 Cocos 论坛中的回复, 我们可以理解为 `peer` 是用来存储 C++ 对象在 Lua 中的扩展的, 他的本质是一个 table. 如果我们试图访问一个 userdata 类型的属性时, 如果这个 userdata 设置了 peer 表, 会优先从这个表中取值.

既然这个 `peer` 是一个 table, 那么是否可以为这个 table 设置元表, 这样在 peer 表中找不到就会触发 __index .

我们对 class 的实现做出下面的修改:

![][5]

用设置 peer 表的 metatable 来代替原来的深拷贝, 重新运行我们的项目, 完美. 让我们想想下这个实现带来的优势: **更快, 更省内存**, 为此我做了一个小的性能测试:

```lua
local A = class("A", function()
    return cc.Node:create()
end)

for i = 1, 100 do
    A["a_index_"..i] = i
end

local B = class("B", A)

for i = 1, 100 do
    B["b_index_"..i] = i
end

for i=1,10000 do
    B.new()
end
```

我设计了 A,B 两个类, B 继承自 A, 每个各有 100 个成员变量, 最后创建 10000 个 B 的实例. 统计了下内存的占用情况和耗时. 结果如下:

```lua
深拷贝:
耗时: 2.988687
内存: 134.1M

setmetatable:
耗时: 0.107388
内存: 38.9M
```

这个相差很多呀, 感觉自己马上就要走上人生巅峰了. 既然相差这么多, 而廖大貌似也早已发现了这个问题:

![][6]

为什么直到 3.3 版本还没有改动呢 ? 莫非是我哪里计算错了, 抛开测试, 我在我们的一个线上项目中做了一个真是的测试, 结果令我大跌眼镜.

> 相差无几

仔细想了下, 是否每一个对象都会有 100 个属性, 游戏内是否会同时存在 1w 个对象 ? 所以那份测试是没有意义的, 但是这个改动却是很有意义的.

其实在发现 peer 表之前, 我还做过另一个尝试, 对于 userdata 类型的实例, 不返回这个实例, 而是返回一个 table, 有一个属性 `_cobj` , 设置这个 table 的元表, 使得所有的属性都优先从 `_cobj` 中取, 取不到再去 super 中取, 后来因为改动太大, 就放弃了, 不过在这个改动的过程中意外的发现 peer 表的存在.

多折腾, 总会有所收获的.

[1]: https://github.com/chukong/quick-cocos2d-x/blob/master/framework/functions.lua#L281-L339
[2]: http://jennal.com/2014/10/18/cocos2dx-lua-oop/
[3]: https://github.com/zfengzhen/Blog/blob/master/article/tolua%2B%2B%E5%AE%9E%E7%8E%B0%E5%88%86%E6%9E%90.md
[4]: https://ww1.sinaimg.cn/large/006tNc79ly1fdbsvt09tnj313o08ewg5.jpg
[5]: https://ww3.sinaimg.cn/large/006tNbRwly1fdbtiv4cbaj30v009i3zv.jpg
[6]: https://ww3.sinaimg.cn/large/006tNbRwly1fdbv2g5bfoj30m208mjs8.jpg