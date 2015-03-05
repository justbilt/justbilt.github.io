title: TiledMap Tips
date: 2013-10-16 14:01:26
tags: [TiledMap]
categories: [cocos2d-x]
---
### 一.断言 TMX: Only 1 tileset per layer is supported 崩溃:

<!--more-->

加载地图时弹出断言失败窗口,跟踪进去发现崩毁地点:
```c++
CCAssert( m_uMaxGID >= m_pTileSet->m_uFirstGid &&
    m_uMinGID >= m_pTileSet->m_uFirstGid, "TMX: Only 1 tileset per layer is supported");
```
从断言中的提示就可以看出:一张图层上只支持一个一个图块集合
就是说编辑器中的每一层只能使用一个图块集合中的图块,不能使用其他图块集合中的图块!





### 二.tileGIDAt返回数据异常:

我们用tileGIDAt获取某一层上的某一格对应的图块ID,GID是什么呢?可以理解为全局唯一ID,而我们的图块集合可能会有多个,所以每个的图块的ID不是从该图块集合1,2,3...这样的,而是紧接着上一个图块集合的最后一个ID顺序下来的!

所以我们要获得正确的ID,应该:
```c++
cocos2d::CCTMXLayer *towerLayer = map->layerNamed("tower");
cocos2d::CCTMXTilesetInfo *towerSet = towerLayer->getTileSet();
int nGid = towerLayer->tileGIDAt(ccp(0, 0)) - towerSet->m_uFirstGid;
if(nGid >= 0)
{
}
```
先获取这一层对应的图块集合的首ID,然后相减就获得了正确的ID.





### 三.拼接出现裂痕(黑线)

**1.快速解决方案**

修改ccConfig.h中的CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL:
```c++
#ifndef CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
#define CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL 1
#endif
```
大体的作用是用实际图片的99%来创建纹理,这样边缘一些细微化的差距就会被忽略掉!



**2.完美解决方案**

用1的方法来解决问题是最快的,但是会带来一些副作用,查找下CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL这个宏,会发现有4个地方都用到了:

[![QQ截图20131016112818]({{BASE_PATH}}/images/b39a481b6b66a40162dad2a2c52008f99129cb83.jpg)](http://blog.justbilt.com/wp-content/uploads/2013/10/QQ截图20131016112818.jpg)

分别是CCSprite,CCLabelAtlas,CCParticleSystemQuad,CCTileMapAtlas,其中只有CCTileMapAtlas的改变使我们想要的,而CCSprite是我们最常用的接口,这将会导致所有的图片都会使用99%来贴图,影响虽然不是很大,但是在某些情况下还是能够看的出来(点击查看大图):

正常:

[![2]({{BASE_PATH}}/images/4220b6514671a4567ca56c6e199f49070b019f31.jpg)](http://blog.justbilt.com/wp-content/uploads/2013/10/2.jpg)

开启后:

[![1]({{BASE_PATH}}/images/8de1d1d02a2860a7cc0eb5f17f4f99daf7a8a97a.jpg)](http://blog.justbilt.com/wp-content/uploads/2013/10/1.jpg)

上面的图是放大两倍后的效果,如果你不是太在意这些细节的话,就可以忽略这个问题了.

但是如果你有强迫症的话,可以移步这篇文章:

[http://blog.sina.com.cn/s/blog_4508e4860101dzkj.html](http://blog.sina.com.cn/s/blog_4508e4860101dzkj.html "http://blog.sina.com.cn/s/blog_4508e4860101dzkj.html")

同时我也修改好了一份,大家可以在文章末尾处下载!



### 四.滚动地图时会抖动

这种情况在RPG游戏中的影响很大,看一会眼睛就花了,我们可以给每一个节点的纹理开启抗锯齿来解决这个问题:
```c++
//开启抗锯齿
CCArray *pChildrenArray = map->getChildren();
CCSpriteBatchNode *child = NULL;
CCObject *pObject = NULL;
CCARRAY_FOREACH(pChildrenArray, pObject)
{
	child = (CCSpriteBatchNode *)pObject;
	if(!child)
		break;
	child->getTexture()->setAntiAliasTexParameters();
}
```


### 五.tilemap HD转SD

感谢[@子龙山人](http://weibo.com/1703959697/AdGzcoLTo?type=repost) 大大的分享,将tilemap从HD转SD可不是简单的将图片缩小一倍就OK的,地图文件中的网格尺寸啊什么的都要随着改变的!感兴趣的请移步这里:[http://wasabibit.org/WasabiBit/Dev_Notes.html](http://wasabibit.org/WasabiBit/Dev_Notes.html "http://wasabibit.org/WasabiBit/Dev_Notes.html")



 [10月25日]

### 六.加载崩溃

崩溃地点如下:
```c++
bool CCTexture2D::hasPremultipliedAlpha()
{
    return m_bHasPremultipliedAlpha;
}
```
原因:地图中有一层为什么都没有,**这是不允许的,要么将这一层删掉,要么刷点东西上去!**





**附件一:**

[http://pan.baidu.com/s/1qYnEQ](http://pan.baidu.com/s/1qYnEQ "http://pan.baidu.com/s/1qYnEQ")

下载后解压,覆盖cocos2dx目录下的同名目录:

**1.win32下,将CCSpriteTileMap.h和CCSpriteTileMap.cpp添加到libcocos2d工程中就OK!**

**2.android下,需要往cocos2dx/Android.mk中添加这么一行:**
```c++
tilemap_parallax_nodes/CCSpriteTileMap.cpp \
```
**3.ios下,需要将CCSpriteTileMap.h和CCSpriteTileMap.cpp添加到XCODE中!**





还有什么问题,发留言告诉我!