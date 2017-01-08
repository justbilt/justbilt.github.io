title: quickx CCGraySprite 消失坑 (已更新解决方案)
date: 2014-10-29 13:01:33
categories:
- quick-cocos2d-x
---

前几天团队中小A遇到了一个非常诡异的问题, 特此记录一下.

<!--more-->

![][1]

大家看上图中的一个灰色的精灵, 这个是由``CCGraySprite``创建出来的, 它被添加了到了CCNode中, 当第一次进入界面是正常, 退出界面, 再次进入改界面就看不见了, 但是只要一移动这个界面就又可以看见了.

这个问题十分的诡异, 出现的条件十分苛刻, 但是复现率是100%, 就比较好解决了. 我猜测有可能是一下几个原因:

第一个可能, 精灵的问题的被释放掉了, 但是这个无法解释 **只要一移动这个界面就又可以看见了**.

第二个可能, lua这边的内存被释放掉了,同样无法解释上边的现象

第三个可能, `CCGraySprite`的内部实现又问题, 这个比较好证明, **换一种变灰的实现**就可以验证了.

于是我们改用zrong贡献的滤镜来实现变灰的功能:

```lua
-- 创建灰度精灵
_spr = CCFilteredSpriteWithOne:createWithTexture(_texture, _rect)
local filters = filter.newFilter("GRAY")
_spr:setFilter(filters)
```

验证结果果然是``CCGraySprite``的问题,用``CCFilteredSprite``替换可以解决.


<del>因为比较着急, 虽然最后解决了, 但没有具体去分析原因, 希望有知道原因的同学可以告知一下.</del>

### Update 12月20日:

今天又遇到了这个bug, 看来得好好研究下了!

Google了一下, 看来不止我一个人遇到了这个问题, [谁用过CCGraySprite，移除再新建就不会显示了][2], 虽然没有解决, 但是知道怎么重现了.

> 谁用过CCGraySprite，**移除再新建就不会显示了**，不知道大家用什么灰态精灵，求解.

这个确实容易重现, 测试代码:

```lua
function MainScene:ctor()
    ui.newMenu({ui.newTTFLabelMenuItem({text = "删除&创建", listener = handler(self, self.init)})})
        :pos(display.cx, display.cy - 100)
        :addTo(self)

    self:init()
end

function MainScene:init()
    if self.spr then
        self.spr:removeFromParent()
    end

    self.spr = CCNodeExtend.extend(CCGraySprite:create("start.png"))
        :pos(display.cx, display.cy)
        :addTo(self)
end
```
我们新建一个CCGraySprite, 点击按钮后删除再重新创建, 就真的看不见了!

起初我们猜测是因为删除后立刻创建的问题, 在加了定时器后依然如此, 排除!

然后我们去看CCGraySprite的实现, 既然第二次创建会出现问题, 那么很可能是init的实现有问题, CCGraySprite 的实现最终都会调用 initWithTexture

```c++
CCGLProgram* pProgram = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureGray);
setShaderProgram(pProgram);
CHECK_GL_ERROR_DEBUG();

//为渲染的顶点流增加属性（顶点属性： position, texCoord, color, normal 等）
getShaderProgram()->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
getShaderProgram()->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
getShaderProgram()->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
CHECK_GL_ERROR_DEBUG();

//链接生成shaderProgram，初始化完毕后调用生成才能够在程序中使用 
getShaderProgram()->link();
CHECK_GL_ERROR_DEBUG();

//更新2dx自带的几个uniform 值（使之与shader中的变量值同步）
getShaderProgram()->updateUniforms();
CHECK_GL_ERROR_DEBUG();
```
因为在``CCShaderCache::loadDefaultShader``中看到过一样的代码, 就怀疑是因为**多次初始化**的原因. 经过排查, 将

```c++
getShaderProgram()->link();
```

这行代码注释掉之后成功! 附图:

![][3]

(EOF)



[1]:http://ww2.sinaimg.cn/large/7f870d23jw1elry310gmoj20dv090jsl.jpg
[2]:http://www.cocoachina.com/bbs/read.php?tid-217500-page-e-fpage-1.html
[3]:http://ww1.sinaimg.cn/large/7f870d23gw1enoa7sayraj20tu0lkjsq.jpg
