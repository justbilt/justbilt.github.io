title: 优化游戏在 iOS 上的内购
date: 2017-06-18 18:58:03
tags: [iOS, IAP, Quick-Cocos2d-x]

---

首先我们看下 iOS 内购的流程, 让我们看下官方的的流程图:

![][1]

<!--more-->

很简单, 是不是 ? 作为一款游戏来说, 实际情况要复杂的多.首先要思考这么几个问题:

1. 返回 Game 层时因为内存不足游戏重启了怎么办 ?
2. 有玩家反应充值不到账怎么办 ?
3. 如何防 IAP 破解 ?

作为一款可能在 "国内" 发售的游戏, 如果你们没有考虑过这些问题, 那可能会遇到很多的 "惊喜". 有一点需要记住, 千万不要低估玩家的 "创造力". 

首先, 请允许用十分简单的几行代码带大家回顾下 IAP 的实现, 因为后面我们要讨论的内容全部是基于这个前提的. 


# IAP 的代码实现

要开启内购很简单, 两个对象三个步骤就搞定了. 


我们先来说说这两个对象, 分别是 `SKPaymentQueue` 和 `SKPaymentTransaction` , 前者是充值订单, 后者是充值队列.

## SKPaymentQueue

它是整个充值的核心, 负责串联充值的各个流程. 可以用这个对象实现发起购买, 恢复已经购买的物品, 结束购买, 监听充值事件等功能.

## SKPaymentTransaction

前面说过它是充值的订单对象, 每一次充值的发起都会产生一个唯一的订单对象, 它记录有这个订单的状态, id, 时间, 收据的信息.

其中收据(`transactionReceipt`)是最重要的数据, 它是这次充值是否真正扣款成功的唯一条件. 我们要把它发送给我们的游戏服务器, 再向苹果的服务器进行验证.

下面来说说三个步骤:

## 1. 添加充值监听

添加监听的逻辑很简单, 就下面这三行:

```objc
if ([SKPaymentQueue defaultQueue]) {
    [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
}
```

但是这个 `self` 必须是一个实现了 `SKPaymentTransactionObserver` 的类的实例, 当有充值的事件时, 类的 `updatedTransactions` 函数会被调用:

```objc
//.h
@interface IAPHelper : NSObject <SKPaymentTransactionObserver>

//.m
@implementation IAPHelper
- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions
{
    for (SKPaymentTransaction *transaction in transactions)
    {
        switch (transaction.transactionState)
        {
            case SKPaymentTransactionStatePurchased: 
                // 处理支付成功
                [[SKPaymentQueue defaultQueue] finishTransaction: transaction];
                break;
            case SKPaymentTransactionStateFailed: // 支付失败
                // 处理支付失败
                [[SKPaymentQueue defaultQueue] finishTransaction: transaction];
                break;
            case SKPaymentTransactionStateRestored: // 恢复内购
                // 处理恢复内购
                [[SKPaymentQueue defaultQueue] finishTransaction: transaction];
                break;
            default:
                break;
        }
    }
}
@end
```

## 2. 请求商品数据

在开始购买之前, 我们需要知道商品的数据, 比如价格, 描述之类的, 这就需要我们先请请求商品的数据:

```objc
SKProductsRequest* request = [[SKProductsRequest alloc] initWithProductIdentifiers:@[@"com.xxx.xxx.01", @"com.xxx.xxx.02"]];
request.delegate = self;
[request start];
```

上面就是请求商品的代码, 商品的 id 列表是需要我们自己准备的. 请求的结果会通过一个函数来通知我们, 同样我们需要实现一个代理 `SKProductsRequestDelegate`:

```objc
//.h
@interface IAPHelper : NSObject <SKProductsRequestDelegate>

//.mm
@implementation IAPHelper
- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response {
    NSArray* products = response.products;
    // products 就是商品的列表
}
@end
```

## 3. 购买

发起一次购买的逻辑:

```objc
SKPayment *payment = [SKPayment paymentWithProduct:@"com.xxx.xxx.01"];

if ([SKPaymentQueue defaultQueue]) {
    [[SKPaymentQueue defaultQueue] addPayment:payment];
}
```

