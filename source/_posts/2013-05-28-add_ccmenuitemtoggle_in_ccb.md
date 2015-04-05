title: ccb中添加CCMenuItemToggle开关按钮
date: 2013-05-28 15:55:47
tags: [cocos2d-x,CCMenuItemToggle]
categories: [cocos2d-x]
---

# 零.预备知识

1. cocosbuilder(以下简称ccb)
这篇文章建立在有一定cocosbuilder使用经验的基础上，如果没有请先学习下cocosbuilder

<!--more-->

## 一.问题

**开关按钮**(switchbutton)在游戏开发种十分常见,cocos2d-x(简称2dx)中对应的接口是:`CCMenuItemToggle`,这个类十分易用




### 2.本文用到了CCMenuItemToggle类，不懂的可以看我之前的教程：

[http://blog.justbilt.com/?p=62](http://blog.justbilt.com/?p=62)



### 3.准备两张图片（off／on）



![](/images/2ee36e37566e1d35173ba79e46700f36b6db4660.png) ![](/images/907d08f499ade41d2d6cc22de817d9e737defedd.png)



### 4.新建ccb工程，新建文件，添加CCMenu，CCMenuItem，分别选择normal和selected的图片为on和off：



![](/images/e273dc58713a49b220aaff7658e7350f297dffa8.jpg)

为了在程序中能够找到这个按钮，我们可以采用两种办法：1.绑定变量  2.设置Tag，为了方便我采用的是设置tag的方式！

我们需要先设置CCMenu的Tag，然后设置CCMenuItem的Tag，分别为999和1000把！





###

### 5.在ccb中按钮的类型为CCMenuItemImage,我们在程序中要把它替换为CCMenuItemToggle:

1).下面这个函数会用CCMenuItemImage的图片来创建CCMenuItemToggle:
```c++
cocos2d::CCMenuItemToggle* CGlobal::createSwitchButton( CCMenuItemImage* pItemImage ,cocos2d::CCObject* target, cocos2d::SEL_MenuHandler selector )
{
	CCSprite* pNormalSprite=(CCSprite*)pItemImage->getNormalImage();
	CCSprite* pSelectedSprite=(CCSprite*)pItemImage->getSelectedImage();

	CCMenuItemImage* pNormal=CCMenuItemImage::create();
	pNormal->setNormalSpriteFrame(CCSpriteFrame::createWithTexture(pNormalSprite->getTexture(),pSelectedSprite->getTextureRect()));
	pNormal->setSelectedSpriteFrame(CCSpriteFrame::createWithTexture(pSelectedSprite->getTexture(),pNormalSprite->getTextureRect()));

	CCMenuItemImage* pSelected=CCMenuItemImage::create();
	pSelected->setNormalSpriteFrame(CCSpriteFrame::createWithTexture(pSelectedSprite->getTexture(),pNormalSprite->getTextureRect()));
	pSelected->setSelectedSpriteFrame(CCSpriteFrame::createWithTexture(pNormalSprite->getTexture(),pSelectedSprite->getTextureRect()));

	return CCMenuItemToggle::createWithTarget(target,selector,pNormal, pSelected,NULL);
}
```
2).替换掉CCMenuItemImage的pos,tag,scale,parent属性,有的我没有替换,大家可以自己加:
```c++
CCMenuItemToggle* CGlobal::createSwitchButtonWithItemImage( CCMenuItemImage* pItemImage,cocos2d::CCObject* target,cocos2d::SEL_MenuHandler selector )
{
	CCMenuItemToggle* pToggle=createSwitchButton(pItemImage,target,selector);
	pToggle->setPosition(pItemImage->getPosition());
	pToggle->setTag(pItemImage->getTag());
	pItemImage->getParent()->addChild(pToggle,pItemImage->getZOrder(),pItemImage->getTag());
	pItemImage->removeFromParent();

	return pToggle;
}
```


### 3).在onNodeLoaded中调用:


```c++
#define kCCBTag_Menu 999
#define kCCBTag_Music 1000

void CGameMenuLayer::onNodeLoaded( CCNode * pNode, CCNodeLoader * pNodeLoader )
{
	CCMenuItemToggle* pMusic=CGlobal::createSwitchButtonWithItemImage(((CCMenuItemImage*)getChildByTag(kCCBTag_Menu)->getChildByTag(kCCBTag_Music)),this,menu_selector(CGameMenuLayer::onBtnMusic));
	if (!CGlobal::s_pProfile->isMusicEnable())
	{
		pMusic->setSelectedIndex(1);
	}
	return ;
}
```


### 4).回调函数:

```c++
void CGameMenuLayer::onBtnMusic( CCObject *pobj )
{
CCMenuItemToggle* pMusic=(CCMenuItemToggle*)pobj;
CGlobal::s_pProfile->setMusicEnable(!pMusic->getSelectedIndex());
}
```


### 6.类似的可以实现3态按钮的,只需要在ccb中选择disable图片,在程序中替换CCMenuItemToggle的第三态图片,这样就可以实现三态按钮在ccb中的编辑!大家有好的建议也可以评论给我!如有错误,敬请指出!



全文完