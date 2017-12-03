title: 更新 spine 的 cocos2d-x runtimes (二)
date: 2017-11-19 16:10:54
tags: [cocos2d-x, spine]
---

前段时间一个朋友向我咨询播放 spine 动画时遇到的问题:

![][1]

这个问题我们在很久之前也遇到过, 当时是另个一小伙伴负责的. 当时猜测可能是 `spine runtimes` 需要升级, 但是升级时遇到了技术问题, 恰好项目比较紧, 便让美术同学重新用老版的 Spine 编辑器重新做了一遍.

<!--more-->

无独有偶, 上周另一个小伙伴在播放 spine 动画时遇到了 `崩溃` 的问题, 而放到 creator 中测试是没有问题的, 看来又是运行库太过陈旧的原因.

趁着周末有点时间, 我也来尝试更新下 `spine runtimes`. 15 年的时候就更新过一次, 当时还写了一篇很水的文章 [<<更新 spine 的 cocos2d-x runtimes>>][2], 时隔两年, 这次是否也会依然顺利呢 ?


![][3]

我这次并没有选择从 spine 的[官方仓库][4] 更新 runtimes, 既然 creator 中是没有问题的, 为什么不选择直接从 creator 附带的 [cocos2d-lite][5] 中复制过来呢 ?

## 1. 替换最新的 spine runtimes

简单浏览了下, 目录结构没有发生变化, 看来是一个好的开端. 我们先从用 cocos2d-lite 中的 `cocos/editor-support/spine` 替换掉 quick-cocos2d-x 中的同名目录.

用 XCode 打开 `quick/player/proj.mac/player3.xcodeproj` 模拟器工程, 我们用 quick-cocos2d-x 的 Mac 版来完成后续的工作. 在 `Project Navigator` 找到 `spine` 文件夹, 删除掉红色的缺失文件, 右键 `Add files to 'cocos2d_libs.xcodepro'` 添加上新增的文件, 编译一下.


## 2. 修复 lua binding 错误

发现有 `lua_cocos2dx_spine_auto.cpp` 有报错, 错误内容:

```cpp
...lua_cocos2dx_spine_auto.cpp:701:32: No viable conversion from 'function<void (int)>' to 'const function<void (spTrackEntry *)>'
...lua_cocos2dx_spine_auto.cpp:758:43: No viable conversion from 'function<void (int, spEvent *)>' to 'const function<void (spTrackEntry *, spEvent *)>'
...lua_cocos2dx_spine_auto.cpp:815:46: No viable conversion from 'function<void (int, int)>' to 'const function<void (spTrackEntry *)>'
...lua_cocos2dx_spine_auto.cpp:872:43: No viable conversion from 'function<void (int)>' to 'const function<void (spTrackEntry *)>'
...lua_cocos2dx_spine_auto.cpp:925:35: No viable conversion from 'function<void (int, int)>' to 'const function<void (spTrackEntry *)>'
...lua_cocos2dx_spine_auto.cpp:982:41: No viable conversion from 'function<void (int)>' to 'const function<void (spTrackEntry *)>'
...lua_cocos2dx_spine_auto.cpp:1035:32: No viable conversion from 'function<void (int, spEvent *)>' to 'const function<void (spTrackEntry *, spEvent *)>'
...lua_cocos2dx_spine_auto.cpp:1143:30: No viable conversion from 'function<void (int)>' to 'const function<void (spTrackEntry *)>'
```

看到这个文件名就应该联想到没有重新生成 lua 绑定的代码. 打开终端进入 `tools/tolua`, 运行:

```sh
python genbindings.py
```

如果之前没有运行过这个脚本的话, 可以[参照这份官方文档][6]需要搭建环境. 等待运行结束后, 再次编译.


## 3. 修复 API 变动导致的编译错误

### 1. *Listener 回调函数参数变化

这次改为 `lua_cocos2dx_spine_manual.cpp` 出错了:

```cpp
...lua_cocos2dx_spine_manual.cpp:233:48: No viable conversion from '(lambda at ...lua_cocos2dx_spine_manual.cpp:233:48)' to 'const StartListener' (aka 'const function<void (spTrackEntry *)>')
...lua_cocos2dx_spine_manual.cpp:241:46: No viable conversion from '(lambda at ...lua_cocos2dx_spine_manual.cpp:241:46)' to 'const EndListener' (aka 'const function<void (spTrackEntry *)>')
...lua_cocos2dx_spine_manual.cpp:249:51: No viable conversion from '(lambda at ...lua_cocos2dx_spine_manual.cpp:249:51)' to 'const CompleteListener' (aka 'const function<void (spTrackEntry *)>')
...lua_cocos2dx_spine_manual.cpp:257:48: No viable conversion from '(lambda at ...lua_cocos2dx_spine_manual.cpp:257:48)' to 'const EventListener' (aka 'const function<void (spTrackEntry *, spEvent *)>')
```

