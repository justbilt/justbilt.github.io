---
title: 将 python 数据转化为 lua 格式
date: 2017-09-03 15:45:53
tags: [Python, Lua]
---

我们团队的技术栈是: `Lua + Python`, 前端采用 Lua 写项目, Python 做工具支持, 而后端则采用纯 Python 开发, 所以会经常遇到 Python 和 Lua 数据的转化.

<!--more-->

目前业内采用的方法大致有两种:

1. 用 Python 写一个 Lua 的语法解析, 用 Lua 写一个 Pyhton 的语法解析, 这样就可以互相转化了.
2. 使用一个大家都支持的数据结构做中转, 如: Json 或 Yaml.

第一种方案比较复杂, Python 解析 Lua 倒是有 [slpp][1] 这样的项目, Lua 解析 Python 的则是没有, 因此大家更多的是采用第二种方案.

第二种方案中 json 其实是一个很不错的解决方案, 性能也比 Lua 快一些, 但是 json 没有办法存储 number 类型的 key, 这一点在数据结构的设计上很蛋疼, 可能会让你的代码充斥着 `tostring`, `tonumber` 这样的代码.

经过观察, json 和 Lua 这两种文件格式其实是十分像的, 那么我是否只要魔改下 python 的 json 库, 使之支持 number 做 key, 然后保存为 Lua 文件 ?

# 开撸


## 1. 准备工作