下面就会进入到我们之前添加的那个充值状态的监听逻辑中去.




# 充值流程的优化

流程方面, 我们大概经过了这么下面几个阶段.

## 野生蛮长的第一阶段

这一阶段的特征是: `先完成交易, 后处理订单`, 如果仔细观察上面 `updatedTransactions` 的逻辑的话, 可以看到这样的一行代码:

```objc
[[SKPaymentQueue defaultQueue] finishTransaction: transaction];
```

这行代码的意义就是标记这个订单已经完成了, 苹果从此以后不会再去通知你这个订单的任何消息了. 在调用这行代码之前, 我们会去处理一些充值结果的逻辑, 其中充值成功我们是要去苹果验证收据的.

但是验证的这个过程是异步的, 所以可能发生的问题就是:

> 在验证结果回来之前就关闭了订单.

因此若是验证的过程不够顺利, 比如网络状况不佳, 应用意外退出, 就会导致玩家的这次充值没有到账.


## 始料不及的第二阶段

有了上面那个问题, 我们不禁会想 ? 是不是可以等收到验证结果再去完成订单. 答案是肯定的, 我们确实可以这样做, 苹果也很仁义, 没有完成的订单会在下次请求商品时再次通知我们充值成功.

简单验证之后, 我们修改了充值的逻辑, 信心满满的上线了. 上线之初确实很顺利, 反馈充值不到账的玩家数量降低了很多, 我们都陷入了巨大的喜悦之中, 再也不会有玩家反应充值不到账了, oh yeah ! 

然后, 后面发生的事情却是我们始料不及的. 我们陆续收到了用户声泪俱下的控诉, 自己是多么多么的热爱这款游戏, 但是却无法享受充值带来的乐趣. 起初还只是可能不到账, 现在则是某一个充值项完全无法购买.

在诸多玩家的控诉中, 我们逐渐还原了问题的成因. 某一次充值成功后游戏重启, 没有收到钻石, 再次购买该商品则会提示:

![][2]

> 您已购买此 App 内购买项目。此项目将免费恢复。

根据这个反馈可以很轻易的推断出是因为我们没有 `finishTransaction` 某一笔订单的原因, 但是却很难重现玩家所遇到的问题. 我们也复现过这个问题, 但是都能够在重启后自动解决. 我们也加了很多埋点去统计玩家的日志, 发现玩家似乎进入了一个卡单的状态, 程序无法获取到某些未完成的订单.

这个问题困扰了我们好久, 而且反馈的玩家越来越多, 有的玩家甚至已经没有可以能够购买的商品了.

## 键入佳境的第三阶段

虽然上面的问题我们可以将锅甩给苹果, 但是实在受到损失的是我们, 因此还是需要解决这个问题. 经过讨论之后, 我们将目光又转回了第一阶段的方案, 看能否改进哪个方案. 

> 我们可以先将订单数据保存在本地, 然后发起验证请求, 关闭订单, 等验证成功后删除本地保存的订单.

下次启动游戏或者充值时, 我们可以先检查下本地是否有缓存的订单, 有的话就先尝试验证这个订单. 虽然还有一些瑕疵, 比如玩家删除掉应用后就再也找不回之前的订单了, 但是已经是一个很不错的方案了.

下面我们来实现一下:

