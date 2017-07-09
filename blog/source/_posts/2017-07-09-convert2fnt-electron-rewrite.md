title: 使用 electron 重写 convert2fnt
date: 2017-07-09 12:54:42
tags: [electron, convert2fnt]
---


大概是 14 年 2 月份的时候, 我使用刚学会 Python 写了一个小工具: [convert2fnt][2], 为此还写了一篇文章 [将一堆图片转化为BMFont工具][1] 介绍这个工具. 它的主要应用场景是这个样子的:

> 美术妹子出了一堆图片字, 但是在程序中使用 BMFont 是更加方便的, 这个时候你可以强硬的要求美术妹子重新用 Glyphdesigner 制作一份字体. 但是也可以很温柔的告诉她: "你先去忙吧, 剩下的交给我了.", 然后在妹子崇拜的目光下, 转身离去, 深藏功与名. 

恩, 最初版的工具确实能够达到这个目的. 只是过程可能略微麻烦一下.

[1]: /2014/02/01/images_to_bmfont/
[2]: https://github.com/justbilt/convert2fnt