title: 聊聊 Google 支付的那些事
date: 2017-07-23 10:20:07
tags: [Android, IAB]
---

在之前的文章中, 有很多篇都是很 Apple IAP 相关的, 而 Google IAB 的却一篇都没有, 这对于占了半壁江山的 Android 很不公平.

就作为一个开发者而言, IAB 的体验是比 IAP 要好的. 它体现在这么几个方面:

1. 测试账号就是 Goolge 账号, 不像苹果那样是沙盒账号. 这样不同账号下的应用可以设置同一个测试账号, 避免来回切换账号.
2. 支持退款查询. 在苹果上, 只能吃了这个哑巴亏.

但是 IAB 也有一些不太便利的地方, 后面我们会讨论它.

<!--more-->

# 一. 集成

IAB 本身提供了一些列的 [API][2] 去实现购买, 一次典型的购买流程是这个样子的:

![][1]

这里面有一些细节我们其实是可以不用知道的, 所以我们使用了一个二次封装的库[android-inapp-billing-v3][3]去实现. 使用了这个库后, 实现内购变得很简单:

## 1. 添加监听

```java
protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    bp = new BillingProcessor(context, "IAB_LICENSE_KEY", new BillingProcessor.IBillingHandler() {
        @Override
        public void onProductPurchased(String productId, TransactionDetails details) {
            // 购买成功
            Log.d("message:", details.purchaseInfo.responseData);
            Log.d("signature:", details.purchaseInfo.signature);
            Log.d("productid:", productId);
        }
        @Override
        public void onBillingError(int errorCode, Throwable error) {
            if (errorCode == Constants.BILLING_RESPONSE_RESULT_USER_CANCELED){
                // 购买取消
                return;
            }

            String reason = "UNKNOWN_ERROR";
            if (mErrorCodeReason.containsKey(errorCode)) {
                reason = mErrorCodeReason.get(errorCode);
            }
            // 购买失败
            Log.d("buy failure: ", reason);
        }
        @Override
        public void onBillingInitialized() {
            // 初始化成功
        }
        @Override
        public void onPurchaseHistoryRestored() {
            // 恢复内购
        }
    });
}
```

## 2. 处理购买结果 

```java
@Override
protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    super.onActivityResult(requestCode, resultCode, data);
    return bp.handleActivityResult(requestCode, resultCode, data);
}  
```

## 3. 购买商品

```java
    bp.purchase(this, "android.test.purchased");
```

## 4. 消耗商品

```java
    bp.consumePurchase(productId);
```

# 二. 测试

## 1. 使用静态响应进行测试

IAB 默认提供了一系列的商品作为测试商品, 这些商品和应用及商品无关, 我们可以使用这些商品来测试充值功能是否集成成功和服务器签名验证是否正确.

详情可以参考 [这篇 Google 官方文档][4].

## 2. 使用真实交易进行测试

在项目上线前, 我们需要测试下应用内的商品处理是否正常, 所以要使用真实的环境来测试. 要达到这个目的, 我们需要进行一些列的操作才可以达到.

+ 正确创建商品并与项目关联.
+ Google Play 后台已经上传并发布了一个 Bata/Alpha 版本.
+ 使用和 Google Play 后台已经发布的应用相同的签名和版本打包出来的应用.
+ 在 Google Play 后台添加[测试账号][5].
+ 确认应用的发布地区有这个测试账号的所在地区.
+ 确保测试用手机安装了谷歌服务框架, 开启翻墙并登陆了上面添加的测试账号.

这只是其中一部分需求, 还有一些原因会导致你无法正确测试充值.

# 三. 常见错误及解决方案

## 1. 需要验证身份. 您需要登录自己 Google 账号

![][6]

解决方案:

> 商品id不正确, 请检查下购买时传入的商品 id 和 Google Play 后台是否一致

## 2. 此版本的应用未配置为通过 Google Play 结算. 

![][7]

解决方案:

> 可能在测试账号所在的区没有包含在应用的发售地区中, 请检查配置, 修改后需要耐心等待生效.

## 3. 无法购买您要买的商品.

![][8]

解决方案:

> 封闭测试需要点击链接成为测试人员, 连接形式: `https://play.google.com/apps/testing/xxx`

Ps. 这个是很早之前的一个错误, 貌似 Google 目前已经不再检查版本号这个属性了.


## 4. 从服务器检索信息出错.[DF-DIC-02]

![][9]

> 可能是商品 id 不正确或者支付逻辑错误导致传入了错误的商品 id, 详细原因可以移步我知乎上的[这个回答][10]

## 5. 添加测试账号

测试账号分为两种:

1. 下载测试账号列表
    1. 如果应用处于未发布的状态, 只有这个列表中的用户可以充值
2. 充值测试账号列表
    1. 这个列表中的用户可以免费充值

测试充值这两个缺一不可.


# 最后

IAB 最麻烦的地方就是它的测试充值, 原因在于它的错误提示不是 "人话", 这个只能靠我们的丰富经验来解决了.


参考资料:

1. [http://blog.csdn.net/n5/article/details/50705745][11]
2. [https://developer.android.com/google/play/billing/index.html][12]

[1]: https://ws1.sinaimg.cn/large/006tNc79gy1fhtste5cqlj30fn0fkta9.jpg
[2]: https://developer.android.com/google/play/billing/billing_reference.html
[3]: https://github.com/anjlab/android-inapp-billing-v3
[4]: https://developer.android.com/google/play/billing/billing_testing.html#billing-testing-static
[5]: https://developer.android.com/google/play/billing/billing_admin.html#billing-testing-setup
[6]: https://ws1.sinaimg.cn/large/006tKfTcgy1fhzbhgvtu7j30dw079gmc.jpg
[7]: https://ws2.sinaimg.cn/large/006tKfTcly1fhzbtvd43jj30dw079gmd.jpg
[8]: https://ws4.sinaimg.cn/large/006tKfTcly1fhyskeq0czj30du08caae.jpg
[9]: https://ws3.sinaimg.cn/large/006tNc79gy1fhtuo2rcywj30ds08fq3a.jpg
[10]: https://www.zhihu.com/question/26935519/answer/182780390
[11]: http://blog.csdn.net/n5/article/details/50705745
[12]: https://developer.android.com/google/play/billing/index.html
