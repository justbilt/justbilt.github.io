title: 从保卫萝卜看cocos2d-x循环播放音效的问题
date: 2013-11-28 08:28:03
tags: [cocos2d-x,音效]
categories: [cocos2d-x]
---


##起因
保卫萝卜2前几天上ios平台了,立刻下了一个玩,卧槽,各种呆萌蠢贱啊,有一个塔挺有意思,**魔法球**:
<!--more-->
![][1]

这种塔的攻击力虽然低,但是攻击速度快啊,而且升级后还能攻击多个目标,用在对付血少速度快的怪物再适合不过了.玩了挺久发现这个塔的音效有些问题:

1. 屏幕中有多个魔法球塔时,卖掉其中一个可能会导**致所有魔法球塔的音效停止.**

2. 有时候卖掉所有的魔法球塔,但**是声音还在播放!**

一般情况下我是不会mind这种细节的,但由于保卫萝卜是用cocos2d-x制作的,所以我稍微多想了一下:

猜想1:由于攻击速度十分快,所以我假设魔法球这个塔的音效播放和其他的塔不太一样,可能是这个样子的:**切换为攻击模式时循环播放音效,切换为待机模式时停止音效!**
猜想2. 如果假设成立,这个音效由于会循环播放,如果声音太吵的话,会**十分的吵**,萝卜显然注意到了这点.
猜想3. 所有的魔法球塔会**公用一个特效声音,**要不造的塔多了就根本没法开声音玩了!

**细思极恐,其实这座塔的音效播放逻辑十分的复杂!**

##验证
下面让我验证下我的猜想,看看能否帮萝卜解决这个问题!

###验证1:循环播放音效
这个十分简单,我在保卫萝卜2的资源中找到了``CarrotFantasy.app\Music\Towers\Ball.mp3``,看了时长1s左右,但是魔法球塔的攻击速度绝对大于2次,开2倍速的会大于4次,1s播放4次音效,没有人会这么做!所以魔法球塔是循环播放音效的!

###验证2:音效的特性
这个大家听一下就会知道了,魔法球的塔混在其他塔的音效里实在是太弱了!为此,我还**假装很专业**的用GoldWave查看了一下:
![][3]
图中上面的声音是瓶子炮的声音,下面是魔法球的声音,从图上可以看出,虽然魔法球的声音很长,但是只有在0.65s时声音较大,其他时候声音都很小!

###验证3.公用一个音效

开启循环播放音效十分简单,只需在``playEffect("1.ogg",true)``时第二个参数填true即可,这时playEffect会返回一个``unsigned int``类型的``id``,这个id会用于``stopEffect(id)``,如下:

```c++
unsigned int uID=SimpleAudioEngine::sharedEngine()->playEffect("1.ogg", true);//开始循环播放

SimpleAudioEngine::sharedEngine()->stopEffect(nID);//停止播放
```
这样看起来似乎很简单嘛,攻击的时候播,顺带记下ID,待机的时候停,这么简单,萝卜的程序太差了吧,这都能写错!其实不然,这只是场景中只有一座塔时的情况!

下面我们来试一下如果我们连续调用多次playEffect,会发生什么问题?我做了测试项目,将每次playEffect的返回ID打在屏幕上,win32截图如下:

![][2]

额,你没有看错,win32下播放同一个音效返回的id都是一样的,只要点击一下停止就全部都停了,为什么呢?让我们看一下win32
下声音的id是怎么生成的吧,在SimpleAudioEngine中我们可以看到这段代码:
```c++
//播放音效的函数
unsigned int SimpleAudioEngine::playEffect(const char* pszFilePath, bool bLoop)
{
    unsigned int nRet = _Hash(pszFilePath);//可以断定nRet就是返回的id

    preloadEffect(pszFilePath);

    EffectList::iterator p = sharedList().find(nRet);
    if (p != sharedList().end())
    {
        p->second->Play((bLoop) ? -1 : 1);
    }

    return nRet;
}

//生成id的函数
unsigned int _Hash(const char *key)
{
    unsigned int len = strlen(key);
    const char *end=key+len;
    unsigned int hash;

    for (hash = 0; key < end; key++)
    {
        hash *= 16777619;
        hash ^= (unsigned int) (unsigned char) toupper(*key);
    }
    return (hash);
}
```
这下明白了吧,**win32下的id是根据音效的路径pszFilePath算出来的**,这一点确实比较坑,大家以后要注意一下!下面我看一下在android上的效果吧!

![][4]

adnroid版是正常的,每次playEffect返回的id会递增,可以明显的听出又多了一个音效,当我音效叠了5层之后已经很乱了,但是萝卜的显然不是如此,所以所有的魔法球共用一个音效是正确的!

**注意:这里一定要提前preloadEffect,不然第一次返回的id是0,这样那个音效就永远停不下来了!**

##分析
基于以上猜想,我们来分析下魔法球塔的逻辑:
1.对于单个的魔法球塔,由于多目标,所以要监听目标数量的变化,当``target_count>0``时,开始播放音效,当``target_count=0``时,停止播放音效.
2.卖掉塔时,如果处于攻击状态,应该调用停止播放音效.
3.对于所有的魔法球塔,音效应该采用类此引用计数的方法,每调用一次``++m_uReference;``,每停止一次``--m_uReference;``,当``m_uReference==0``时调用真正的停止音效方法.

嘿嘿,以上纯属个人见解,本人不对此负任何责任!



##总结
综上所述,我们在编写游戏音效逻辑的代码时,要注意:
1.循环音效多次播放在windows上有问题,可以考虑在android或ios端调试.
2.音效一定要preloadEffect.


**(全文完)**

[1]:http://ww1.sinaimg.cn/large/7f870d23jw1eb0uj0md4qj20sh0ixn32.jpg
[2]:http://ww1.sinaimg.cn/large/7f870d23jw1eb0wcy5p7pj20di09l74r.jpg
[3]:http://ww1.sinaimg.cn/large/7f870d23jw1eb0xzlrpzxj217s0hv44w.jpg
[4]:http://ww3.sinaimg.cn/large/7f870d23jw1eb1p27ln4tj208p0fkmx6.jpg