这个文件中放的是手动导出 lua 接口的代码, 我们需要推测一下这些错误发生的原因. 让我们看下出错的具体代码:

```cpp
self->setStartListener([=](int trackIndex){
    executeSpineEvent(self, handler, eventType, trackIndex);
});
```

`StartListener` 是声明在 `SkeletonAnimation.h` 中的, 相关代码的 git diff 如下:

```cpp
-typedef std::function<void(int trackIndex)> StartListener;
-typedef std::function<void(int trackIndex)> EndListener;
-typedef std::function<void(int trackIndex, int loopCount)> CompleteListener;
-typedef std::function<void(int trackIndex, spEvent* event)> EventListener;
+typedef std::function<void(spTrackEntry* entry)> StartListener;
+typedef std::function<void(spTrackEntry* entry)> InterruptListener;
+typedef std::function<void(spTrackEntry* entry)> EndListener;
+typedef std::function<void(spTrackEntry* entry)> DisposeListener;
+typedef std::function<void(spTrackEntry* entry)> CompleteListener;
+typedef std::function<void(spTrackEntry* entry, spEvent* event)> EventListener;
```

显而易见, 函数的参数发生了变化, 我们修改下 lambda 函数参数即可, 同时还需要考虑 `executeSpineEvent` 函数的变化. 我的修改如下:

```cpp
int executeSpineEvent(LuaSkeletonAnimation* skeletonAnimation, int handler, spEventType eventType,  spTrackEntry * entry, int loopCount = 0, spEvent* event = nullptr )
```

改变了这个函数的参数, 调用的地方也要改变, 这个就不贴代码了.

### spEventType 变化

还需要注意到的一个变化是:
```cpp
typedef enum {
    SP_ANIMATION_START, SP_ANIMATION_INTERRUPT, SP_ANIMATION_END, SP_ANIMATION_COMPLETE, SP_ANIMATION_DISPOSE, SP_ANIMATION_EVENT
} spEventType;
```

想必之前多了 `SP_ANIMATION_INTERRUPT` 和 `SP_ANIMATION_DISPOSE` 事件, 我们要更新所有用到 `spEventType` 的地方.

SpineConstants.lua :

```lua
sp.EventType =
{
    ANIMATION_START = 0, 
    ANIMATION_INTERRUPT = 1, 
    ANIMATION_END = 2, 
    ANIMATION_COMPLETE = 3, 
    ANIMATION_DISPOSE = 4, 
    ANIMATION_EVENT = 5,
}
```

以及 `LuaScriptHandlerMgr.h` 和 `lua_cocos2dx_spine_manual.cpp` 中共计五处变化.

### TrianglesCommand 参数错误

解决了这个问题, 编译后发现还有一个错误:

```cpp
SkeletonBatch.cpp:95:37: No matching member function for call to 'init'
```

是调用 `TrianglesCommand:init` 时多传入了一个参数, 可能是新版 cocos 的 api 改动吧, 我们直接删掉最后一个参数:

```cpp
-        _command->trianglesCommand->init(globalZOrder, textureID, glProgramState, blendFunc, *_command->triangles, transform, transformFlags);
+        _command->trianglesCommand->init(globalZOrder, textureID, glProgramState, blendFunc, *_command->triangles, transform);
```

---

到此搞定了所有的编译问题, 尝试了下 Android 和 iOS 的编译, 也没有什么问题. 让我们运行下看下效果:

![][8]

后记:

# 1. Spine Error:Unknown attachment type: skinnedmesh

跟新完后有的 Spine 文件会报这个错误, [论坛里有解决方案][9], 虽然不报错了, 但是也不动了, 可能需要美术重新做一遍.

# 2. 真的有必要从 cocos2d-x-lite 中更新吗?

经过好友 @郭彬 的提醒, 确实没有必要, 他就是从官方仓库更新的, 是我画蛇添足了.

[1]: https://ws3.sinaimg.cn/large/006tNc79ly1flngumf7luj30ht04mq41.jpg
[2]: /2015/08/16/upgrad-spine-runtime/
[3]: https://ws1.sinaimg.cn/large/006tNc79ly1flnh9m7zekj307w04x746.jpg
[4]: https://github.com/EsotericSoftware/spine-runtimes
[5]: https://github.com/cocos-creator/cocos2d-x-lite/subscription
[6]: https://github.com/dualface/v3quick/blob/v3/tools/tolua/README.mdown
[7]: https://ws4.sinaimg.cn/large/006tNc79ly1flnqfyzvmzj30x90u7n7p.jpg
[8]: https://ws2.sinaimg.cn/large/006tNc79ly1flnr7bq9vtg306v08skah.gif
[9]: http://forum.cocos.com/t/1-3-1-spine-c-spine-skinnedmesh/41180/2