title: Quick-cocos2d-x 编译 Lua 代码加速
date: 2017-09-10 10:37:53
tags: [Quick-Cocos2d-x, Lua]
---

quick-x 在手机上运行的时候并不是加载项目的 Lua 的源码, 而是一个名为 `game.zip` 的压缩包. 当项目中的 Lua 文件越来越多的时候, 生成这个 game.zip 的时间会越来越久.

我们目前的项目有 `1256` 个 Lua 文件, 每次执行编译操作需要耗时将近 40 秒的时间:

```sh
time $QUICK_V3_ROOT/quick/bin/compile_scripts.sh -q -i ../src -o ../res/game.zip -e xxtea_zip -ek xxxx -es xxx
---
real    0m38.603s
user    0m25.792s
sys     0m3.369s
```

尤其是当需要在手机上调试 Lua 代码的时候, 每改一次代码就需要运行一次这个命令, 然后 40 秒就过去了, 你恐怕都忘记自己该关注什么改动了吧!

<!--more-->

# 分析 compile_scripts

在 `$QUICK_V3_ROOT/quick/bin` 目录我们可以找到 `compile_scripts.sh` :

```sh
#!/bin/bash
export LUA_PATH="$QUICK_V3_ROOT/quick/bin/mac/?.lua;;"
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
php "$DIR/lib/compile_scripts.php" $*
```

这个脚本很简单, 只是一个入口, 真正的逻辑是在 `compile_scripts.php` 文件中, 打开这个文件后发现它主要功能是处理输入的参数, 剩余的工作是通过 `ScriptsCompiler` 这个类完成的, 它位于 ScriptsCompiler.lua 中.

必须承认, 廖大他们写的代码还是很棒的, 我们只看 ScriptsCompiler 的 `run` 函数便可以猜到这个类都干了那些事情:

```php
    function run()
    {
        $files = $this->searchSourceFiles();
        $modules = $this->prepareForCompile($files);
        if ($this->config['encrypt'] == self::ENCRYPT_XXTEA_CHUNK)
        {
            $bytes = $this->compileModules($modules, $this->config['key'], $this->config['sign']);
        }
        else
        {
            $bytes = $this->compileModules($modules);
        }
        if (!is_array($bytes))
        {
            $this->cleanupTempFiles($modules);
            return false;
        }
        if (!$this->createOutput($modules, $bytes))
        {
            $this->cleanupTempFiles($modules);
            return false;
        }
        $this->cleanupTempFiles($modules);
        return true;
    }
```

配合着其中调用的函数的细节, 我们可以得知该类的主要工作流程:

1. 找到所有的 Lua 文件.
2. 调用 `getScriptFileBytecodes` 函数获取上一步那些 Lua 文件的字节码.
3. 创建输出文件 `game.zip`.
4. 加密输出文件 `game.zip`.

当然还有一些分支的情况, 不过我们关注的工作流程就是这个样子的.


# 快速优化

从上一步的源码阅读过程中得知, 如果我们在调用 `compile_scripts.sh` 时, 不传入加密信息的话, 就不会第四步操作. 在我们开发过程中, 加密这一步其实是可以省略, 最终打包的时候加密一次就可以了.

那么干掉第 4 步对于打包花费到底能减少多少呢 ? 我们来测试一下, 还是文章最开始的那端脚本, 我们这次删除最后面的一些加密参数试一下:

```sh
time $QUICK_V3_ROOT/quick/bin/compile_scripts.sh -q -i ../src -o ../res/game.zip
---
real    0m16.362s
user    0m8.333s
sys     0m2.579s
```

很幸运, 加密确实消耗了很多的时间, 因此我们没有花费多少精力就把编译时间减少为了当初的 `42%`. 我们可以在游戏工程中建立两个脚本, `pack_lua.sh` 和 `pack_lua_dev.sh`, 分别放入上述的两行命令, 这样我们在开发/调试期间只用调用 `pack_lua_dev.sh` 就可以了.

然而这一切还不值得我们欢呼, 这是一个很取巧的方案, 或许是我们团队本身就对 `compile_scripts` 这个操作的理解不够, 导致我们一直以来就来使用的这个方案就是错误的.

况且, `16` 秒还是很久.


# 深度优化

根据上一步得出的信息, 有 16 秒的时间是消耗在了前三步, (根据多年经验)而其中第一步和第三步的消耗也非常小, 那么主要的消耗是在第二步上.

经过观察, 每次我们运行 `compile_scripts` 都会把所有的 Lua 代码都编译一遍, 那么我们是不是可以把历史的结果缓存下来, 只编译新增和变化的文件呢 ?

首先我们需要弄懂, 这个所谓的 `编译` 操作, 到底做了什么 ? 让我们再次把目光转向之前得知的 `getScriptFileBytecodes` 函数.

