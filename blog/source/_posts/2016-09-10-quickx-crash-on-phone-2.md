title: 最近遇到的几个 Quick-cocos2d-x 真机崩溃(二)
date: 2016-09-10 23:28:45
tags:
- Quick-Cocos2d-x
- crash
description: "卧槽, 又崩溃了!"
---

大概是 8 月中旬的时候, 我们项目发生了一个很严重的线上事故. 在版本更新之后, 部分 Android 玩家反馈点击按钮开始游戏或活动按钮会闪退.

开始收到这个反馈时, 并没有太在意, 心想是不是机型适配有问题 ? 加上当时有别的工作在忙, 就没有去理会. 大概一个小时后, 玩家的邮件像雪花一样纷纷而至, 我才开始意识到, 更新出问题了.

汇总了一下玩家的反馈:

1. 新注册账号没有问题, 老账号会崩溃
2. 新服务器没有问题, 老服务器崩溃
3. 删除游戏重装后就没有问题了

看着这些条件, 有经验的老司机可能已经看出倪端了, 但当时我并没有看出. 我们平时测试的时候, 多数都会新注册一个账号, 或者将原先的老包删除掉, 所以 Q&A 并没有测试出这个问题, 也没有办法复现这个我问题.

我们游戏是集成了两个错误统计sdk的, 一个是 Bugtags, 一个是 umeng 错误收集, 但是这两个都是没有办法收集到 native 层面的 crash 的. 

![][2]

正当我一筹莫展的时候, 运营人员发来了 umeng 的这个错误列表, 我恍然大悟:

> 本地存档出问题了

因为我们的玩家存档是以 `服务器id+玩家id` 形式存入的, 所以上面列举的问题一下子就解释的通了.

# 一. 用 getStringForKey 去获取一个 bool 型的存档

后台看到的错误日志是这个样子的:

> java.lang.ClassCastException: java.lang.String cannot be cast to java.lang.Boolean

![][1]

这也可以? 这个错误闻所未闻, 只能说 Android 那边类型检测太严格了, 同时 cococ 读取存档时没有做异常处理.

有了方向就好办了, 我搜索了代码中所有用到 `getStringForKey` 的地方, 加上崩溃出现的时机, 很快定位了这个错误发生的原因.

团队的中的一位成员, 这次更新时更换过存档的类型, 之前的某一个数据是以 `setBoolForKey/getBoolForKey`, 现在更换成了 `setStringForKey/getStringForKey`, 这个问题不能怪他, 若是我也会遇到这个问题.

修改也很简单, 我换了一个新 key 去 存取/读取 这个存档.


# 二. setSpriteFrame(“a.png”) 传入不存在的 Sprite Frmae Name 就会崩溃

这个问题以前我没有遇到过, 我甚至都不知道 setSpriteFrame 可以传入 Sprite Frmae Name, 我每次都是 从 SpriteFrameCache 中获取一个 `SpriteFrame` 对象传入的.

导致这个问题发生原因是我们变化了图片打包策略, 之前用的是纹理图集, 现在改为碎片纹理了. 原理这些图片都在一个纹理图集上, 在游戏开始时已经 loading 过了, 所以不会有问题. 但是改为碎图就会找不到了.

```lua
function newSpriteFrame(_name)
    assert(_name and _name ~= "")
    -- 尝试在大图中找
    local frame = cc.SpriteFrameCache:getInstance():getSpriteFrame(_name)
    if not frame then
        local texture = cc.Director:getInstance():getTextureCache():addImage(_name)
        if texture then
            frame = cc.SpriteFrame:createWithTexture(texture, cc.rect(0, 0, texture:getContentSize().width, texture:getContentSize().height))
            cc.SpriteFrameCache:getInstance():addSpriteFrame(frame, _name)
        else
            print("[ERROR]no such file:", _name)
        end
    end 

    return frame
end
```

我们封装了这个函数来兼容这个 `纹理图集/碎片纹理`, 这样这个问题就解决了.

# 反思

每经历一次事故, 总要有所收获, 除了下次同样的问题不再发生, 流程上是否还有什么值得优化的地方 ?

## 1. 重视线上错误

由于我一开对这个错误的轻视, 导致问题没有在第一时间控制住, 损失的不只是收入, 更是玩家对我们信心, 如果一个游戏经常出事故, 你还会去玩吗?

错误率对我们而言只是一个数字, 是一个概率, 但是具体到某一个发生故障的玩家, 对他而言就是 100% , 就是焦急, 愤怒.

## 2. 真实的线上环境测试

这个问题发生, 有很大一部分原因是我们的测试没有在一个真实的环境中去测试, 应该保证一个手机只安装线上包, 模拟真实玩家, 每天去玩一段时间.

## 3. 错误收集系统的选择

我们采用的 bugtags 不具有完全完整的错误收集能力, 对错误的检索也很弱, 甚至不如免费 bugly . 为此我要付一定责任, 当初选择的时候, 没有完整的调研对比过, 只是看到这个还不错就选择了.

做出判断前一定要慎重思考.

[1]: /face/yilianmengbi.jpg
[2]: http://ww4.sinaimg.cn/large/7f870d23jw1f7xim4h1aaj21b50ej0ul.jpg