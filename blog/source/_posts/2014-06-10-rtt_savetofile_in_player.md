title: 在Player中保存CCRenderTexture到文件
date: 2014-06-10 17:42:36
categories:
- quick-cocos2d-x
- CCRenderTexture
---

在 player 中调用 CCRenderTexture 的 saveToFile 函数会导致player崩溃,crash log 如下:

![][1]

<!--more-->

从日志中我们可以找到这句话:

> Assertion failed: (false), function saveToFile, file /Users/bilt/Documents/quick-cocos2d-x/lib/cocos2d-x/cocos2dx/platform/mac/CCImage.mm, line 921.

说的很清楚,CCImage.mm 的 921 行触发了断言失败,嗯,一定是我的用法不太对.

我们打开CCImage.mm看到下面这段代码:

```c++
bool CCImage::saveToFile(const char *pszFilePath, bool bIsToRGB)
{
	assert(false);
	return false;
}
```

喂,不带这样子的,分明是没有实现嘛.

好吧,我们试着把`ios/CCImage.mm`中的实现搬过来试试,这里我就不贴大段的代码了,拷过来编译时会出错(预料之中),我OC学的不好大家不要见怪,错误如下:

```smalltalk
CGImageRef iref                    = CGImageCreate(m_nWidth, m_nHeight,
                                                    bitsPerComponent, bitsPerPixel, bytesPerRow,
                                                    colorSpaceRef, bitmapInfo, provider,
                                                    NULL, false,
                                                    kCGRenderingIntentDefault);

UIImage* image                    = [[UIImage alloc] initWithCGImage:iref];

```
这里通过`CGImageRef` 创建一个 `UIImage`, 但是 Mac os 上是没有 `UIImage` 的,首先想到的是可以寻找一个接口替换掉它,短暂Google后放弃.

后来想着能否直接将`CGImageRef`保存为文件,还真让我找到了,见`stackoverflow`的这个([戳我][2])问题:
```smalltalk
void CGImageWriteToFile(CGImageRef image, NSString *path) {
    CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:path];
    CGImageDestinationRef destination = CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, NULL);
    CGImageDestinationAddImage(destination, image, nil);

    if (!CGImageDestinationFinalize(destination)) {
        NSLog(@"Failed to write image to %@", path);
    }

    CFRelease(destination);
}
```

大家可以看到关键的函数是`CGImageDestinationCreateWithURL` ,大概查了一下这个函数支持的格式还真不少,如下:

```smalltalk
extern const CFStringRef kUTTypeImage                                __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
extern const CFStringRef kUTTypeJPEG                                 __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
extern const CFStringRef kUTTypeJPEG2000                             __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
extern const CFStringRef kUTTypeTIFF                                 __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
extern const CFStringRef kUTTypePICT                                 __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
extern const CFStringRef kUTTypeGIF                                  __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
extern const CFStringRef kUTTypePNG                                  __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
extern const CFStringRef kUTTypeQuickTimeImage                       __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
extern const CFStringRef kUTTypeAppleICNS                            __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
extern const CFStringRef kUTTypeBMP                                  __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
extern const CFStringRef kUTTypeICO                                  __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
```

这下可爽了,保存`png`和`jpg`都可以,我对这个函数进行了微小的改造,将`kUTTypePNG`作为参数传入,一番修改之后`saveToFile`函数的关键代码变成了这个样子:
```smalltalk
CGDataProviderRef provider        = CGDataProviderCreateWithData(NULL, pixels, myDataLength, NULL);
CGColorSpaceRef colorSpaceRef    = CGColorSpaceCreateDeviceRGB();
CGImageRef iref                    = CGImageCreate(m_nWidth, m_nHeight,
                                                    bitsPerComponent, bitsPerPixel, bytesPerRow,
                                                    colorSpaceRef, bitmapInfo, provider,
                                                    NULL, false,
                                                    kCGRenderingIntentDefault);
if (saveToPNG)
{
    CGImageWriteToFile(iref,[NSString stringWithUTF8String:pszFilePath], kUTTypePNG);
}
else
{
    CGImageWriteToFile(iref,[NSString stringWithUTF8String:pszFilePath], kUTTypeJPEG);
}
```

重新编译运行player,运行`samples/cocos2dx_luatest`,找到`RenderTextureTest` 点击 `SaveImage` ,这下不会崩溃了,在例题的根目录可以找到保存的图片.

> 这个更改已经提交pr并且已经merge到了develop分支,详情可以戳[这里][4].


这里说下我的感悟:

1. Quickx 的 player 真的是个特别棒的东西,这里不得不佩服@廖大 ,绝对是提高效率的利器, 但是我们经常会看到player的一些行为和真机不太一样, 这时不能放弃player, player的本质就是cocos2d-x的Mac版, 根据这个去找寻解决方案会非常的方便.

2. 这个问题是发生在quick 2.2.2 中的, 因为quick的最新版是2.2.3, 所以我会先去那里找看有没有解决这个问题, 发现没有, 然后我会去cocos2d-x 的2.2.3找解决方案,因为本是一家东西, 发现还是没有没有, 接下来我发现cocos2d-x 3.x 版本解决了这个问题, 3.x完全用C++ 实现了保存png,jpg的功能, 在各个平台都管用, 这个非常赞, 但是迁移过来成本太高, 会有一堆依赖改变的地方, 所以不到迫不得已是不会自己实现的.

3. 这个问题好像遇到的人特别少,唯一在官网有一个人提问(看[这里][3]),但是没有回答,看版本是2.1.4,这应该算是历史遗留问题了.

(全文完)



[1]:http://ww2.sinaimg.cn/large/7f870d23gw1eh9yz0coh4j20rs0m8n4d.jpg
[2]:http://stackoverflow.com/questions/1320988/saving-cgimageref-to-a-png-file
[3]:http://discuss.cocos2d-x.org/t/screenshot-on-mac/8211
[4]:https://github.com/chukong/quick-cocos2d-x/pull/370/files




