title: 编写高质量的代码
date: 2016-04-17 20:51:28
tags:
---

我将高质量代码分为三个层级: **可读**, **健壮**, **优美**.

然而非常可惜的是, 我遇到的绝大多数程序员甚至都没有达到**可读**的层级. 这或许与我长期混迹在小公司有关系的经历有关系, 这些公司的满脑子想的都是怎么在严酷的市场竞争中存活下来, 怎么可能会将代码质量这种大后期才会遇到的坑放在第一位? 至于代码风格, 代码审查, 技术交流, are you kid me ?

而在这些公司的任职的程序, 绝大多数都是刚毕业的新人, 或者刚从培训学校出来的同学. 偶见一些老人, 也被长期加班/频繁变动需求/糙快猛的公司理念折磨的身心疲惫, 写代码也开始不那么讲究了, 开始复制黏贴, 开始到处临时代码, 注释中也充满了各种脏话. 

对于新人, 我有两点建议:

1. 严格要求自己, 对脏代码零容忍
2. 独立思考学习


对于第一点, 我曾经的很多同事, 技术实力很强, 但写出来的代码却很乱, 告诉他们后. 他们很坦然道, "我知道这段代码写的很冗余, 我也知道怎样写更好, 但是就是没有时间改." 或者 "代码写的那么整洁有什么用, 最后还是会随着需求变化修改, 有这时间我用来干别的多好.". 其实他们就是没有养成良好的习惯, 重构的多了, 很多简单错误随手写出来的代码就会避免. 而且这个脏代码, 越堆越多, 越来牵扯越广, 越无法重构. 等到那天需要改的时候, 就傻逼了, 改个变量得改十几处.

第二点也尤为重要, 到底什么样的代码才是健壮, 优秀的代码 ? 一行解决问题的代码背后的意义到底什么 ? 可能导致这个 bug 的原因有哪些, 如何排查 ?

OK, 蛋逼了这么久, 什么样代码才是优秀代码?

# 一. 可读性

可读性是一个合格的程序所必备的, 它决定了你代码的第一印象, 如果你连这个层次都没有达到, 那么就要努力了, 不然阅读/接手你代码的人可能会偷偷骂你哟 ~

## 1. 空白

基本上一个人的水平从空白就可以看出来大半, 因为空白错误并不会导致编译运行错误, 所以没有被重视起来, 但它却是你阅读代码很重要的一环. 空白的形式主要有两种 `空格` 以及 `空行`, 空格可能还好, 因为好多语言从语法层面就有强制要求, 换行绝对是新同学所犯错最严重的一点. 

**空格**主要是分隔一行内独立意义的语句, 可以是一个符号, 一个参数, 一个条件判断.

**空行**的作用和写作中的段落相似, 起着隔离不同功能代码作用. 比如一个函数内的操作, 我们从 参数检查/更新数据/更新界面 等等都需分隔开来, 甚至某一项操作的东西太多都需要分隔下. `滥用空行` 是另一个非常严重的错误, 经常看着代码莫名出现大段空白, 不明所以, 是高能预警吗 ? 让我们看一个真实的例子:

```lua
function ActivityLayer:setBrother(  )

    self._title_content:setString(tr(brother_static[1]["name"]))


    GameUtils.setSpriteFrameByName(self._icon,"leijichongzhi.jpg")

    self._desc:setString(tr(brother_static[1]["desc"])) 




    local scene = CellActivityBulletin.new(tr(brother_static[2]["desc"]))
    self._scroll_view_content:push(scene)

    self._scroll_view_content.swallow = true    
    self._scroll_view_content:layout(false)     
end
```

这段代码别的问题我们暂且不提, 就空行而言, 做的特别差. 参数内的两个空格, 第`6`行函数调用的参数间隔, 尤其是中间 `9-12` 的空行是什么意思吗? 我们来简单修改下.

```lua
function ActivityLayer:setBrother()
    self._title_content:setString(tr(brother_static[1]["name"]))
    self._desc:setString(tr(brother_static[1]["desc"])) 
    GameUtils.setSpriteFrameByName(self._icon, "leijichongzhi.jpg")

    local scene = CellActivityBulletin.new(tr(brother_static[2]["desc"]))
    self._scroll_view_content:push(scene)
    self._scroll_view_content.swallow = true    
    self._scroll_view_content:layout(false)     
end
```

是不是感觉好了一些, 代码行数减少了近一倍.

因为语法没有明确规定空白的用法, 而且有的用法也可以有多种选择, 所以团队应该有明确的规定.