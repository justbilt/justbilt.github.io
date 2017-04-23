title: Quick-cocos2d-x EditBox 几个小技巧
date: 2016-05-01 08:56:03
tags:
- Quick-Cocos2d-x
- EditBox
description: "为了应付 PM, EditBox 你需要知道的几个小技巧"
---

我们项目中的输入框使用的都是 EditBox , 但是 EditBox 还存在一些问题, 这里给大家分享一下我们的解决方案.

# 一. 字体过大

用过 EditBox 的同学都知道这样一个情况, EditBox 在创建时是无法传入字体大小的, 字体大小默认和 EditBox 的 size 一致. 如果要修改字体大小的话, 就必须有程序的参与, 十分讨厌.

而我们聪明的设计师 @大勇同学 则想到了一个非常棒的办法, 使用一张透明的9图来创建 EditBox, 后面再放置一个真实效果的 Scale9Sprite , 这样就可以实现字体比边框小很多的输入框了.


# 二. 多行输入

多行输入是一个很有必要的事情, 我们在写邮件, 军团公告等界面都有类似的需求, 然而 EditBox 并不能很好的支持多行输入, 不同平台间也存在差异, 一直很头疼这件事情. 

然而团队中另外一位成员 @小齐同学 却用另一种十分脑洞的方案解决了这个问题, 着实让人佩服. 他的思路是这样子的:

> 创建一个和需求大小一致的 EditBox, 同时创建一个 LalbelTTF , 将 dimensions 属性设置为需求大小. 处理 EditBox 使之看不见, 但又能正常输入, 同时监听输入文字变化事件, 在事件中修改 LalbelTTF 的文字.


核心就是让 EditBox 承担只文字输入的功能, 而让另外一个 LalbelTTF 来承担文字显示的功能. 实现的代码如下:

```lua
function EditBoxUtil.multiline(_editbox, _label, _params)
    _params = _params or {}

    -- 本来这个应该只是Android上的设置, 但是为了避免平台的差异性, 因此统一处理
    _editbox:setCascadeOpacityEnabled(true)
    _editbox:setOpacity(0)
    if device.platform == "android" then
        -- 避免文字过大导致Android系统崩溃
        _editbox:setFont("Helvetica",2)
    elseif device.platform == "ios" or device.platform == "mac" then
        _editbox:setFont("Helvetica",0)
    end

    _editbox:registerScriptEditBoxHandler(function(event)  
        if event == "began" then  
            _editbox:setText(_label:getString())
        elseif event == "changed" then
            _label:setString(_editbox:getText())
        elseif event == "return" then
            _label:setString(_editbox:getText())
        end
        if _params.listener then
            _params.listener(event, _editbox)
        end
    end)
end
```

这段代码和简单, 但背后所遇到的坑却不少, 且听我来道一道:

## 1. 为什么要调用 setFont

如果只是想设置字体大小, EditBox 明明有提供 `setFontSize` 接口, 为什么要调用 `setFont` ? 请看 setFontSize 实现:

```cpp
void EditBox::setFontSize(int fontSize)
{
    _fontSize = fontSize;
    if (_editBoxImpl != nullptr && _fontName.length() > 0)
    {
        _editBoxImpl->setFont(_fontName.c_str(), _fontSize);
    }
}
```

可以看到 setFontSize 在没有设置字体名称 `_fontName` 时是没有作用的.

## 2. 为什么要分平台来实现

在接入 android 前,我们是没有分平台实现的, 只是 `setFont("Helvetica",0)` , 在 iOS 上没有任何问题, 但是在 Android 上会 catch 到 `divide by zero` 崩溃, 估计是某一个地方用 fontsize 做被除数了吧 , 于是 Android 上改为设置透明度.

## 3. Android 输入过多文字后会崩溃(OOM)

崩溃在 `Cocos2dxBitmap.java` 的 `getPixels` 函数中:
```java
    private static byte[] getPixels(final Bitmap bitmap) {
        if (bitmap != null) {
            final byte[] pixels = new byte[bitmap.getWidth() * bitmap.getHeight() * 4];
            final ByteBuffer buf = ByteBuffer.wrap(pixels);
            buf.order(ByteOrder.nativeOrder());
            bitmap.copyPixelsToBuffer(buf);
            return pixels;
        }

        return null;
    }
```

在 `new byte` 这里触发了报错的原因是 `Out Of Memory` 异常 ! 调试发现 `bitmap` 的宽高惊人的达到了 `12000x600` ! 

