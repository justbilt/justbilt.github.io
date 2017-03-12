title: cocos2d-x AssetsManager 问题汇总
date: 2015-06-06 00:42:33
tags:
- Quick-Cocos2d-x
- hotupdate
---

大家做热更新的时候都会用到 AssetsManager , 我们使用的 2.2.6 版本还存在一些问题, 在这里记录一下, 希望大家能够避开这些!

<!--more-->


# 1. "can not open destination file xxx"

AssetsManager 在解压时有一定概率会出现这个错误, 让我们先定位错误发生点:

```c++
FILE *out = fopen(fullPath.c_str(), "wb");
if (! out)
{
    CCLOG("can not open destination file %s", fullPath.c_str());
    unzCloseCurrentFile(zipfile);
    unzClose(zipfile);
    return false;
}
```

嗯, 其实就是打开一个文件去写的时候发生了错误, 这个 `fullPath` 就是解压的目标路径, 但是这个目标路径的文件夹是有可能不存在的, 比如 `download/res/ui/test.png`, 如果 ``download/res/ui` 目录不存在, 就会解压失败.

这个问题也有别人遇到了, 还有具体的原因分析, 大家可以看这里: [AssetsManager在下载某些特定内容的zip文件后解压缩会失败][1].

解决方案也比较简单, 已经在 3.x 有实现了, 就是在解压文件前, **遍历创建下每个层级的目录** 就可以了, 代码如下:

```c++
//There are not directory entry in some case.
//So we need to test whether the file directory exists when uncompressing file entry
//, if does not exist then create directory
const string fileNameStr(fileName);

size_t startIndex=0;

size_t index=fileNameStr.find("/",startIndex);

while(index != std::string::npos)
{
    const string dir=_storagePath+fileNameStr.substr(0,index);
    FILE *out = fopen(dir.c_str(), "r");

    if(!out){
        if (!createDirectory(dir.c_str())){
            CCLOG("can not create directory %s", dir.c_str());
            unzClose(zipfile);
            return false;
        }
        else{
            CCLOG("create directory %s",dir.c_str());
        }
    }
    else{
        fclose(out);
    }

    startIndex=index+1;
    index=fileNameStr.find("/",startIndex);
}
```

# 2. 下载进度问题

AssetsManager 的下载进度会在控制台上打印出来, 但是这个进度却与我们注册监听的进度不太一致, 经常都下载完成了, 进度却只走了 10% 左右, 而解压确实是没有占用进度的, 令人十分困惑.

经过仔细阅读源码和分析, 发现了问题所在, AssetsManager 实际上是多线程的, 使用消息队列在线程间通信. 下载线程有消息了, 会压入队列中, 主线程注册了update, 不断的从队列中拿取消息, 处理, 删除. 这套设定其实还是蛮不错的, 但是却**没有考虑到主线程的处理能力不足**的情况. 下面是下载进度的逻辑:

```c++
int assetsManagerProgressFunc(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
    AssetsManager* manager = (AssetsManager*)ptr;
    AssetsManager::Message *msg = new AssetsManager::Message();
    msg->what = ASSETSMANAGER_MESSAGE_PROGRESS;
    
    ProgressMessage *progressData = new ProgressMessage();
    progressData->percent = (int)(nowDownloaded/totalToDownload*100);
    progressData->manager = manager;
    msg->obj = progressData;
    
    manager->_schedule->sendMessage(msg);
    
    CCLOG("downloading... %d%%", (int)(nowDownloaded/totalToDownload*100));
    
    return 0;
}
```

举个例子, 下载线程每秒压入5个消息, 但是主线程的update回调是1秒一次, 得等5s才能处理完成. 这样就会导致下载早已完成, 但主线程却还有一大坨的消息没有处理完成, 仍旧在不紧不慢的处理着. 解决问题的方法也比较简单, 可以从两个角度入手, 提升主线程的处理能力和减少下载线程的消息制造. 由于主线程已经是注册的帧事件 update 了, 没有提升的空间, 所以只能从第二个角度入手了.

分析了下需求, 对于下载进度的索取, 其实没有必要过于精确, 精确到1%就可以了. 这样的话一共只会产生100个事件, 大大减少了事件的数量. 这个问题, 3.x 也做了处理, 和我的想法一致. 摘录实现如下:

```c++
int assetsManagerProgressFunc(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
    static int percent = 0;
    int tmp = (int)(nowDownloaded / totalToDownload * 100);
    
    if (percent != tmp)
    {
        percent = tmp;
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
            auto manager = static_cast<AssetsManager*>(ptr);
            if (manager->_delegate)
                manager->_delegate->onProgress(percent);
        });
        
        CCLOG("downloading... %d%%", percent);
    }
    
    return 0;
}
```

在查看源码的时候, 也发现了一些非常有意思的事情. AssetsManager 有做一个这样的设定, 下载全部完成后, 会将 `downloaded-version-code` 字段写入到 `UserDefault` 中, 解压完成后删除这个字段, 为什么呢? 

为了应对解压过程中出现了一些意外, 比如关闭了游戏进程, 这样重启游戏的时候就不用再下载更新包了. 这本是一个不错的设定, 但是它使用了事件队列来做这个事情, 上面说过, 下载完成后, 其实积压了一大坨的事件, 所以这个事件根本不会立刻被执行到. 但是解压的操作却不会受次影响, 会立刻执行到. 这会导致什么问题呢? **导致先删除`downloaded-version-code`字段, 后设置, 与预期的执行顺序完全想反**. 虽然最后可能不会影响到什么, 但是却是一段非常危险的代码.


---
## Update: 2015年06月25日

还是接着上面的那个问题, AssetsManager 将 `downloaded-version-code` 记录到了 `UserDefault` 中, 但是只有**解压成功**了才会删除, 那么解压失败了呢? 解压失败失败的原因有好多, 如果是下载的更新包有问题的话, 重启后仍然不会重新下载, 直接开始解压, 就会陷入到一个循环中, 一直出错. 解决的办法是什么呢? 

> 舍弃掉这个优化.

将 AssetsManager 中的这几行注释掉, 就不会记录 `downloaded-version-code`, 出错后重启就会重新下载:

```
CCUserDefault::sharedUserDefault()->setStringForKey(KEY_OF_DOWNLOADED_VERSION,
                                                    ((AssetsManager*)msg->obj)->_version.c_str());
CCUserDefault::sharedUserDefault()->flush();
```

我们的游戏就遇到了这样的情况, 玩家热更解压失败后就一直处于解压失败的状态, 只能删除游戏重新安装了!

哈哈,正如你所想的,3.x 也做了这样的处理:

```
if (! uncompress())
{
    Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, this]{
        UserDefault::getInstance()->setStringForKey(this->keyOfDownloadedVersion().c_str(),"");
        UserDefault::getInstance()->flush();
        if (this->_delegate)
            this->_delegate->onError(ErrorCode::UNCOMPRESS);
    });
    break;
}
```

解压失败后就直接清除了 `downloaded-version-code` 的记录.

---

从上述可以看到, 这些问题都已在 3.x 中解决, 所以能升级引擎的还是赶紧升级. 同时也会明白cocos的坑还是蛮多的, 大家一定要做好测试呀!

--EOF--

[1]: http://bbs.firedragonpzy.com.cn/forum.php?mod=viewthread&tid=119

