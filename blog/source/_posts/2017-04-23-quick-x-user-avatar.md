title: Quick-cocos2d-x 自定义头像
date: 2017-04-23 21:54:41
tags: [Quick-Cocos2d-x]
---

前段时间我们团队实现了游戏中自定义头像模块, 这是一个比较有意思的功能, 加上网上这方面的资料也不多, 所以给大家分享下我们的做法. 

自定义头像这个功能牵扯到的内容还是比较多的, 一个人做的话需要技术储备比较大, 因此我们将这个任务做了如下的拆分:

1. 选择头像
2. 上传头像
3. 下载头像
4. 显示头像

每个人负责其中的一小部分内容, 再把这些小模块组合起来就完成了任务. 

<!--more-->

# 一. 选择头像

头像选择这部分的内容基本上和 Quick 没有关系, 主要内容有:

1. 拍照/相册
2. 剪裁
3. 获取图像数据

这部分逻辑 Android/iOS 都要单独实现, 我们自己从头写起恐怕不太现实, 毕竟我们不是专业做这个的, 不过不用担心, 已经有前辈们为我们造好了轮子. 

Android 上我们选择是的 [TakePhoto][1] 这个库, iOS 上我们选择的是 [VPImageCropper][2], 具体接入方式大家查看官方文档就可以. 这两个库我们做了很微小的修改, 以适应项目的实际需求, 同时因为 Lua 这边对二进制的数据处理比较麻烦, 我们选择将图片数据进行 base64 编码后返回给 Lua 端.

# 二. 上传头像

上传头像其实是比较简单的, 我们尝试过 tcp 和 http 这两种上传方式. 我们在尝试 tcp 上传的时候遇到了一个很棘手的问题, 就是发送数据过大会导致 tcp 连接断开. 

这个问题十分诡异, 同样长度的数据, 在 macOS 和 iOS 上没有任何问题, 但是在 Android 上就不行, scoket 在发送数据一段时间后收到了 RST 指令, 紧接着链接就会断开. 和服务器同学研究一段时间后无果, 就放弃了这个问题, 转为使用 http 上传.

使用了 http 之后, 还是上传失败, 服务器收到的 post 请求中没有 body, 而客户端加日志显示数据没有问题都塞到 body 中了. 使用抓包工具分析后确实长度有问题, 这就十分诡异了. 在偶然间发现微信也会遇到图片发布出去的情况后, 我们分析是 wifi 有问题, 换了另一个网络之后就没有问题了.

```lua
	local req = network.createHTTPRequest(reponse, url, "POST")
	
	req:setTimeout(timeout)
	req:addRequestHeader("Content-Type: mmf/bin")
	req:addRequestHeader("Accept: mmf/bin")
	req:setPOSTData(imagedata)

	req:start()
```

这是一段比较简单的 http 上传数据的代码示例, 这期间我还遇到了另一个诡异的问题, 就是客户端这里明明发送的是 POST 请求, 但是服务器收到的却是 GET, body 中没有数据, 同样的代码我们在 Mac 上使用模拟器是没有问题的, 非常诡异呀. http 在 Android 上的实现位于 QuickHTTPInterface.java 中, 经过在 Android 断点调试, 发现并没有调用 setRequestMethod . 

原来这种 http 请求我们是封装了一层的, 这个 `POST` 和 `GET` 是通过参数传入的, 外部传入的其实是一个枚举. 然而在我之前的重构中, 修改了这个枚举的名字, 导致传入的值是 nil, 这就导致没有调用 createHTTPRequest 时没有传入 methed 参数, 默认就是 GET 类型的, 因为平台的 http 实现不同, 导致有些平台正常, 有些平台会出错.

上传头像的内容就是这些.


# 三. 下载头像

对于这个问题, 我们开始还打算在 c++ 端用 curl 实现, 然后到处到 Lua 这边使用, 直到发现了 HttpClient 的 `saveResponseData` 接口. 调用了这个接口可以直接把 Response 中的数据保存在本地, 这样下载单个文件就搞定了. 下载代码如下:

```lua
-- http下载
--[[
	@param string _url _url地址
	@param string _savepath 指定的存储路径
	@param function _callfunc 下载完成回调方法
]]
local function _download(_url, _savepath, _callfunc)
	local req = network.createHTTPRequest(
		function(event)
            if event.name == "completed" then

                --判断是否下载成功
                local success = false
                local code = event.request:getResponseStatusCode()
                if code == 200 and event.request:saveResponseData(_savepath) then
                	success = true
                end
                if _callfunc then
                	_callfunc(success, {url = _url, req = event.request, path = _savepath})
                end
            end
        end, _url, "GET")
    req:setTimeout(60)
    req:start()
end
```

但是整个下载功能呢并没有这么简单, 我们应该考虑到 `本地缓存`, `去重`, `队列限制`等情况, 这样我们可以实现一个通用的下载模块, 不仅仅是为自定义头像功能, 其他的模块也可以使用.


# 四. 显示头像

这个功能其实很简单了, 为了什么要单独拿出来说呢? 就是我们要实现的很优雅. 自定义头像这个功能可能不是一开始就提出的, 因为改了 Native 层的代码, 所以需要线上换包, 这就比较麻烦了, 所以我们这个模块要和之前的头像保持兼容. 

我们游戏中的显示头像的地方有很多, 我们不能把下载的调用写的到处都是, 比较优雅的做法是实现一个头像控件, 在这个控件内部处理普通头像和自定义头像的逻辑. 为了提高用户体验, 下载的过程中我们可以显示一张替代图或者之前的普通头像.


---

下面上一张最终的效果图:

![][3]

[1]: https://github.com/crazycodeboy/TakePhoto
[2]: https://github.com/windshg/VPImageCropper
[3]: http://ww1.sinaimg.cn/large/7f870d23ly1ffd57bsbyij20f00qo0u0.jpg