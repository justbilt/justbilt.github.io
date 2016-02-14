title: 在Command Line中使用TexturePacker
date: 2013-12-12 10:33:02
tags: [TexturePacker, Tool]
---

TexturePacker是一个非常好用的小图合并工具,介绍它的文章非常多,多数都是使用GUI工具的,但是:
1. 如果原始图片发生了改变,我们就需要重新手动拼接一下,麻烦.
2. 使用GUI界面**非常不高端**,我高大猿族绝对首选使用Command Line啊.

安装TexturePacker会附带一个command line工具,让我们一起学习一下如何使用吧.
<!--more-->

##环境搭建
我一般首选在Windows下开发(不要打我),这里就只讲Windows环境的配置

1.先去[这里][3]下载安装文件,完成后一路无脑下一步,OK.

2.大家在``C:\Program Files\TexturePacker\bin``目录下可以看到两个exe文件,``TexturePacker.exe``和``TexturePackerGUI.exe``,前者是命令行工具,后者是GUI工具.

3.默认``TexturePacker``是没有加到环境变量中的,我们需要手动来,右键点击**计算机**->**属性**->**高级系统设置**->**环境变量**,找到``PATH``后将``C:\Program Files\TexturePacker\bin``添加到末尾,注意要在前面加``;``.

4.运行cmd,输入``TexturePacker``大家应该能够看到下面的内容:
![][1]
如果不能的话请检查拼写和第``2``步.大家可以仔细阅读一下内容,写的十分详细,用过TexturePacker的话基本上都能对应起来.


