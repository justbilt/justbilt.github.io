title: 手机游戏攻防(二) 守护变量法
date: 2015-04-05 10:51:34
categories:
- 游戏心得
- 手机游戏攻防
---

## 一.思路

我在13年8月的时候写过一篇游戏防八门神器修改的文章([见这里][1]), 当时介绍了一个守护策略就是`变量加密法`, 今天我们来介绍下另一种思路: `守护变量法`.

<!-- more -->

这个最开始想法来自于我之前看到的一篇文章[<<从外行的视角尝试讲解为什么这回丰田栽了>>][2]中的一小节: 

> 对关键变量缺乏保护。
嵌入式系统，或者任何系统，都会在一定条件下发生硬件或者软件错误。客观上这是无法避免的。而且汽车作为一个时常在震动、发热、位移的系统，它的内部环境不能说不恶劣，发生硬件错误的可能性甚至更高。什么样的硬件错误呢？别忘了变量都是0和1的组合，这些0和1由存储器上的高低电平代表。由于某些不可抗原因，一个电平从高变成低，或者反过来，那么这个变量就被更改了。这被称为“位反转（Bit Flip）”。为了对抗这样的事情发生，需要对变量进行保护。保护的方法一般是镜像法。简单来说就是在两个不同的地方写入同一个变量，读取的时候两边都读，比较是不是一致。如果不一致，那么可以得知这个变量已经不可靠，需要进行容错处理。


这篇文章非常有意思, 大家不妨仔细阅读一下! 当时我看完文章后是非常震惊的, 作为一个游戏程序员, 我们出错后可能只会导致公司的一些收入损失. 但是对于开发汽车系统的程序员, 他们出错的后果可能就是一条条人命了! 这使得我之后写代码时更加谨慎, 觉得可能出错的地方都会加以处理.

言归正传, **守护变量法的核心思想是为每一个变量都建立一个守护变量, 通过守护变量去验证原始变量的合法性**, 思路与原文一致, 但实现略有不同, 就是守护变量与原始变量**不是单纯的镜像,而是进行加密处理**! 原因很简单, 单纯镜像会使得守护变量也被修改掉, 失去守护的能力!

那么问题就来了? 同样需要加密, `守护变量法`较`变量加密法`又有什么优势呢? 两点:

1. 可以监测到用户修改的行为
2. 可以对数据进行还原

**可以监测到玩家修改数据的行为**使得我们掌握了主动权, 接下来是杀是留全在我们掌控之中. **可以对数据进行还原**又能保证进行温和的惩罚后游戏仍能正常的进行!


## 二.实现

这里我用 quick-cocos2d-x 简单实现下:

1). `CryptoNumber.lua`:

```
local CryptoNumber = class("CryptoNumber")

function CryptoNumber:ctor(_value)
	self:setValue(_value)
end

function CryptoNumber:decode()
	return tonumber(crypto.decodeBase64(self.protect_data))
end

function CryptoNumber:encode(_value)
	return crypto.encodeBase64(_value)
end

function CryptoNumber:setValue(_value)
	 self.data = _value
	 self.protect_data = self:encode(_value)
end

function CryptoNumber:getValue()
	if not self:check() then
		print("error!") --这里做修改后的处理!
		self:setValue(self:decode())
	end
	return self.data
end

function CryptoNumber:check()
	return self.data == self:decode()
end

return CryptoNumber
```

2). `MyApp.lua`

```
local n = require("utils.CryptoNumber").new(5)
print("getValue:",n:getValue())
n.data = 10 -- 这里用直接赋值模拟数据修改
print("getValue:",n:getValue())
```

3). console

```
[LUA-print] getValue:	5
[LUA-print] error!
[LUA-print] getValue:	5  //可以看到数值被还原回去了
```

演示代码, 没有考虑太多. 对于 lua 来说有更简单的做法, 比如使用`metamethod`.

## 三.后记

其实我是建议对修改数据的玩家进行温和处理的, 比如监测到玩家修改数据后, 弹出提示告诉玩家解锁成就, 比如*游戏大师*. 或者走煽情路线, 告诉玩家我们开发游戏的不易, 各种苦逼各种惨. 也可以做成一个彩蛋, 弹出开发者信息. 甚至可以给玩家一些小奖励. 核心就是: **玩家不是我们的敌人, 让玩家感受到我们的诚意**. 




[1]:/2015/04/04/phone-game-AND-encrypt-var/
[2]:http://club.tgfcer.com/thread-6817371-1-1.html