title: "quickx-3.3 热更新若干心得"
date: 2016-01-01 08:43:50
tags: [quick-cocos2d-x, 热更新]
---

关于热更新, 虽然这已经是第三次实现了, 但每一次都会有新的收获, 都会比上次更加完善一下.

<!-- more -->

# 一. App 版本号的获取

对于我们的游戏来说,其实是有两个版本配置的,一个是 `versionName`, 一个是 `versionCode`. versionName 只是一个显示, 类似于 `1.2.3`, 而 versionCode, 它是一个纯数字的字符串 (e.g. 4083), 它可以简单的**转化为 int 去对比版本大小**. 那么如何去获取app对应的版本呢 ?

iOS:
```cpp
std::string Device::getAppVersionCode()
{
    return [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"] cStringUsingEncoding:NSUTF8StringEncoding];
}
```

MacOS 上获取版本号的方式与 iOS 一致.

Android:
```java
    public static String getAppVersionCode() {
        String versionCode = "";
        PackageInfo pInfo;
        try {
            pInfo = sActivity.getPackageManager().getPackageInfo(sActivity.getPackageName(), 0);
            versionCode = ""+pInfo.versionCode;
        } catch (NameNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return versionCode;
    }
```

为了调用这个方法, 我们还需要额外实现一个 jni 的调用.

# 二. 如何修改 Mac 上 UserDefault 记录

quickx-3.3 Mac 版不再像 2.x 时将 UserDefault 保存在项目的根目录, 而是使用 `NSUserDefaults` 将配置保存在了系统的配置目录:

```
~/Library/Preferences/com.cocos.quick.apps.player.plist
```

双击可以使用 XCode 打开, **修改保存完毕后并不会立刻生效**, 而是需要再额外运行一条命令:

```
killall -SIGTERM cfprefsd
```

# 三. 如何重启游戏

这个做法有好多, 纯App层面的, 纯Lua层面的, 我们使用了一个虽然很土但却很简单的做法. 因为热更新只会更新 lua 代码和资源, 因此可以通过重启 LuaEngine, 清空三大缓存来实现重启. 在 Lua 中发送重启消息, C++ 接收到消息后销毁并重新创建 LuaEngine. 关键代码如下:

Lua:

```lua
cc.Director:getInstance():getEventDispatcher():dispatchEvent(cc.EventCustom:new("NEED_RESTART_APP"))
```

AppDelegate.cpp:

```cpp
// 监听重启事件
auto callBack = [this](EventCustom* event)
{
    //do restart LuaEngine
};
Director::getInstance()->getEventDispatcher()->addCustomEventListener("NEED_RESTART_APP", callBack);
```

# 四. 代码更新策略

大体分为三类:

1. 全量更新
2. 相对于上一个版本增量更新
3. 相对于渠道版本增量更新

**全量更新**指每一个热更新包内都含有项目全部代码, 如果项目体量较小的话勉强可以接收这种方案, 如果项目略大一些, 更新包就会很大, 随便更改一行代码都会有 3,4M 的更新, 实乃下下之策.

**相对于上一个版本增量更新**, 这样更新包就会小很多, 达到资源的最大利用, 缺点是如果版本相差较大就会需要下载多个更新包, 如果代码采用 zip 压缩的话, 需要将这些 zip (e.g. `1.1-1.2.zip`, `1.2-1.3.zip`)都加载进来.

**相对于渠道版本增量更新**, 这一种折中的解决方案, 对比差异是永远是基于线上渠道版本, 这种做法的优点在于游戏只用额外挂载一个 `patch.zip`, 玩家也只用更新一次即可到最新版, 而代价只是会增加一些服务器更新包的存储开销.

很长一段时间内, 我们都很 low 的采用了第一种解决方案, 造成这种错误的原因在于我错误的估计了增量更新的难度, 进而放宽了自我要求, 觉得全量更新还OK balabala... 多亏了 @BinStartup 童鞋强烈地需求, 才迫使我们去考虑更优解决方案, 用我们内部的一个梗来夸赞一下 Bin 童鞋: '牛逼呀~' . 

# 五. 更新包的制作

相比于代码层面的热更新支持, 自动化的制作热更新包才是更为重要的事情, 因为这个操作会持续的发生, 并且随着更新次数的增多复杂度会剧增.

从大的层面来看, 更新包的制作主要有这么几个问题:

## 1. 如何标记并导出版本?

首先我们要考虑如何标记版本, 因为要做增量更新就必须得对比不同的版本, 实现的方法有好多, 你甚至可以每次都将项目拷贝一份, 用版本号来命名. 我们项目版本控制工具是 git , 因此可以使用 git tag 来标记版本号.

至于导出版本, 一开始并没有考虑到这个, 本来打算直接使用 `git diff`来进行版本对比, 不幸的是我们项目中有一堆的 `submodule`, git diff 并不能对比子模块的变化. 同理 `git archive` 也不可以, 最终我们使用了:

```bash
git checkout $tag
git submodule update --init --recursive
```
这样当前项目的环境就到了 `$tag` 这次提交, 然后将 `src` 目录拷贝到一个临时目录, 再切换到下一个 tag . 多次之后, 就获取到了所有版本的脚本文件了.

## 2. 如何对比生成 diff 文件

因为是拷贝到了临时的目录, 所以不能通过 `git diff` 来进行对比了. 简单搜索了下, 可选的是系统自带的 `diff` 命令, 不过它的结果不方便处理. 最终决定使用 python 的 `filecmp` 模块进行对比. 

## 3. 其他后续处理
diff 出来的脚本问题, 我们需要加密, 和加密 src 一样调用 `compile_scripts.sh` 即可.

因为热更新需要的是一个 zip 文件, 我们可能会用到 `zip` 命令, 需要注意的一点是不能简单的使用 `zip -r xxx.zip yyy`, 这样的话压缩包内会含有 `yyy` 目录. 正确的做法是:

```
(cd yyy; zip -r ../xxx.zip *)
```

---

重启的那个方案还有一些隐患, 所以没有放出具体的代码, 等修复之后会补上. 
暂时就这些, 后面想到啥了再加上来.