首先我们找到 json 库的源码 [https://github.com/python/cpython/blob/master/Lib/json][2], 这个目录中有 4 个文件, 我们把 `encoder.py` 下载下来, 保存为 `lua.py`.

但是只有这一个文件是没有办法直接运行的, 我们再从 `encoder.py` 同级的 `__init__.py` 中复制 `dumps` 函数到 `lua.py` 的末尾.

```python
def dumps(obj, skipkeys=False, ensure_ascii=True, check_circular=True,
              allow_nan=True, cls=None, indent=None, separators=None,
              encoding='utf-8', default=None, sort_keys=False, **kw):
       if cls is None:
              cls = JSONEncoder
       return cls(
              skipkeys=skipkeys, ensure_ascii=ensure_ascii,
              check_circular=check_circular, allow_nan=allow_nan, indent=indent,
              separators=separators, encoding=encoding, default=default,
              sort_keys=sort_keys, **kw).encode(obj)
```


我们在 `lua.py` 同级目录新建一个 `test.py` 做测试, 这里我们把一个python的数据保存为 `test.lua` 文件:

```python
import lua
data = {
    "a": 1,
    2:2,
    "3a": 3,
    "4":[
        1,2,"a",False
    ]
}
with open("test.lua", "w") as f:
    f.write(lua.dumps(data, ensure_ascii=False, indent=2, sort_keys=True))
```


完成后直接终端运行 `python test.py` 就可以看到一个新生成了一个 lua 文件, 内容如下:

```lua
{
  "2": 2, 
  "3a": 3, 
  "4": [
    1, 
    2, 
    "a", 
    false
  ], 
  "a": 1
}
```

我们第一步的工作就搞定了.


## 2. 魔改 lua.py

其实上一步生成的文件内容并不是 lua 格式的, 其实还是 json 格式, 只是我们将后缀改为 lua 了而已. 这个格式和真正的 lua 想比有两处不对:

1. Lua 的数组使用 `{}` 表示.
2. Lua 使用 `=` 做 key,value 的分隔.
3. Lua string 类型的 key, 不用加引号, 如果要加引号的话, 需要同时加一对方括号.

第一个问题很好解决, 我们搜索 `'[`, 可以找到 `_iterencode_list` 这个函数, 大概看一眼就知道怎么改了:

```python
    def _iterencode_list(lst, _current_indent_level):
        if not lst:
            yield '{}'
            return
        ...
        buf = '{'
        ...
        yield '}'
        ...
```

我们修改完, 运行 `python test.py` 看下效果:

```lua
{
  "2": 2, 
  "3a": 3, 
  "4": {
    1, 
    2, 
    "a", 
    false
  }, 
  "a": 1
}
```

是不是已经有点意思了? 我们再接再厉, 找找第二个问题如何解决. 我们搜索 `':` 可以找到这行代码:

```python
    key_separator = ': '
```

将 `': '` 修改为 `' = '` 就可以啦, 我们看下结果:

```lua
{
  "2" = 2, 
  "3a" = 3, 
  "4" = {
    1, 
    2, 
    "a", 
    false
  }, 
  "a" = 1
}
```

那么第三个问题该如何入手呢? 给童鞋们三秒钟的思考时间. 

> 3s
> 
> 2s
> 
> 1s

揭晓答案, 既然 `key_separator` 是分隔 key, value 的, 我们找找哪里用到 key_separator 是不是发现点什么呢 ? 很快我们能定位到 `_iterencode_dict` 函数的这行代码:

```python
    yield _encoder(key)
    yield _key_separator
```

这个 `_encoder(key)` 会不会对应 key 的生成呢? 我们修改下试试, 将这句话替换为:

```python
    yield '[' + _encoder(key) + ']'
```

运行, 结果如下:

```lua
{
  ["2"] = 2, 
  ["3a"] = 3, 
  ["4"] = {
    1, 
    2, 
    "a", 
    false
  }, 
  ["a"] = 1
}
```

哎呀, 我这是搞定了吗? 骚年, 恭喜你, 你已经完成了 99% 的工作了, 这段代码的最前方还少一个 `return` , 我们可以很轻易的加上. 我通过修改 `test.py` 实现了这个功能:

```pyhton
    f.write(lua.dumps(data, ensure_ascii=False, indent=2, sort_keys=True))
    # 修改为:
    f.write("return " + lua.dumps(data, ensure_ascii=False, indent=2, sort_keys=True))
```

结果:

```lua
return {
  ["2"] = 2, 
  ["3a"] = 3, 
  ["4"] = {
    1, 
    2, 
    "a", 
    false
  }, 
  ["a"] = 1
}
```

## 3. 更加 lua 化

上面生成的 `test.lua` 其实很不 Lua, 大家不会写出 `["a"] = 1` 这种类型的语句, 而是直接 `a = 1`. 

这个问题发生在了上面的第三步上, 我们偷懒直接写出了这样的代码:

```python
yield '[' + _encoder(key) + ']'
```

大家不妨看看 `_encoder` 这个函数, 它会把任意类型转化为字符串, 两端加上 `"`. 那么 Lua 在什么情况下才会 `["x"]` 这种类型的 key 呢?

1. 关键字, 如: `require`, `false` 等, 我们可以在[这里][3]找到所有的关键字.
2. 以数字开头的字符串.
3. 数字, 包括 `int`, `lang`, `float`.
4. 含有奇怪字符的字符串, 如: `-`, `*` 等, 非奇怪字符只有: `[a-zA-Z_0-9]`.

知道了规则, 我们就很好办了, 写一个解析函数就好了嘛:

```python
key_filter = re.compile("[^a-zA-Z_0-9]")
key_filter_equal = {"require","false","for","function","if","nil","not","or","while","then","true","until","end","in","local","repeat","return","break","do","else","and","elseif"}
def format_key(s):
    if isinstance(s, (int, long, float)):
        return '['+str(s)+']'
    try:
        int(s)
        return '["'+s+'"]'
    except Exception, e:
        pass
    for f in key_filter_equal:
        if s == f:
            return '["'+s+'"]'
    if (ord(s[0]) >= ord("0") and ord(s[0]) <= ord("9")) or key_filter.search(s):
        return '["'+s+'"]'
    return s
```

然后把 `yield '[' + _encoder(key) + ']'` 替换为 `yield format_key(key)` 就搞定了. 我们看下生成的结果:

```lua
return {
  ["2"] = 2, 
  ["3a"] = 3, 
  ["4"] = {
    1, 
    2, 
    "a", 
    false
  }, 
  a = 1
}
```

好像不太对嘛, 骗纸, 数字的 key 还是有问题的. 不要慌, 我们顺着 `format_key` 调用的地方向上找找, 发现问题了吧:

```python
            elif isinstance(key, float):
                key = _floatstr(key)
            elif key is True:
                key = 'true'
            elif key is False:
                key = 'false'
            elif key is None:
                key = 'null'
            elif isinstance(key, (int, long)):
                key = str(key)
```

可以看到这里已经对 `float`, `int`, `lang` 类型做过转化了, 我们把 elif 中的内容换成 `pass` 再试下:

```lua
return {
  [2] = 2, 
  ["3a"] = 3, 
  ["4"] = {
    1, 
    2, 
    "a", 
    false
  }, 
  a = 1
}
```

哈哈, 搞定了! 我太棒了! 咦, 纳尼, Are you kidding me ? 为什么只有 `2` 是 ok 的, `"4"` 是怎么回事嘛 ? 仔细检查了一番代码, 有做了一个额外的修改, 还是没有发现原因.

当我把目光转向我们的测试代码的时候, 我看到的答案:

```python
data = {
    "a": 1,
    2:2,
    "3a": 3,
    "4":[
        1,2,"a",False
    ]
}
```

我们的 `"4"` 本身就是 string 类型的, 这样更加说明了我们代码的正确性.

---

到此为止, 我们的魔改之旅就结束了, 我把最终版的代码放到了 [Gist 上][4], 大家如果有兴趣的话可以看一下.

# 附录:

一个 24W 行的 json 和 Lua 读取时间:

```lua
time1 = os.clock()
data = require("filters")
time2 = os.clock()
data2 = json.decode(io.readfile(cc.FileUtils:getInstance():fullPathForFilename(("res/filters.json"))))
time3 = os.clock()
print("time lua:", time2 - time1)
print("time json:", time3 - time2)
```

结果如下:

```
time lua:   0.278526
time json:  0.182085
```


[1]: https://github.com/SirAnthony/slpp
[2]: https://github.com/python/cpython/blob/master/Lib/json
[3]: https://www.lua.org/manual/5.1/manual.html#2.1
[4]: https://gist.github.com/justbilt/a5cfafe761b8571e7b5a9e7c22cc7c57
