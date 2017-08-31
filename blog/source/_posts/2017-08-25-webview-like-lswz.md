title: 手撸一个像《乱世王者》那样的 WebView
date: 2017-08-25 00:45:08
tags: [Quick-Cocos2d-x, WebView]
---

天美新游戏《乱世王者》出了之后, 我玩了一阵子, 这个游戏的精美程度刷新了我对 slg 游戏的认识, 比它的模型产品《列王的纷争》给我带来的震撼还要大. 不仅仅是在美术方面, 更多的是在产品和对细节的处理上. 

![][8]

<!--more-->

在游戏中更多界面中有几个按钮, 点击后会跳转到 WebView, 如下图:

![][1]

这个 WebView 做得很不错呀, 比 Cocos 自带的光秃秃的 WebView 好数倍, 本着从(hou)善(yan)如(wu)流(chi)的人生哲理, 便开始思索着是不是搞到我们的游戏中去? 

那么它是如何实现的呢? 根据多年经验, 从资源入手是最简单的办法了, 拆开乱世王者的 Android 包, 在 res 文件夹中搜索 png, 我们很容易的发现下面这些资源:

![][2]

命名都是类似: `com_tencent_msdk_webview_left_unclickable.png`, 看来是 `msdk` 里面的一个组件, 我们没有接入 msdk 的计划, 所以没有办法直接调用了. 让我们来分析下这个界面:

![][3]

Cocos 默认提供的 WebView 只有中间部分, 上下是没有的. 这两部分应该是用 Android 写的原生界面, 但是我对 Android 也是一知半解, 写出来估计都是 bug, 还没有办法热更新. 

看着满地的资源, 我不禁陷入了深思. 有了, 是不是只创建出中间尺寸大小的 Webview, 然后用这些图片在 Cocos 中绘制出标题栏和工具栏, 菜单和 WebView 的交互可以用 WebView 提供的 api 来搞. 

粗略看了下 WebView 提供的 api:

```cpp
    bool canGoBack();
    void goBack();
    bool canGoForward();
    void goForward();
    void reload();
```

在加上 `removeFromParent`, 下方的工具栏是没有问题了. 我们 ui 编辑器使用的是 Cocos Builder, 我很快拼出了效果:

![][4]

下面便是在代码中撸这几个菜单的逻辑了, 这部分还是挺麻烦的, 只要细心一些, 很快也能搞定:

```lua
--
-- Author: wangbilt@gmail.com
-- Date: 2017-08-21 11:26:18
--
local PageFullscreenWebview = class("PageFullscreenWebview", require("app.layout.package.page_fullscreen_webview_layout"))
function PageFullscreenWebview:ctor()
    PageFullscreenWebview.super.ctor(self)
    self:_disableToolBar()
    self.webview = ccexp.WebView:create()
    self.webview:setAnchorPoint(cc.p(0, 0))
    self.webview:setScalesPageToFit(false)
    self.webview:setContentSize(cc.size(display.width, display.height - self._nodeToolBar:getContentSize().height))
    self.webview:setPosition(0, self._nodeToolBar:getContentSize().height)
    self.webview:setOnDidFinishLoading(function()
        self:performWithDelay(handler(self,self._loadFinishCallBack), 0)
    end)    
    self:addChild(self.webview)
end
function PageFullscreenWebview:loadURL(_url, _callback)
    self.webview:loadURL(_url)
    self.on_finish_callback = _callback
    self._btnRefresh:setVisible(false)
    self._btnLeft:setButtonEnabled(false)
    self._btnRight:setButtonEnabled(false)    
end
function PageFullscreenWebview:_disableToolBar()
    self._btnLeft:setButtonEnabled(false)
    self._btnRight:setButtonEnabled(false)
    self._btnRefresh:setVisible(false)
end
function PageFullscreenWebview:_loadFinishCallBack(...)
    self._btnRefresh:setVisible(true)
    self._btnLeft:setButtonEnabled(self.webview:canGoBack())
    self._btnRight:setButtonEnabled(self.webview:canGoForward())
    if self.on_finish_callback then
        self.on_finish_callback()
    end
end
function PageFullscreenWebview:onClose(_callback)
    self.on_close_callback = _callback
end
function PageFullscreenWebview:onClickClose(_params)
    print("onClickClose",_params.tag, _params.target)
    self.on_close_callback()
    self:removeFromParent()
end
function PageFullscreenWebview:onClickLeft(_params)
    print("onClickLeft:",_params.tag, _params.target)
    self:_disableToolBar()
    self.webview:goBack()
end
function PageFullscreenWebview:onClickRefresh(_params)
    print("onClickRefresh:",_params.tag, _params.target)
    self:_disableToolBar()
    self.webview:reload()
end
function PageFullscreenWebview:onClickRight(_params)
    print("onClickRight:",_params.tag, _params.target)
    self:_disableToolBar()
    self.webview:goForward()
end
function PageFullscreenWebview:show()
    display.getRunningScene():addChild(self, 10000)
end
return PageFullscreenWebview
```