##小试牛刀
你过你看完打印出的信息的话可以发下末尾部分有三个示例,如下:
![][2]
看起啦十分简单,让我们试一下吧,感谢微博小伙伴[@sosoayaen][4]发现的小秘密,使得我们可以快速的在任意位置打开命令行.找一个有这一堆图片的文件夹的上层目录,按住``Shift``点击``右键``,会发现一个菜单项``在当前目录打开命令行``,打开后按照示例输入``TexturePacker 001/*.png``,然后如示例说的:

> creates out.plist and out.png from all png files in assets
 trimming all files and creating a texture with max. 2048x2048px

实际情况却不完全是这样的,你可能会得到这样的错误:

![][5]
卧槽,这是什么情况啊,明明就是按照官方的示例来的嘛,google完全没有任何答案啊,经过一番痛苦的实验后,终于发现了原因所在,这里不能输入:
```
TexturePacker 001/*.png
```
而应该是这个样子:
```
TexturePacker 001
```

**这个问题可能是由于我使用的版本太旧或使用Windows导致的,如果你没有遇到,那么最好!**


##参数详解
本想着把所有参数都讲一遍,写到一半发现卧槽太多了,而且好些都用不到,这里挑一些常用的分析下,**以下内容都有进行测试**,还是比较准确的,如果有问题,欢迎指出.
注:
1. 选项含有``<...>``的选项表示含有参数需要填写.
2. 粗体表示比较重要的选项.

####输出
---

**--sheet** ``<filename>``
+生成的图片名,支持**png,jpg,pvr,pvr.czz,pvr.gz**格式
+示例:``--sheet out.png``

**--data** ``<filename>``
+ 生成的plist文件名 
+ 示例:``--data out.plist``

**--format** ``<format>``
+ 生成的plist格式,我们使用cocos2d格式 
+ 示例:``--format cocos2d``
+ 注:其他支持格式见下图:
![][6]

**--auto-sd**
+ 自动生成sd资源
+ 示例:``--auto-sd``
+ 注:这个要注意一点,如果要使用这个参数,你输入的``sheet``和``data``名必须含有``-hd``或``@2x``,TP会自动生成不带后缀的sd数据.

--texturepath ``<path>``
+ 在生成的``sheet``文件的路径前加你<path>
+ 示例:``--texturepath image/tower`` 这样在plist文件中``realTextureFileName``的值为``image/tower/out.png``
+ 注:这个参数主要用在当你的**图片与plist文件不再同一个目录**时使用,不会改变``out.png``的目录

--trim-sprite-name
+ 剪裁掉拼接图片的后缀名
+ 示例:``--trim-sprite-name`` 这样在plist文件中``<key>001.png</key>``会变成``<key>001</key>``
+ 注:是剪裁用来拼接的文件而不是生成文件,如果你的资源管理类似于android那样,使用图片时不加后缀名,那么打包时可以使用这个选项

**--replace** ``<regexp>=<string>``
+ 按照原文的翻译是使用``<string>``替换掉拼接图片的文件名中正则表达式匹配的字符串
+ **卧槽,正则表达式啊一听就尼玛高大强啊,可惜老夫不会啊,怎么办呢?回家去翻翻金瓶梅改天告诉大家.**
+ 这个TM太有用了,后面我遇到的那个问题,用这个来解决最好不过了

--ignore-files ``<regexp>``
+ 按照原文的翻译是忽略所有满足给定条件的图片(可以使用时间作为条件),你可以使用``*``或``?``,但在使用bash时应避免使用通配符.


####尺寸
---
先上一张cocos2d-x支持的最大图片尺寸:
![][7]
还有官方的这句话:
>  For the developers, if you want to cross platforms and run your games smoothly, you should keep your texture size **less than 1024*1024**, that is the lowest restriction for most machines.

**--width/--height** ``<int>``
+ 两个参数,放在一块说了,设置输出图片的宽度/高度
+ 示例:``--width 100`` ``--height 100``
+ 注:这个值设置的大了无所谓,会产生空白区域,但是如果太小,就会报错:
```
error: Could not fit all sprites into the sprite sheet.
```

**--max-width/--max-height/--max-size** ``<int>``
+ 设置输出图片的最大宽度/高度/尺寸
+ 示例:``--max-width 1024`` ``--max-height 1024`` 前面两个等价于后面 ``--max-size 1024``
+ 注:
	1.和上面两个参数的区别在于告诉TP实际值别超过这个值就OK,而上边那两个参数告诉TP实际值一定是这个.
	2.默认值为``2048``
	3.如果实际值大于设置的最大值会产生错误:
```
error: Sprite sheet size is too small.
```

**--allow-free-size**
+ 允许输出图片不是2的幂,以最小尺寸输出
+ 示例:``--allow-free-size``
+ 注:这个一般开启,cocos2d-x2.0开始就已经支持图片不是2的幂了


####间距和旋转
---

--shape-padding ``<int>``
+ 图块之间缝隙的宽度,默认值是``2``
+ 示例:``--shape-padding 100``

--border-padding ``<int>``
+ 可以理解为边框的宽度,默认值为``2``
+ 示例:``--border-padding 100``

--padding <int>
+ 间距,这个参数等价于上面两个参数同时同时作用
+ 示例:``--padding 100``
+ 注:如果没有开启``--allow-free-size``可能和你想象的不太一样

--inner-padding <int>
+ 试了一下,这个参数的作用应该是给每个sprite的周围加一个边框,默认值为0
+ 示例:``--inner-padding 100``
+ 注:上面的这几个参数作用都不是很大

--enable-rotation/diable-rotation
+ 开启/关闭旋转,默认值和输出的格式有关系,cococ2d格式默认enable
+ 示例``--enable-rotation`` ``--diable-rotation``
+ 注:这个很好理解,为了排版更密集些,有的图片会被旋转

**--trim/no-trim**
+ 剪裁图片,即移除图片周围的透明像素,保留原始尺寸,默认开启
+ 示例:``--trim`` ``no-trim``
+ 注:这个要格外注意一下,这个参数略**微有些问题**,如果没有了解带来的后果的话还是使用``--no-trim``更安全些,我会在后面的仔细讲一下.

**--crop**
+ 与上面的一条类似,移除图片四周的透明像素,不保留原始尺寸,保存为一张更小的图片
+ 示例:``--crop``
+ 注:同上,要小心使用,尽量不在这里使用,而是改为前期用其它工具处理

--trim-threshold <int>
+ 与Trim类似,只是这个选项有一个参数,表示剪裁掉alpha值小于这个参数的像素,取值``0~255``,默认为``0``.
+ 示例:``--trim-threshold``

**--disable-auto-alias**
+ 关闭自动命名,什么意思呢?**TP在打包时会自动识别相同的图片,最终在大图里只会保留一张**,这样会更加的节省资源,这个参数将会关闭这个功能
+ 示例:``--disable-auto-alias``
+ 注:这样参数还是**不要设置**的好

####其他常用选项
---

**--opt** <pixelformat>
+ 设置输出图片的像素格式 一般默认RGBA8888
+ 示例:``--opt RGB444``
+ 注:这个选项一般不做更改,如果想压缩资源体积的话,可以改为RGBA4444这样图片可以减小一半的体积.具体大家可以看下面这张图:
![][8]


##常见问题

**错误:** error: Error in sprite: *.png: Failed to load image!
**解决方案:** 去掉目标路径末尾的*.png试试.

**错误:** error: Could not fit all sprites into the sprite sheet.
**解决方案:**
1.查看有无设置``--width or --height``,这个错误通常是由于输出图片的尺寸太小导致的.
2.查看有无``--max-width/--max-height/--max-size``,没有的话加上,有的话将参数值改大一些.

**错误:** error: Sprite sheet size is too small.
**解决方案:**参见上个错误中的解决方案2,``--max-width/--max-height/--max-size``默认值为``2048``,试着改成``4096``试试,如果解决了,不要高兴,因为大多数移动设备都不支持这个尺寸,可以考虑分开打包.

**错误:** error: Unknown argument --XXX - please check parameters or visit http://www.texturepacker.com for newer version
**解决方案:** 检查XXX的拼写是否正确

**问题:** 程序中获得图片的尺寸与打包前不一致
**解决方案:** 检查参数是否含有``--crop``,有的话删除,有没有``--no-trim``,没有的话加上.

##后记
讲了那么多参数,其实常用的没有几个,下面这个是我最终使用的命令:
```
TexturePacker --sheet out.png --data out.plist --allow-free-size --no-trim --max-size 1024 --format cocos2d animation
```

文章到这里就要结束了,但其实还没有完,``--trim``到底为什么不建议使用?使用了会发生什么问题?它和``--crop``的具体区别在哪里?plist文件中frame的路径和其他plist的重复了怎么办?程序中怎么做才能做到打包和不打包的差异最小化?金瓶梅你什么时候看完?

我后面会再写一篇文章来讲这些东西(逃).


**全文完**


[1]:http://ww1.sinaimg.cn/large/7f870d23jw1ebgr9fza56j20mo08rabj.jpg
[2]:http://ww4.sinaimg.cn/large/7f870d23jw1ebgrdsac69j20qj06rgmj.jpg
[3]:http://www.codeandweb.com/texturepacker
[4]:http://weibo.com/1061005610/A8lXXwagW
[5]:http://ww1.sinaimg.cn/large/7f870d23jw1ebgt6ya8d9j20iq0820te.jpg
[6]:http://ww1.sinaimg.cn/large/7f870d23jw1ebgwbnyra9j20fx083aay.jpg
[7]:http://ww4.sinaimg.cn/large/7f870d23jw1ebh3kmq7ebj205i05awej.jpg
[8]:http://ww3.sinaimg.cn/large/7f870d23jw1ebi5nnszqmj20po0g0go5.jpg