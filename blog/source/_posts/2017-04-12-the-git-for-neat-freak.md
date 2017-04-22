title: 洁癖患者的 Git GUI 指南
date: 2017-04-12 23:34:12
tags: [git, neat-freak]
---

前段时间我看到了一篇文章, [洁癖者用 Git：pull --rebase 和 merge --no-ff][1] , 发现和我们目前的工作流程很像, 区别在于我们是使用 GUI 工具来做的这些, 也做了一些新的尝试.

注: 阅读下面的文章前, 我默认你已经读过上面推荐的那篇文章, 本篇文章不会赘述其中的内容.

# 一. pull --rebase 和 merge --no-ff

使用软件: [SourceTree][2]

## 准备工作

SourceTree 需要更改这些偏好设置: 

> Allow force push

位于 General 标签, 如果有追踪的远端, 本地 rebase 后会造成本地提交历史和历史不一致, 必须强制推 (Force Push) 才能推送到远端. 勾选这个选项后, 会在 push 界面上多出一个 Force push 选项.

![][5]

> Use rebase instead of merge by default for tracked branches

这个就是那篇文章中的 `git pull --rebase` , 勾选后 pull 界面上的 Rebase instead of merge 选项会默认处于勾选状态 .

> Do not fast-forward when merging, always create commit

这个就是 `merge --no-ff` , 合并分支后, 总是会产生一次 Merge Commit .

> Display author date instead of commit data in log (可选)

一旦 rebase 后, 提交时间就会变成 rebase 的时间, 影响判断, 如果勾选这个选项, 就会一直显示 commit 创建时的日期, 这条不是必须的, 大家根据自己需要选择.

## pull --rebase

> git pull --rebase

![][6]

如果打算 pull 一个 **没有跟踪中** 的远端分支, 即使你修改了偏好设置, `Rebase instead of merge` 选项也不会默认勾选, 需要手动勾选.

> git rebase <branch>

![][7]

假如想让 A 分支上的该动都基于 B 分支, 先保证你处于 A 分支, 在 BRANCHES 中 B 上右键选择 `Rebase current changes onto B` .

> rebase 冲突后的处理

rebase 过程实际上可以理解为是将当前分支上的所有 commit 再一次次应用的过程, 那么如果其中一步冲突了怎么办?

rebase 会暂停下来, 等待你解决完冲突后的操作. 如果在终端中的话, 你有四个选项可选:

```sh
git rebase --continue //继续
git rebase --skip //跳过
git rebase --abort //终止
git rebase --quit //结束
```

在 SourceTree 中, 我们解决完冲突后, 可以点击 `Commit` 按钮, 这时会出现下面这个界面:

![][14]

`Abort Rebase` , `Continue Rebase` 分别与 `--continue` 和 `--abort` 对应. 如果解决完冲突后这次 Commit 不包含任何 Changes 的话, 我们就需要 `--skip` 了, 很不幸的是 SourceTree 并没有提供这个选项, 只能在终端中操作.

## merge --no-ff

![][8]

和 rebase 操作基本一致, 菜单选择 `Merge B into A`. 下面会弹出这个界面:

![][9]

> Commit merged changes immediately

勾选或如果合并没有冲突的话, 会自动提交这次 Merege Commit, 否则的话, 你需要手动提交.

> Include messages form commits being merged in merge commit

会把这个分支上的所有的提交日志附加到 Merege Commit 的提交日志中.


# 二. rebase befroe merge

就是在 merge 前先 rebase, 假如我们想把 B 合并进 A 中, 应当先在 B 上面 rebase A, 再切换到 A 上面 merge B. 这样可以避免出现下面这样的历史:

![][10]

这样做, 还会附带一个好处, 就是会使得这次 `Merege Commit` 非常的纯粹, 因为在 rebase 时已经把潜在的冲突都解决完了, 所以这次 merge 一定不会有冲突, 这样 Merege Commit 的改动就会和分支 B 的改动完全一致, 不会再有其他的改动(解决冲突时的修改).



# 三. 合并前整理分支

使用软件: [GitUp][3]

Gitup 功能十分神奇, 而且还提供了很强大的 undo/redo 操作, 所以可以很不负责任的说, 大家可以随意尝试其中的各种功能, 不必担心把仓库搞挂.

如果坚持使用 rebase 话, 就会明白 rebase 的复杂度, 是与这个分支上的 commit 次数有一定关系的. 所以我们可以在 rebase 前整理这个分支:

> 合并相同功能的 Commit 

![][11]

使用 gitup 打开仓库, 随便在一次 commit 上右键, 便可以看到上图的菜单. 这里面有所有的操作, 下面我来说一下常用的:

> Fixup with Parent / Squash with Parent

作用都是将本次提交和上一次合并, 区别是后者可以同时修改提交日志.

> Swap with Parent / Child

就是讲两次提交互换位置, 比如你有 A-B-C 三次提交, 你想把 A 和 C 合并, 但是中间隔着一个 B, 这时就是把 B 和 C 的位置交换后 Squash.

> Split

将一次提交拆分为两次, 不过拆分粒度只能到文件级. 也很常用, 比如误将一些不属于这次提交做的改动提交了上去, 可以用这个拆分出来.

# 四. 后记


![][12]

这个是我们坚持了一段时间之后的提交历史对比, 左侧杂乱无章, 犬牙呲互, 这是一条无法直视的提交历史, 右侧的则清晰不少. 

![][13]

我们可以修改筛选器中的 `All Parents` 为 `First parent only`, 这样我们住分支上的历史便只剩下了 Merge Commit ,那么只看这些 Commit 便可以知道整个分支的进化历程.


[1]: http://hungyuhei.github.io/2012/08/07/better-git-commit-graph-using-pull---rebase-and-merge---no-ff.html
[2]: https://www.sourcetreeapp.com/
[3]: http://gitup.co/
[4]: https://ww2.sinaimg.cn/large/006tKfTcgy1felu3vt3hnj30do0b6ac0.jpg
[5]: https://ww1.sinaimg.cn/large/006tKfTcgy1feoct36xyhj30lx0k0whu.jpg
[6]: https://ww1.sinaimg.cn/large/006tKfTcly1feohl206naj30mk0fajup.jpg
[7]: https://ww3.sinaimg.cn/large/006tKfTcly1feoifjhienj30jb0bz0xi.jpg
[8]: https://ww2.sinaimg.cn/large/006tKfTcly1feoipaaqqoj30jb0c2tbm.jpg
[9]: https://ww4.sinaimg.cn/large/006tKfTcly1feois0fn45j30il08jwfl.jpg
[10]: https://ww2.sinaimg.cn/large/006tKfTcly1feojg14lnjj301003q3yh.jpg
[11]: https://ww2.sinaimg.cn/large/006tKfTcly1feok1ndseyj30pe0iemzi.jpg
[12]: https://ww3.sinaimg.cn/large/006tKfTcly1feolo8so66j305k0m8gmk.jpg
[13]: https://ww4.sinaimg.cn/large/006tKfTcly1feoluos10ej30n604hab9.jpg
[14]: https://ww1.sinaimg.cn/large/006tKfTcly1feoohf4h49j30hs07u0u0.jpg