看看实际的效果:

![][5]

> Ps. 在这之间还遇到了一个bug, 就是在 WebView 出现的时候按 back 键会崩溃, 最终重写 Avtivity 一个空的 `onBackPressed` 实现解决问题.


是不是足以以假乱真了呢 ? 不要高兴太早, 还有标题,进度条功能没有实现呢. 让我们来分析下这几个功能:

## 标题

看了下 Cocos 提供的 API, 是没有与标题相关的, 只能我们自己去绑定. 那么该如何获取标题呢?

### Android:

在 [stackoverflow 的这个答案][5] 中找到解决方案:

```java
    WebView mWebView = (WebView) findViewById(R.id.mwebview);
    mWebView.setWebViewClient(new WebViewClient() {
        @Override
        public void onPageFinished(WebView view, String url) {
            ExperimentingActivity.this.setTitle(view.getTitle());
        }
    });
```

或者

```java
webview.setWebChromeClient(new WebChromeClient() {
    @Override  
    public void onReceivedTitle(WebView view, String title) {  
        super.onReceivedTitle(view, title);  
    } 
});
```

### iOS:

同样, 我们可以很容易的找到 iOS 的[解决方案][7]:

```objc
- (void)webViewDidFinishLoad:(UIWebView *)webView{
    NSString *theTitle=[webView stringByEvaluatingJavaScriptFromString:@"document.title"];
}
```

有了上面代码后, 我们可以很容易的实现获取标题的代码, 比较麻烦的是还需要修改 WebView 的 c++ 部分, 如果最终要在 Lua 中使用的话还需要导出接口.

## 进度条

### Android:

```java
webview.setWebChromeClient(new WebChromeClient() {
    public void onProgressChanged(WebView view, int progress)   
    {
    }
});
```

### iOS

iOS 我找了很久, 貌似是没有办法获取进度的, 只能做一个假的效果:

> 在开始加载网页的时候启动一个定时器, 让进度条以一个缓慢的速度前进, 收到加载完成的消息时让进度条瞬间移动到终点.

所以, 我们可以分平台实现, 也可以统一代码, 都使用假的进度条.

---

经过与 PM 的一番交流后, 一致同意放弃这两个功能. 因为觉得目前的效果还可以, 加上不改动 c++ 就可以热更新上去.

所以很可惜, 这次没有办法搞彻底了, 下次有机会我们接着搞.

[1]: https://ws4.sinaimg.cn/large/006tNc79ly1fj3pou45vcj30af0ijn2s.jpg
[2]: https://ws2.sinaimg.cn/large/006tKfTcly1fixluudqm1j30yo0jcn3g.jpg
[3]: https://ws3.sinaimg.cn/large/006tKfTcly1fiy0x704zhj309h0dzmxd.jpg
[4]: https://ws2.sinaimg.cn/large/006tNc79ly1fj110v95r4j31kw1f9127.jpg
[5]: https://ws4.sinaimg.cn/large/006tNc79ly1fj11145fcrj30f00qoab6.jpg
[6]: https://stackoverflow.com/a/8193479
[7]: https://stackoverflow.com/a/20059707
[8]: https://ws1.sinaimg.cn/large/006tNc79ly1fj3pcpg3k8j31kw0xw4qp.jpg

