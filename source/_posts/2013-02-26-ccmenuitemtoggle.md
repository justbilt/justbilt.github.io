title: cocos2d-x开关按钮三态按钮的实现CCMenuItemToggle
date: 2013-02-26 15:13:41
tags: [cocos2d-x,CCMenuItemToggle]
categories: [cocos2d-x]
---

# 一.开关按钮:

## 1.效果图:

![][1]

![][2]

## 2.创建

从源码种我们可以看到`CCMenuItemToggle`有3个创建函数,如下:

```c++
static CCMenuItemToggle * createWithTarget(CCObject* target, SEL_MenuHandler selector, CCArray* menuItems);
static CCMenuItemToggle* createWithTarget(CCObject* target, SEL_MenuHandler selector, CCMenuItem* item, ...);
static CCMenuItemToggle* create();
```
第一个和第二个类似,需要传入**回调目标**和**回调函数**,然后是一组状态,每一个状态都是一个`CCMenuItem`,每个状态可以是一张图片`CCMenuItemImage`,一个文字`CCMenuItemLabel`.

这里为什么是一组状态而不是两个状态是因为: CCMenuItemToggle不仅可以实现开关按钮,还可以实现多态按钮.

第三个创建函数什么参数都没有,后面可以调用`setTarget`来设置回调,通过`addSubItem`来添加状态,这个后面会说到.

下面我们来看一段具体的创建代码:

```c++
//创建
CCMenuItemToggle *btnSnd =CCMenuItemToggle::createWithTarget();

//设置回调
btnSnd->setTarget(this,menu_selector(GameLayer::onMusicEnable));

//添加状态
btnSnd->addSubItem(CCMenuItemImage::create("soundoff.png","soundon.png"));
btnSnd->addSubItem(CCMenuItemImage::create("soundon.png" ,"soundoff.png"));

//添加到CCMenu中
CCMenu* pMenu = CCMenu::create(btnSnd, NULL);
    pMenu->setPosition(CCPointZero);
    this->addChild(pMenu);
```

## 3.回调代码:

回调的参数就是`CCMenuItemToggle`,只是需要强转一下:

```c++
void GameLayer::onMusicEnable( cocos2d::CCObject* pObj )
{
	CCMenuItemToggle *toggleItem = (CCMenuItemToggle *)pObj;
	CCLog("%d",toggleItem->getSelectedIndex());
}
```

# 4.按钮状态

获取按钮的当前状态使用`getSelectedIndex`函数,返回的是状态的id,这个id是由创建时传入状态的顺序决定的,从0开始,依次递增,我们可以通过下面几行代码做逻辑判断:

```c++
int nID=toggleItem->getSelectedIndex();
swith(nID)
{
case 0:
    //...
    break;
case 1:
    //...
    break;
}
```

同样的,我们可以调用`setSelectedIndex`设置按钮当前的顺序:

```c++
toggleItem->setSelectedIndex(0);
toggleItem->setSelectedIndex(1);
```

# 二.多态按钮:

多态按钮的实现非常简单,我们在创建开关按钮时传入了两个状态,这样状态的切换顺序就是:` 1 --> 2 --> 1 `

如果我们创建时传入多个状态,那么状态的切换顺序就是:` 1 --> 2 --> .. --> N --> 1 `, 这样多态按钮就实现了.


#完

[1]:{{BASE_PATH}}/images/90463242a2984681963753c2165c80ae28ed4d28.png
[2]:{{BASE_PATH}}/images/26c9d29cefc1e43b42e1c4de5a5d82a033367966.png

