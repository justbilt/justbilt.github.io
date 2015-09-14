---
title: "Mac 重装系统"
categories: MacOS
date: 2015-09-06 15:57:59
---

# 一. 系统偏好设置

1. 安全性与隐私: 允许任何来源
2. 触摸板: 轻拍来点按
3. 鼠标: 辅助点按
4. 键盘: 将 F1,F2 等键用作标准功能键

# 二. Apple Store 软件

**1. Xcode**
这个必装, 安装完后需要打开一次, 同意下条款, 然后等待自动安装完成.

**2. 欧陆词典Free**
我用过的最好的翻译工具, 虽然是免费版的, 但已完全够用, 该有的全都有. 取词,划词我都用不着, 所以我都关掉了, 只保留了翻译选中内容.

![][1]

**3. QQ**
这个没啥说的, 不过我一般会关掉 Swiftly. 

![][2]

# 三. 第三方软件

**1.Chrome**
这个必备, 虽说比较吃内存, 但确实甩开 Safari 好几条街.

**2.iTerm2**

下载地址: [戳这里][4]

terminal 替代工具.

**3.Alfred**
非常可耻的使用了破解版. 我觉的 Alfred 被严重低估了的一个功能就是 ClipBoard , 真心牛逼, 我现在已经完全离不开了, 体验上秒杀所有同类粘贴板工具.

![][3]

**4.Sublime Text 3**

下载地址: [戳这里][7]

安装插件管理器:

```
import urllib.request,os,hashlib; h = 'eb2297e1a458f27d836c04bb0cbaf282' + 'd0e7a3098092775ccb37ca9d6b2e4b7d'; pf = 'Package Control.sublime-package'; ipp = sublime.installed_packages_path(); urllib.request.install_opener( urllib.request.build_opener( urllib.request.ProxyHandler()) ); by = urllib.request.urlopen( 'http://packagecontrol.io/' + pf.replace(' ', '%20')).read(); dh = hashlib.sha256(by).hexdigest(); print('Error validating download (got %s instead of %s), please try manual install' % (dh, h)) if dh != h else open(os.path.join( ipp, pf), 'wb' ).write(by)
```


**5.搜狗输入法**

下载地址: [戳这里][4]


**6. Thunder 迅雷**

Apple Store 中的版本太老了, 建议去官网下载.
下载地址: [戳这里][8]



# 四. 命令行工具

1. Homebrew

下载地址: [戳这里][6]

2. Node.js

```
brew install node
```

3. pip

```
brew install python
```

其实安装办法有好多, 但是还是这一个最省心.


[1]: /image/2015-09-06-1.png
[2]: /image/2015-09-06-2.png
[3]: /image/2015-09-06-3.png
[4]: http://pinyin.sogou.com/mac/
[5]: https://www.iterm2.com/
[6]: http://brew.sh/
[7]: https://www.sublimetext.com/3
[8]: http://mac.xunlei.com/

