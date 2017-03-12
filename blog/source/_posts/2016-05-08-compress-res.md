title: cocos2d-x 优化游戏资源体积
date: 2016-05-08 19:57:07
tags:
- 游戏心得
- 优化
description: "优化资源体积是一个旷日持久的战役, 那我们从哪里入手呢? "

---


# 一. 删除无用资源

在我们版本迭代的过程中, 总有一些图片被废弃掉, 如果当时忘记删除的话, 久而久之也就忘记了. 如果在上线前不做一次整理的话, 它们就会残存在你的资源中, 浪费包的体积.

为避免这种情况, 我们可以做的是:

## 1. 废弃的图片一定要及时删除
## 2. 编写废弃资源查找工具

可以用到的系统命令是 `ack`, 我们可以通过 `brew install ack` 安装, 使用的效果:

![][1]

关于 ack 的更多用法请[移步这里][2].

# 二. 使用替代对象

## 1. 使用9图

大家都知道图片在拉伸的过程中会失真, 那么如何避免这个情况呢? 使用9图. 

![][3]

注: 配图来自[http://mux.baidu.com/?p=1506][4]

这样我们就可以将一张很大的图缩小到很小, 然后使用9图拉伸, 起到节省资源的目的. 9图在 cocos 中的对象是 `Scale9Sprite`, 具体用法可以参考[这篇文章][5].

## 2. 通过修改色调实现资源复用

知乎上有一篇问题讲的就是这个:
[拳皇中的人物变色是如何实现的？][19]
[知乎日报上的这篇][18]

![][6]

cocos2d-x 版由 [@偶尔e网事][7] 大神实现, 对应的对象是 `SpriteWithHue`, 目前已经默认集成到了 cocos2d-x 中.

这样我们就可以将原来只是色调差异的图片用程序来实现啦~

## 3. 使用平铺

![][8]

注: 配图来自[http://bullteacher.com/7-textures.html][9]

游戏中的有些图片完全可以通过平铺实现, 这样的话我们就可以让美术只出一个平铺单元的图片,在程序中去实现平铺.

首先, 平铺的这个功能是 opengl 层面就支持的, 详情大家可以[移步这里][9], cocos2d-x 中实现平铺很简单:

```lua
-- 首先, 使用平铺单元图片创建一个精灵
local sprite = cc.Sprite:create("your_repeat_image.png")
-- 然后, 设置纹理参数
sprite:getTexture():setTexParameters(gl.LINEAR, gl.LINEAR, gl.REPEAT, gl.REPEAT)
-- 最后, 将这个精灵的纹理矩形设置为我们想要的大小
sprite:setTextureRect(cc.rect(0,0,1024,1024))
```

> 注意:　平铺单元图片的尺寸只能是2的幂


# 三. 压缩

## 1. 无损压缩

无损压缩还是十分值得推荐的, 它的原理知乎上[这个答案][10]讲的很清晰, 我节选其中的关键文字:
> 1.核心原理很简单，通俗的解释一下，就是由于PNG格式的灵活性，他可以有很多种方式表示同一张图片，不同方式有时就会导致文件大小不一样...
> 2.还有一点是PNG采用的是deflate算法，也非常的灵活，他的压缩率和encoder的实现有关，不同的encoder使用的时间，压缩出来的大小都不一样...
> 3.当然除了上面这两点是真正的无损压缩以外，还有减小PNG文件大小的方式就是去除一些对图片本身没有任何影响的metadata...

所以无损压缩纯粹是单方面的受益, **是一定要做的**.

我们无损压缩主要用到的工具: [ImageOptim][11]

## 2. 有损压缩

![][12]

有损压缩会损失一部分的图片质量, 但带来的受益还是十分可观的. 这是一个抉择的过程, 以最小的代价获取最大的受益, 甚至不能批量处理, 可能需要一张一张的人肉对比压缩.

我们有损压缩主要用到的工具: [PP鸭][13]


# 四. 选择正确的图片格式

## 1. 将无 alpha 通道的 png 图片存储为 jpg
## 2. 选用压缩率更高的图片格式

# 五. 其他

## 1. 圆形图片只使用 1/4

然后在程序中翻转3次,得到其他角度的图片. 一般会用在图片尺寸特别大的场景. 

![][14]

如上图, 我们游戏中一个全屏幕的雷达就是通过这个方案减少图片体积的.

## 2. 缩小图片

将展示精度不强的图片(比如: 游戏背景上的小装饰, 爆炸的序列帧)缩小, 在程序中放大. 

## 3. 特殊方案分离png的透明通道

[用jpg和黑白色png作为遮罩实现透明][15]
[用shader使图片背景透明][16]
[cocos2dx中使用JPG图和只带Alpha的PNG图合成渲染][17]

我们之前曾经采取过其中的一个方案, 将一张 png 图片拆分为 jpg+alpha.png 的形式, 整体的包体小了近 25% , 不过也带来的其他的一些副作用.

建议大家使用这类黑科技前一定要做好调研和测试用例, 评估一下实际的收益.

[1]: http://ww2.sinaimg.cn/large/7f870d23gw1f3upxy29uej20iy083gna.jpg
[2]: http://beyondgrep.com/documentation/
[3]: http://ww2.sinaimg.cn/large/7f870d23gw1f3uqafyaevj206t04mjrj.jpg
[4]: http://mux.baidu.com/?p=1506
[5]: http://shahdza.blog.51cto.com/2410787/1543284
[6]: http://ww4.sinaimg.cn/large/7f870d23gw1f3uqjn3tz0j20ig08at9r.jpg
[7]: https://fusijie.github.io/2015/05/27/sprite-with-hue/
[8]: http://ww4.sinaimg.cn/large/7f870d23gw1f3ur1pd7j5j20m8069tbd.jpg
[9]: http://bullteacher.com/7-textures.html
[10]: https://www.zhihu.com/question/23752454
[11]: https://imageoptim.com/mac
[12]: http://ww1.sinaimg.cn/large/7f870d23gw1f3uvav83k9j20g707fabp.jpg
[13]: http://ppduck.com/
[14]: http://ww4.sinaimg.cn/large/7f870d23gw1f3uvnfb31aj20hd09v74p.jpg
[15]: http://www.cocoachina.com/bbs/read.php?tid-201144.html
[16]: http://blog.csdn.net/dawn_moon/article/details/8631783
[17]: http://www.cnblogs.com/elang/p/4104452.html
[18]: http://daily.zhihu.com/story/4797855
[19]: https://www.zhihu.com/question/31133351