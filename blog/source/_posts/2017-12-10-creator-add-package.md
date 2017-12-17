title: 如何为 Creator 添加 js/ts 第三方库支持
date: 2017-12-10 19:57:39
tags: [CocosCreaor]
---

正如知乎[我的这个回答][1]中所说, 我们开始使用 Cocos Creaor 开发新的项目, 所以以后这个博客会陆续开始更新一些 Creator 相关的文章.

我们在项目开发的过程中, 肯定会遇到一些项目间通用的需求, 比如 `压缩/加密` 之类的, 我们当然可以自己实现一个, 但更好的做法是找找网上有没有别人实现好的库直接拿来用, 今天我们就来看看如何为 Creator 添加添加 js/ts 第三方库支持.

<!--more-->

# 一. 下载源文件

以 [javascript-state-machine][2] 为例, 我们可以在 `Clone or Download` 中选择 `Download Zip` 下载源文件压缩包, 解压后在将 `dist/state-machine.js` 复制到我们项目中的 `assets/Script` 目录中.

然后在我们的代码中:

```javascript
import StateMachine from "state-machine";
cc.Class({
    extends: cc.Component,
    properties: {
        label: {
            default: null,
            type: cc.Label
        },
        text: 'Hello, World!'
    },
    onLoad: function () {
        this.label.string = this.text;
        var fsm = new StateMachine({
            init: 'solid',
            transitions: [
              { name: 'melt',     from: 'solid',  to: 'liquid' },
              { name: 'freeze',   from: 'liquid', to: 'solid'  },
              { name: 'vaporize', from: 'liquid', to: 'gas'    },
              { name: 'condense', from: 'gas',    to: 'liquid' }
            ],
            methods: {
              onMelt:     function() { console.log('I melted')    },
              onFreeze:   function() { console.log('I froze')     },
              onVaporize: function() { console.log('I vaporized') },
              onCondense: function() { console.log('I condensed') }
            }
        });
        fsm.melt();       
    }
});
```

在控制台中可以看到输出:

```sh
I melted
```

# 二. 使用 npm

搞 js 的同学都会知道 [npm][3] 这个 js 最大的轮子集中营, 它可以很方便的帮你安装/管理所需的第三方依赖库. 如果你觉得上面下载代码的形式太过繁琐的话, 就可以考虑使用 npm 来安装. 

npm 会在你安装 [nodejs][4] 时自动拥有, 可以使用 `npm -v` 命令检查 npm 状态. 下面我们要观察下你的项目有没有 `package.json` 这个文件, 如果没有的话我们要先使用:

```sh
npm init
```

初始化项目, 需要填一堆信息, 基本上一路回车就可以. 完成之后我们可以去 npm 的网站上搜索下是否存在我们需要的库, 然后用 `npm instal xxx --save` 命令安装.

我们还是以 `javascript-state-machine` 距离, 我们可以找到它在 npm 上的[这个页面][5], 然后安装:

```
npm install javascript-state-machine --save
```

安装完成后, 我们有了一个新的问题, 如何去 require 这个文件呢 ? 之前源码直接 require 的方式肯定是不行的了, 最终在论坛中找到了[解决方案][6], 我们需要这样:


```javascript
import StateMachine from "javascript-state-machine";
```

# 三. 在 TypeScript 中使用 npm 安装的三方库

上面讲的都是 js 的项目, 那么 ts 的项目又该如何搞呢 ? 我们除了 js 中的那些操作外还需要安装一个 ts 的标注库. 

我们先去 [TypeSearch][7] 搜索下是否有我们需要的库的标注库:

![][8]

点击后会跳转的 npm 对应的页面, 我们要根据提示安装这个库:

```sh
npm install --save @types/javascript-state-machine
```

这样我们就可以在 TypeScript 中使用下面代码去 import :

```typescript
import * as StateMachine from "javascript-state-machine";
```


值得一提的是, 这些 @types 的库很多都是大家自己制作后贡献给社区的, 集合在 [DefinitelyTyped][9] 这个仓库中, 省去了我么自己标注的工作, 感谢这些贡献者们. 

还有一个问题, 这些标注库更新可能落后于原始项目, 因此我们要安装对应版本的原始库才能够正确提示, 对应版本可以在 `node_modules/@types/javascript-state-machine/index.d.ts` 找到:

```sh
// Type definitions for Finite State Machine 2.4
```


因此, 我们要安装 `javascript-state-machine` 需要指定版本:

```sh
npm install javascript-state-machine@2.4 --save
```

---

以上就是我们安装第三方库的经验, 希望能够帮助到大家. 我们也是才开始使用 Creator, 如果遇到问题, 希望可以和大家多多交流.

[1]: https://www.zhihu.com/question/66819338/answer/259901965
[2]: https://github.com/jakesgordon/javascript-state-machine
[3]: https://www.npmjs.com/
[4]: https://nodejs.org/zh-cn/
[5]: https://www.npmjs.com/package/javascript-state-machine
[6]: http://forum.cocos.com/t/npm/37930/2?u=justbilt
[7]: https://microsoft.github.io/TypeSearch/
[8]: https://ws4.sinaimg.cn/large/006tNc79ly1fmc08dgaohj30gc07zq3j.jpg
[9]: https://github.com/DefinitelyTyped/DefinitelyTyped