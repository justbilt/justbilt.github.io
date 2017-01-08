title: 修改SimpleAudioEngine在win32下的实现
date: 2013-11-29 16:07:51
categories:
- cocos2d-x
- 音效
---

###2013年12月6日更新
试着跟OAE的作者联系了下,向他诉苦**必须装vs2013的运行库**的问题,没想到作者竟然回复了,而且很快出了一新的版本:

>I've updated the package on: OAE_1_6_0_7,It's still with Visual Studio 2013, but linked statically. So you don't need anymore Visual C++ redistributable to run Open Audio Engine.
Now it should really more portable :P

哈哈哈,实乃业界良心啊,这下小伙伴们**再也不用去安装vs2013的运行库了**,实测无误,大家快些更新吧!
<!--more-->

###2013年12月3日更新
感谢微博小伙伴[@Hewen_Xie][14]的纠正,在检测缓冲时没有使用FullPath,可能导致缓冲检测失效,已上传至[github][13],大家更新下就OK!

###2013年12月2日更新
之前文章有两个错误,更正下,小伙伴们对不住了!
1.**必须装vs2013的运行库**,之前说的那个不用装的方法是错误的,引擎作者很早就使用vs2013开发了,用别人的东西就是蛋疼,改天研究下openAL吧!

2.大量测试后发现OAE引**擎最大支持缓存256个声音,一旦超出后会卡死程序,**我写了缓冲池修复了这个问题,效率也有所提升.

##问题
在上篇文章中抱怨了下**SimpleAudioEngine**在win32下的实现,总结一下,有这么几个缺点:

1.**循环播放音效的返回id有问题,与其他平台存在差异.**

2.**每次播放音效都会开启一个线程,io操作,声音一多,卡的不成样子**

其中第2点是致命的,保卫萝卜1出过一个[桌面版][1],由于声音的问题,**开着声音几乎不可玩**,大家可以下一个感受一下:
![][2]

这样来看,难道真如王哲大神所说,**桌面版只是一个用来调试的版本**?

其实不然,因为cocos2d-x是开源的,而**SimpleAudioEngine**在不同平台下又有不同的实现,这样我们可以很容易的去修改win32下的实现.


##解决方案
想了下,决定先放出解决方案,下面的文章其实可看可不看的,最终我采用的是**免费不开源**的OAE声音引擎,基于OpenAL的二次封装,OOP,简单易用.大家只需几个步骤即可**完美解决**win32下的声音问题! ~~由于最新版的OAE使用vs2013开发,**必须装vs2013的运行库**!~~ **(已不用安装)**

**注意:OpenAE有一个缺点,就是只支持ogg文件.**

1.解压缩从github将[这个][15]拽下来!

2.**32位**系统将``lib/x86``目录下的``OpenAE.dll``,``OpenAL.dll``拷贝到``Debug.win32``目录下,64位类似.

3.用``win32/`` **替换掉**你工程``CocosDenshion/win32``目录下的所有文件.

4.大功告成,播放一堆声音试试吧.

5.游戏结束的时候一定要在AppDelegate的析构函数中调用``SimpleAudioEngine::end()``,不然会因没有释放内存而崩溃,如下:

```c++
AppDelegate::~AppDelegate()
{
	CocosDenshion::SimpleAudioEngine::end();
}
```

6.将``OpenAE.dll``,``OpenAL.dll``拷贝到``cocos2dx/platform/third_party/win32/libraries``中,这样重新编译时会自动拷贝到``Debug.win32``目录下.

好了,是不是十分容易,**下面的内容是写当时考虑的一些东西及实现的一些细节**,如果有什么错误和疑问,请指出,谢谢!


##声音引擎的选择
好的,如果你对win32下的声音播放比较熟悉的话,你可以看下SimpleAudioEngine现在的实现,试试能否改的更高效些!否则我们只能找一个现成的声音引擎了.

###FMOD
说起声音引擎,FMODE必须是首选啊,功能强大,跨平台,个人使用免费,简单易用,久经考验,好多牛逼的游戏引擎都使用它的,[官方网址][3],下面是使用它的引擎:

![][4]

但是由于商用是收费的,而且我们用不到那么多牛逼的功能,所以这个就pass掉咯~

###OpenAL
另一个特别有名的就收OpenAL(Open Audio Library)了,这个也是大名鼎鼎啊,让我们看看维基百科的介绍:

