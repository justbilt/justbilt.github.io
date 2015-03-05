title: "cocos2d-x 在cocosbuider中添加CCBFile类型的控件"
id: 549
date: 2013-07-26 11:56:07
tags: 
categories: 
- cocos2d-x
---

这篇文章会讲解cocosbuilder的一些问题,会用到cocosbuilder的相关知识,如果您对cocosbuilder 不熟悉的话建议您先学习cocosbuilder!
<!--more-->


### 一.界面部分

使用过cocosbuilder的同学都知道每一个界面会对应一个.ccb文件,将好几个界面都重复拥有的东西单独制作一个ccb界面,然后以ccb控件的方式加入到其他界面中.

**1.如下,我的游戏中暂停,胜利,失败界面都会有一个分享的东西.**

[![hhh]({{BASE_PATH}}/images/12320f88fec0ab1934bff002ce95b0d071552874.png)](http://blog.justbilt.com/wp-content/uploads/2013/07/hhh.png)



如上图所示,如果我每个界面都去添加这些素材,按钮什么的,必定十分费神费力,遂我们不如将它单独作为一个页面,然后让每个用到的页面去引用它!



**2.如下图所示,我们创建一个单独的页面,命名为SharedLayer.ccb(点击查看大图):**

[![003]({{BASE_PATH}}/images/7ccf3dffbd0811a76a73824f9ee0c09e62e322a9.jpg)](http://blog.justbilt.com/wp-content/uploads/2013/07/003.jpg)



对于这个界面,我们需要注意到一下几点:

*   创建时选择根节点为CCNode,而不是默认的CCLayer,因为CCLayer一旦创建就不能改变尺寸!
*   和其他界面相同的方式去绑定类,变量,函数!
*   尽量做得CCNode的尺寸和实际内容的尺寸一致!
*   综合图中4,6所示,在position(0,0),anchor(0,0)的情况下,CCNode的中心会在背景黑框的左下角,这个可能是ccb的bug,正常!


**3.完成后我们就可以在其它界面引用SharedLayer.ccb界面了(点击查看大图):**

[![004]({{BASE_PATH}}/images/3c210540a793b2fd7656eede96b544d07d6e89fe.jpg)](http://blog.justbilt.com/wp-content/uploads/2013/07/004.jpg)



对于这个界面,步骤如下:

1.  如图中1所示,点击右上角绿色树叶的按钮,添加CCBFile类型的控件!
2.  选择图中2的控件,在右侧3位置选择对应的ccb文件(即SharedLayer.ccb)!
3.  注意,在这个界面是不能够调整SharedLayer.ccb中的东西的,只能调整这个CCBFile的位置,缩放等!
4.  调整好位置后效果会如第一张图片所示!




### 二.程序部分

**1.在这里先要说下小弟的文件布局:**

[![001]({{BASE_PATH}}/images/285d067a8786f295e4868d61cef2f6f73a8e0c7e.png)](http://blog.justbilt.com/wp-content/uploads/2013/07/0011.png) [![002]({{BASE_PATH}}/images/186c81f61c821b10e114879ffc0061ae64098e28.png)](http://blog.justbilt.com/wp-content/uploads/2013/07/0021.png)

不知大家能否看的懂?主要就是

1.  proj.w32目录同级多了个proj.cocosbuilder的cocosbulider的工程文件夹,所有的界面文件都存在proj.cocosbuilder目录下的Resources文件夹中,当然这个文件夹名字可以随便起!
2.  cocosbuilder设置导出目录为../Resources/ccbResources目录,为什么这么做呢?因为如果散在Resources目录中的话非常乱!
3.  同时cocosbuilder添加资源目录为../Resources/ccbResources!


**2.下面让我们看下程序中的加载吧!**

1).对于普通的ccb的加载可以用到下面的这个函数:
```c++
CCNode *createCCBLayer(LPCSTR szClass, CCNodeLoader *pLoader, LPCSTR szName, CCNode *pOwner)
{
	CCNodeLoaderLibrary *ccNodeLoaderLibrary = CCNodeLoaderLibrary::newDefaultCCNodeLoaderLibrary();
	cocos2d::extension::CCBReader *ccbReader = NULL;
	ccNodeLoaderLibrary->registerCCNodeLoader(szClass, pLoader);//注册类名和loader
	ccbReader = new cocos2d::extension::CCBReader(ccNodeLoaderLibrary);
	ccbReader->autorelease();
	ccbReader->setCCBRootPath("ccbResources");//因为我的资源没有位于根目录中,所以需要设置下ccb的root路径!
	return ccbReader->readNodeGraphFromFile(szName, pOwner);
}
```
2).加载的时候调用如下代码即可:
```c++
CCNode *pLayer = createCCBLayer("CPauseLayer", CPauseLayerLoader::loader(),"PauseLayer.ccbi", CCDirector::sharedDirector()->getRunningScene());
pScene->addChild(pLayer);
```


3).但是我们现在的这个PauseLayer还包含着另一个界面,所以只注册一个类名和loader是不行的!于是貌似我们把函数改成如下的就OK了?
```c++
CCNode * CLoadingScene::createCCBLayer(LPCSTR szClass1, cocos2d::extension::CCNodeLoader *pLoader1,LPCSTR szClass2, cocos2d::extension::CCNodeLoader *pLoader2, LPCSTR szName, CCNode *pOwner)
{
	CCNodeLoaderLibrary *ccNodeLoaderLibrary = CCNodeLoaderLibrary::newDefaultCCNodeLoaderLibrary();
	cocos2d::extension::CCBReader *ccbReader = NULL;
	ccNodeLoaderLibrary->registerCCNodeLoader(szClass1, pLoader1);//类1 和loader1
	ccNodeLoaderLibrary->registerCCNodeLoader(szClass2, pLoader2);//类2 和loader2
	ccbReader = new cocos2d::extension::CCBReader(ccNodeLoaderLibrary);
	ccbReader->setCCBRootPath("ccbResources");	
	ccbReader->autorelease();
	return ccbReader->readNodeGraphFromFile(szName, pOwner);
}
```
 

4).但是结果是你只能得到崩溃!这个问题困扰我足足有1整天时间,最后一直断点追中的很深才明白的!原来ccbreader内部也是按照如下的方式去处理CCBFile类型的控件的:
```c++
cocos2d::extension::CCBReader *ccbReader = NULL;
ccbReader = new cocos2d::extension::CCBReader(ccNodeLoaderLibrary);
ccbReader->autorelease();
return ccbReader->readNodeGraphFromFile(szName, pOwner);
```
只不过它少了这一句:
```c++
ccbReader->setCCBRootPath("ccbResources");
```
于是资源位置就会错误,自然会崩溃!