经过 @bin 的提醒, 发现这里可能是因为 EditBox 字体过大的原因, 因为 EditBox 会默认设置字体字体大小和 Scale9Sprite 的 PreferredSize 一直, 就可能设置字体大小为 100+ , 文字一多尺寸当然就上去了! 所以便有了这么一行:

```lua
if device.platform == "android" then
    -- 避免文字过大导致Android系统崩溃
    _editbox:setFont("Helvetica",2)
```


# 二. 屏蔽 Emoji 输入

按照要求游戏中玩家可以输入文字的地方都是不能够输入 Emoji 表情的, 原因有两点:

1. 不同系统间表现存在差异
2. EditBox Android 版输入确认后会变乱码
3. 后台搜索玩家时不太方便

因此, 我们需要屏蔽 Emoji 表情的输入, 我们有两种做法:

1. 无法输入, 弹出键盘点击表情没有反应
2. 输入完成后, 游戏内点击提交时提示非法

我们采用的是第二种方案, 这个无法通过纯 lua 代码实现, 需要分平台去做.

## 1. iOS

修改 `UIEditBoxImpl-ios.mm` 文件的 `shouldChangeCharactersInRange` 函数:

```objc
- (BOOL)textField:(UITextField *) textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
    __block BOOL returnValue = NO;
    [string enumerateSubstringsInRange:NSMakeRange(0, [string length]) options:NSStringEnumerationByComposedCharacterSequences usingBlock:
     ^(NSString *substring, NSRange substringRange, NSRange enclosingRange, BOOL *stop) {
         
         if ([substring rangeOfCharacterFromSet: [NSCharacterSet characterSetWithRange:NSMakeRange(0xFE00, 16)]].location != NSNotFound) {
             returnValue = YES;
         }
         
         const unichar high = [substring characterAtIndex: 0];
         
         // Surrogate pair (U+1D000-1F9FF)
         if (0xD800 <= high && high <= 0xDBFF) {
             const unichar low = [substring characterAtIndex: 1];
             const int codepoint = ((high - 0xD800) * 0x400) + (low - 0xDC00) + 0x10000;
             
             returnValue = (0x1D000 <= codepoint && codepoint <= 0x1F9FF);
             
             // Not surrogate pair (U+2100-27BF)
         } else {
             returnValue = (0x2100 <= high && high <= 0x27BF);
         }
     }];
    
    if (returnValue) {
        return NO;
    }
    
    
    if (getEditBoxImplIOS()->getMaxLength() < 0)
    {
        return YES;
    }
    
    NSUInteger oldLength = [textField.text length];
    NSUInteger replacementLength = [string length];
    NSUInteger rangeLength = range.length;
    
    NSUInteger newLength = oldLength - rangeLength + replacementLength;
    
    return newLength <= getEditBoxImplIOS()->getMaxLength();
}
```

这段代码是我从 [https://github.com/woxtu/NSString-RemoveEmoji][1] 中提取出来的.

## 2. Android

Android 上的实现也很简单, 主要是需要创建一个新的 `InputFilter` 用来过滤 Emoji 表情. 需要修改 `Cocos2dxEditBoxDialog.java` 文件成员变量添加:

```java
    public static InputFilter EMOJI_FILTER = new InputFilter() {

        @Override
        public CharSequence filter(CharSequence source, int start, int end, Spanned dest, int dstart, int dend) {
            for (int index = start; index < end; index++) {
                int type = Character.getType(source.charAt(index));
                if (type == Character.SURROGATE || type == Character.OTHER_SYMBOL || type == Character.PRIVATE_USE) {
                    return "";
                }
            }
            return null;
        }
    };
```

修改 `onCreate` 函数 `setFilters` 处逻辑:

```java
    if (this.mMaxLength > 0) {
        this.mInputEditText.setFilters(new InputFilter[] { new InputFilter.LengthFilter(this.mMaxLength), EMOJI_FILTER });
    }else{
        this.mInputEditText.setFilters(new InputFilter[] { EMOJI_FILTER });
    }
```

---

关于 Emoji 的相关修改都已经推送到了 github 上, [点击这里][2]查看.


[1]: https://github.com/woxtu/NSString-RemoveEmoji
[2]: https://github.com/mafiagame/quick-cocos2d-x/commit/8a7c27aedb919e0ae5d178d688b704e620c2c1bb