>OpenAL（Open Audio Library）是自由软件界的跨平台音效API。它设计给多通道三维位置音效的特效表现。其 API 风格模仿自 OpenGL。penAL 最初是由 Loki Software 所开发。是为了将 Windows 商业游戏移植到 Linux 上。Loki 倒闭以后，这个项目由自由软件／开放源始码社区继续维护。不过现在最大的主导者（并大量发展）是创新科技，并得到来自 苹果公司 和自由软件／开放源代码爱好者的持续支持。

现在官方网站貌似打不开,感兴趣的同学可以去[这里][5]看看.OpenAL十分强大,这就意味着要在短时间弄懂它并不容易,这里我们就不考虑它了.


###OpenAE
最终我选择了OpenAE(Open Audio Engine),一个基于OpenAL的声音引擎,面向对象,上手十分容易(我只用了10分钟就会用了),虽然不开源但是免费啊,这已经足够了.官方网站在[这里][6],同时大家可以看看codeproject上的[这个示例][7].

![][8]

##改动
其实要改动的地方很多,之前的实现基本要重写,下面我介绍下要点:

1.首先我们要初始化引擎,由于SimpleAudioEngine是单例,所以它的**构造函数**就挺好:

```c++
SimpleAudioEngine::SimpleAudioEngine()
{
	HINSTANCE lib = LoadLibrary("OpenAE.dll"); //加载引擎


	//得到设备列表地址
	oae::Renderer* (*driver)(const char*,const unsigned&) = nullptr;
	driver = (oae::Renderer*(*)(const char*,const unsigned&)) 
		GetProcAddress(lib, "GetRenderDevice"); 


	//得到设备列表
	const char* (*available)(unsigned int&) = nullptr;
	available = (const char*(*)(unsigned int&)) GetProcAddress(lib, "GetDeviceName"); 

	//打印可用设备的列表
	for(unsigned j=0; available(j)!=nullptr; j++)
	{
		CCLog(available(j));
	} 

	//默认使用第一个设别去创建Renderer,48000是端口号随便填
	unsigned choice=0; 
	oae::Renderer* dev = driver(available(choice), 48000);

	//这行代码十分诡异,貌似没有什么用处,但是不调用就是无法播放声音
	oae::Listener* lis = dev->GetListener();

	//由于创建声音和最终释放资源会用到这两个指针,所以我以static方式存在的MciPlayer中
	MciPlayer::lib=lib;
	MciPlayer::dev=dev;
}
```

2.更改MciPlayer的实现,对oae,每一个声音的实例是``oae::Screamer``,我们在``open``中去创建它:

```c++
void MciPlayer::Open(const char* pFileName, UINT uId)
{
	m_scr=dev->GetScreamer(pFileName);
	assert(m_scr!=NULL && "Only suppost .ogg file!");
	m_strFileName=pFileName;
	
	m_nSoundID=uId;
}
```

3.更改``playEffect``返回值streameID的生成方式,变为递增的就OK:
```c++
unsigned int _StreameID()
{
	static unsigned int nSteameID=0;
	++nSteameID;
    
    return nSteameID;
}
```

其他改动的地方也很多,大家可以直接去看源码,下面有下载地址.


##后话

具体实现什么的都在源码里,大家可以看一下,经过本人测试,**没有发现任何问题,并且已经应用到了现在的项目中**,如果遇到了什么问题,留言告诉我.


**(全文完)**





[1]:http://www.luobo.cn/pc/
[2]:http://ww2.sinaimg.cn/large/7f870d23jw1eb1z09u9awj20qn0bv0ul.jpg
[3]:http://www.fmod.org/
[4]:http://ww4.sinaimg.cn/large/7f870d23jw1eb1zrn20prj20ok05ht8z.jpg
[5]:http://kcat.strangesoft.net/openal.html
[6]:http://www.openaudioengine.com/
[7]:http://www.codeproject.com/Tips/674472/Open-Audio-Engine
[8]:http://ww2.sinaimg.cn/large/7f870d23jw1eb20ikphx4j20u00df75l.jpg
[9]:http://ww4.sinaimg.cn/large/7f870d23jw1eb216q34nuj20mc05cq3e.jpg
[10]:http://pan.baidu.com/s/1EddhF
[12]:http://www.openaudioengine.com/downloads/OAE_1_6_0_7.zip
[13]:https://github.com/justbilt/CocosDenshion_win32/tree/master/win32
[14]:http://weibo.com/1158210694/AlAcdvfWx
[15]:https://github.com/justbilt/CocosDenshion_win32/archive/master.zip