5).解决这个问题有如下两个解决方案:

**1.修改ccbreader源代码:将setCCBRootPath内部的mCCBRootPath改为静态变量,这样所有的ccbreader就会公用一个rootpath!**

**2.彻底在程序中抛弃:ccbReader->setCCBRootPath("ccbResources")这行代码!**

不想改源代码,所以我采用的第二种解决方案!最简单的做法是将ccbResources目录中的文件全部拷贝出来放在Resources目录下就大功告成!但是我们的Resources目录下还可能防其他的东西啊,比如声音文件,数据文件,及游戏中的图片文件,这样子会很乱,而且cocosbuilder很容易出现递归查找的错误!

最终做法如下,将ccbResources和ipad文件夹同时放在Resources路径下,设置文件查找路径为:
```c++
vector<string> searchPath=CCFileUtils::sharedFileUtils()->getSearchPaths();
searchPath.insert(searchPath.begin(),"ipad");
searchPath.insert(searchPath.begin(),"ccbResources");
CCFileUtils::sharedFileUtils()->setSearchPaths(searchPath);
```
这样我将游戏中的所有文件全放在iapd路径下,而cocosbuilder的导出文件全放在ccbResources目录下!

设置:cocosbuilder的导出路径为:../Resources/ccbResources

文件布局截图:

[![QQ截图20130726114948]({{BASE_PATH}}/images/932e98efedae8f6c3fff8cda6c9aead6a4db92f6.png)](http://blog.justbilt.com/wp-content/uploads/2013/07/QQ截图20130726114948.png)







文章下半部分比较晦涩,有哪里看不太明白,欢迎指出!!!



(全文完)