---
title: "Fatal Error: Allowed Memory Size of 134217728 Bytes Exhausted"
categories: quick-x
date: 2015-04-02 13:57:52
---

今天加密脚本的时候出现了这样的错误:

```
  > get bytes [  1 KB] move.ModelsBase
  > get bytes [  1 KB] move.Move
  > get bytes [  1 KB] move.OperationsBase
  > get bytes [  1 KB] move.ViewsBase
create ZIP archive file: ../res/game.zip
PHP Fatal error:  Allowed memory size of 134217728 bytes exhausted (tried to allocate 32 bytes) in /Users/zxsk/Documents/Project2/trunk/Developer/3rdlib/quick-cocos2d-x-2.2.5/bin/lib/quick/xxtea.php on line 351
bogon:proj.tools zxsk$ sh make_package.sh 
```

在 `stackoverflow.com` 上发现[这个][1]答案:

> The correct way is to edit your php.ini file. Edit memory_limit to your desire value.


> As from your question, 128M (which is the default limit) has been exceeded, so there is something seriously wrong with your code as it should not take that much.
 

> If you know why it takes that much and you want to allow it set memory_limit = 512M or higher and you should be good.


解决方案:

在 `$QUICK-COCOS2D-X/bin/lib/quick/xxtea.php` 的文件起始位置`<?php`上方加入:

```
<?php ini_set('memory_limit', '-1'); ?>
```


解决!

[1]:http://stackoverflow.com/questions/561066/fatal-error-allowed-memory-size-of-134217728-bytes-exhausted-codeigniter-xml