```objc
- (NSString*)getRecordTransaction
{
#if USE_ICLOUD_STORAGE
    NSUbiquitousKeyValueStore *storage = [NSUbiquitousKeyValueStore defaultStore];
#else
    NSUserDefaults *storage = [NSUserDefaults standardUserDefaults];
#endif
    
    NSArray *saved_transactions = [storage arrayForKey:@"transactions"];
    if (!saved_transactions or [saved_transactions count] <= 0) {
        return nullptr;
    }
    return [saved_transactions objectAtIndex:0];
}

- (void)recordTransaction:(NSString *)transaction
{
#if USE_ICLOUD_STORAGE
    NSUbiquitousKeyValueStore *storage = [NSUbiquitousKeyValueStore defaultStore];
#else
    NSUserDefaults *storage = [NSUserDefaults standardUserDefaults];
#endif
    
    NSArray *saved_transactions = [storage arrayForKey:@"transactions"];
    if (!saved_transactions) {
        // Storing the first receipt
        [storage setObject:@[transaction] forKey:@"transactions"];
    } else {
        // Adding another receipt
        NSArray *updated_transactions = [saved_transactions arrayByAddingObject:transaction];
        [storage setObject:updated_transactions forKey:@"transactions"];
    }
    
    [storage synchronize];
}

- (void)removeRecordTransaction:(NSString *)purchase_id transactionId:(NSString*) transaction_id
{
#if USE_ICLOUD_STORAGE
    NSUbiquitousKeyValueStore *storage = [NSUbiquitousKeyValueStore defaultStore];
#else
    NSUserDefaults *storage = [NSUserDefaults standardUserDefaults];
#endif
    
    NSMutableArray *saved_transactions = [NSMutableArray arrayWithArray:[storage arrayForKey:@"transactions"]];
    if (!saved_transactions or [saved_transactions count] <= 0) {
        return;
    }
    
    int index = 0;
    for (NSString* transaction in saved_transactions) {
        if ([transaction rangeOfString:purchase_id].location != NSNotFound and (not transaction_id or ([transaction rangeOfString:transaction_id].location != NSNotFound))) {
            [saved_transactions removeObjectAtIndex:index];
            [storage setObject:saved_transactions forKey:@"transactions"];
            [storage synchronize];
            return;
        }
        index = index + 1;
    }
}
```

这里我们实现了存储, 获取, 删除的逻辑, 只是参数是 `NSString` 类型而不是 `SKPaymentTransaction`. 它其实是 Transaction 的一些有用的数据和我们自己的一些信息放的一个到了一个 `NSDictionary` 中, 然后将这个 dict 转化为 json 的字符串, 这样做是为了方便存储.

## 锦上添花的第四阶段

其实第三阶段的逻辑已经很严谨了, 我们为了更好的了解玩家的充值行为, 加了很多的日志在这里:

1. 我们存下了玩家的所有充值结果, 成功的, 失败的, 取消的, 异常的.
2. 我们存下了玩家在充值界面的各种操作, 比如点击按钮, 界面跳转等.

在合适的时候, 将这些日志发送到服务器. 这些日志将会成为后面分析玩家行为, 解决纠纷的重要依据.

# 总结

我用 ProcessOn 制作了一个全新的流程图, 地址在[这里][3]:

![][4]

现在在回答下上面提出的几个问题:

## 1. 返回 Game 层时因为内存不足游戏重启了怎么办 ?

答: 我们在本地存储了充值订单, 游戏会重启, 但这个本地存储不会自动删除.

## 2. 有玩家反应充值不到账怎么办 ?

答: 这里我们要解决两个问题:

1. 是否真的没有到账 ?

可能是实际已经到账了, 只是玩家没有发现, 这个需要后端查询玩家的钻石变化后向玩家解释清楚.

2. 是否真的充值成功 ?

这个首先要玩家提供苹果充值收据邮件截图, 但是这个是可以伪造的, 但是可以阻挡一部分心怀不轨的玩家. 再根据我们前面收集的日志, 加上这个账号历史行为作出一个判断, 是否要给这个玩家补单.

## 3. 如何防 IAP 破解 ?

这个对于网游来说, 很简单, 就像在服务器端向苹果充值服务器发起验证. 需要注意的是: 服务器端在向苹果验证收据时, 一定要先检查`订单的唯一性`, `充值时间`, `商品id` 是否正常.


[1]: https://developer.apple.com/library/content/documentation/NetworkingInternet/Conceptual/StoreKitGuide/Art/remote_store_fetch_2x.png
[2]: http://ww1.sinaimg.cn/large/7f870d23ly1fgw9pxzo6ej206q04jt96.jpg
[3]: https://www.processon.com/view/link/594e7af4e4b0ad619ac50bd4
[4]: http://ww1.sinaimg.cn/large/7f870d23ly1fgwqn64m5sj20ng09laai.jpg