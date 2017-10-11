title: 订阅 Hexo 的主题页
date: 2017-10-09 21:08:54
tags: [blog, hexo]
---

作为一个经常换博客主题的人, 时不时的就会去 [Hexo 的主题页][1] 逛逛, 看看有什么新的主题, 膜拜下大触们的作品.

比较麻烦的是这个页面并不是按照更新时间排序的, 也没有提供按时间排序的功能, 所以只能靠记忆力了. 这样很容易错过新的主题, 而且每次都是从头找起, 效率极低.

我自己是一个 Feed 的重度用户, 几乎是我扩展学习的唯一途径. 那么能不能将这个界面做成一个订阅源呢? 

<!--more-->

# 1. Feed43

刚好看到我订阅的 [@XA技术不宅][2] 兄更新了一篇文章 [利用 Feed43 制作什么值得买众测产品的RSS订阅链接][3], 感觉挺不错的, 我也试试吧.

![][5]

网站看起来十分简洁, 我们点击 `Create your own feed` 开始制作订阅源, 同意一大堆看不懂的条款后, 我们会来到这样的一个界面:

![][6]

我们在 `Address` 填入 Hexo 主题页的网址, 点击 `Reload` 就可以加载网页了, 成功后我们在下面填入提取规则, 主要就是 `Item (repeatable) Search Pattern` 这一栏, 其他都可以空着.

经过观察及一番思考后, 我填写的规则如下:

```
<li class="plugin on">
{*}<div class="plugin-screenshot">
{*}<noscript>
{*}<img data-src="{%}" {*}>
{*}<a href="{%}" class="plugin-preview-link" target="_blank"><i class="fa fa-eye"></i></a>
{*}<a href="{%}"{*} class="plugin-name" target="_blank">{%}</a>
{*}<p class="plugin-desc">{%}</p>
{*}
</li>
```

结果如下:

```
{%1} = https://hexo.io/build/screenshots/Ada-139a3754c7.jpg
{%2} = https://shuirong.github.io/
{%3} = https://github.com/shuiRong/hexo-theme-Ada
{%4} = Ada
{%5} = Ada is an concise theme for Hexo.
```

这里面提取了每一个主题的 `预览图片`, `预览地址`, `Github 项目地址`, `主题名称`, `主题简介`. 下面就是填写渲染规则了, 也很简单, 薛微有一点 html 基础就可以搞定. 最后的成果如下:

![][7]

还不错哈, 有模有样的, 大家感兴趣的话 [点击这里][8] 可以跳转查看. 仔细查看的话会发现一个问题:

> 数量比官网少了很多, 这里大概只有不到 20 个主题, 而官网上则有数 154 个主题.

在字体在后台看了下, 发现了这句话:

> Page too big. First 102400 bytes loaded.

Feed43 为了加快速度, 只节选了原始网页的一部分内容, 这下完蛋了, 这个订阅源没有任何作用, 权当练手了, 反正也没有花费太多的时间.


# 2. hexojs/site

[hexojs/site][9] 这个 Github 仓库是 hexo.io 这个网站的原始代码, 其中 [source/_data/themes.yml][10] 这个文件是主题页的原始数据文件.

我们可以通过观察这个文件的[历史记录][11]来得知主题的变化情况:

![][12]

那么如何订阅这个文件的变化呢? stackoverflow 上的[这个答案][13]可以解吾辈疑惑. 

> 在文件 url 后面加上 `.atom` 就可以生成这个 url 的 feed 源

大家可以点[这里查看][14], 复制 url 订阅, 效果很给力:

![][15]


---

经过这次折腾, 收获颇丰呀:

1. 会使用 Feed43 生成自己喜欢网页的订阅源
2. 知道了 Github 在任意 url 后面加 .atom 可以生成订阅源

生命不息, 折腾不止.

[1]: https://hexo.io/themes/
[2]: https://aoxuis.me/
[3]: https://aoxuis.me/post/bo-ke/li-yong-feed43-zhi-zuo-shi-yao-zhi-de-mai-zhong-ce-chan-pin-de-rssding-yue-lian-jie
[4]: https://feed43.com/
[5]: https://ws4.sinaimg.cn/large/006tKfTcgy1fkcbou91p9j30j70izq89.jpg
[6]: https://ws3.sinaimg.cn/large/006tKfTcgy1fkcbu7z8lbj30ix0ah75z.jpg
[7]: https://ws4.sinaimg.cn/large/006tKfTcly1fkcc1se8wrj30iw0rlq5q.jpg
[8]: https://feed43.com/8308348441438115.xml
[9]: https://github.com/hexojs/site
[10]: https://github.com/hexojs/site/blob/master/source/_data/themes.yml
[11]: https://github.com/hexojs/site/commits/master/source/_data/themes.yml
[12]: https://ws1.sinaimg.cn/large/006tKfTcly1fkdgeh5qyrj30sw0w3n1j.jpg
[13]: https://stackoverflow.com/a/7353586/3003184
[14]: https://github.com/hexojs/site/commits/master/source/_data/themes.yml.atom
[15]: https://ws4.sinaimg.cn/large/006tKfTcly1fkdgm3o20zj30v90rc431.jpg