该函数位于 `$QUICK_V3_ROOT/quick/bin/lib/quick/init.php` 中, 我们在函数中执行命令前加上输出:

```php
    ...
    printf("command: %s\n", $command);
    passthru($command);
    ...
```

再次执行 `compile_scripts`, 注意去掉 `-q` 参数, 我们可以在输出中看到:

```sh
command: /xxx/quick-cocos2d-x/quick/bin/mac/luac -o "xxx.bytes" "xxx.lua"
```

看来就是调用了下 `luac` 命令嘛, 挺简单的呀!

> (注: 实际情况会复杂一些, 因为可能会用到 luajit 工作流程, 这里只讨论我们目前的工作流)

搞懂了原理的话, 就可以开撸了, 我并不打算修改 quick 的 php 代码, 我会用我更熟悉的 Python 来写一个优化版的 Lua 编译脚本. 下面是核心的代码, 我加了一些必要的注释:

```python
def package(_src, _dst):
    cache_path = CACHE_ROOT
    shutils.mkdir(cache_path)
    hash_file = os.path.join(cache_path, "hash.json")
    hash_data = shutils.read_json(hash_file, {})
    hash_data_new = {}
    shutils.remove(_dst)
    dst_zip = zipfile.ZipFile(_dst, "w", zipfile.ZIP_DEFLATED)
    for f in shutils.file_list(_src):
        if f.startswith(".") or not f.endswith(".lua"):
            continue
        src_filepath = os.path.abspath(os.path.join(_src, f))
        dst_filename = f.replace(".lua", "").replace("/", ".")
        dst_filepath = os.path.abspath(os.path.join(cache_path, dst_filename))
        src_hash = shutils.file_hash(src_filepath)
        dst_hash = shutils.file_hash(dst_filepath)
        record_hash = hash_data.get(dst_filename, {})
        exists_error = False
        # 检查缓存是否有效
        if not (dst_hash and record_hash.get("src", "") == src_hash and record_hash.get("dst", "") == dst_hash):
            # 这里为移除时为了感知 luac 编译失败
            shutils.remove(dst_filepath)
            print "ADD: " + f
            luac_compile(src_filepath, dst_filepath)
            dst_hash = shutils.file_hash(dst_filepath)
            if not dst_hash:
                print "\n---"
                print "ERROR: " + src_filepath
                exists_error = True
                break
        dst_zip.write(dst_filepath, dst_filename)
        hash_data_new[dst_filename] = {"src": src_hash, "dst": dst_hash}
    shutils.save_json(hash_data_new, hash_file)
    if exists_error:
        return False
    dst_zip.close()
    # 删除原始文件已经不存在的缓存
    for f in shutils.file_list(cache_path):
        if f == "hash.json":
            continue
        if not hash_data_new.get(f):
            print "REMOVE: " + f
            os.remove(os.path.join(cache_path, f))
    return True
```

这段代码里多是逻辑的排列, 没有什么好说的. 值得一提的是是, luac 编译失败的处理, luac 失败后会输出错误日志, 不会生成对应的字节码:

```sh
$ python luapack.py src res/game.zip
ADD: main.lua
/xxx/quick-cocos2d-x/quick/bin/mac/luac: /xxx/src/main.lua:10: '=' expected near 'package'

---
ERROR: /Users/bilt/Documents/wanghaitao/easy_slg_client/src/main.lua
```

因此我们先尝试移除掉目标文件, 如果生成文件不存在则表示本次 luac 失败了. 

完整版的代码位于[我的 gist 上][1], 这份代码只是最基本的实现, 还有很多优化的空间. 好的, 我迫不及待的想看看我们优化的成果了.

让我们看看第一次运行的结果:

```sh
$ time python luapack.py src res/game.zip
---
real    0m15.590s
user    0m6.664s
sys     0m2.946s
```

`16` 秒, 接近之前无加密版的 `compile_scripts` 消耗, 让我们变更一个文件试试呢?

```sh
$ time python luapack.py src res/game.zip
ADD: main.lua
---
real    0m1.611s
user    0m1.403s
sys     0m0.175s
```

只有不到 2 秒的耗时, 太牛逼了!

---

在本次的优化过程中, 我们成功的将耗由最初的 `38.603s` 降低为最终的 `1.611s`, 接近 `24` 倍的效率提升, 这是一个质的飞跃! 感觉自己棒棒哒,晚饭申请加一个鸡腿.


当然我们还做了一些其他的东西, 比如我们给日志加上了彩色的输出:

![][2]

是不是变的很漂亮 ?

[1]: https://gist.github.com/justbilt/7c881cbdbff74f3fdd97278355d22fed
[2]: https://ws4.sinaimg.cn/large/006tKfTcly1fjep3wv9cjj30vz03uq3p.jpg