title: 将 python 数据转化为 TypeScript 格式
date: 2017-12-24 21:33:03
tags:
---

前段时间写过这篇文章[将 python 数据转化为 lua 格式][1], 这段时间因为新项目改用 `Creator + TypeScript` 的原因, 需要导出 ts 格式的数据.

当然我们可以选择使用 json/yaml 等格式作为数据文件, 这会简单很多, 但是有两个原因, 使得我们一致认为 ts 格式的数据是更好的选择:

1. 访问数据时 VSCode 会根据数据内容给出提示
2. 打包时会被编译加密, 省去自己加密数据文件的工作

<!--more-->

其中第一点是最为重要的, 这也是我们立项之初就选择 `TypeScript` 的重要原因:

![][2]

这太棒了, 下面就让我们看看这个文件是如何生成的吧.

# 魔改 lua.py

我们直接在前一篇文章的 [lua.py][3] 的基础上改, 这样会节省一些准备工作. 首先我们来对比一下 lua 和 ts 语法的区别:

```lua
 {
    name = "Nick", 
    condition_VIP_level = 0, 
    item = {
        [1] = 5, 
        [2] = 10
    }
}
```

```typescript
 {
    name: "Nick", 
    condition_VIP_level: 0, 
    item: [
        5, 
        10,
    ], 
}
```

从上面的示例可以看出, 这两个语言还是蛮像的, 只是有一些细微的区别.


## Dict

lua 使用 ` = `, 而 ts 使用 `: `, 我们直接修改:

```python
key_separator = ': '
```

另有一个小细节, tslint 推荐最后一个元素后面也加上 `,`, 猜测这样做的好处是方便移动, 新增新的条目, 这样只用改动一行就行. 我们修改:

```python
# yield '\n' + (' ' * (_indent * _current_indent_level))
yield _item_separator + '\n' + (' ' * (_indent * _current_indent_level))
```

## Array

lua 中创建 Array 也是用的 `{}`, 而 ts 是 `[]`, 这个很好改, 将 `_iterencode_list` 中的标签改一下就行. 

Array 和 Dict 一样, 最后一个元素要加逗号, 和 Dict 的修改方法一致.

另我们的原始数据有一个 bug, List 类型的数据也会创建 Dict 来处理, 下标还是从 1 开始的. 于是我写了一个小函数来将这种类型转换为 List 格式:

```python
def dict_to_array(dct):
	array = [None] * len(dct)
	for key, value in dct.iteritems():
		if type(key) == int:
			try:
				array[key-1] = value
			except Exception as e:
				break

	for item in array:
		if not item:
			return
	return array
```

然后在渲染 Dict 的时候判断下, 如果可以转化为 List 的话就用 List 去渲染.

## 导出

之前 lua 的导出用的是 `return xxx;` 的形式, 在 ts 中修改为:

```
return "export default " + cls(...) + ";\n"
```


---

以上就是修改的全部内容, 技术含量不高, 以后也不会再写类似的文章了. 从这两次的修改中, 可以看到 Python json 库设计的非常棒, 只需要做少量修改就可以变成其他数据格式. 其中对 `yield` 用法堪称典范, 十分值得我们学习, 有机会我们应该就这点进行赏析.

[1]: /2017/09/03/serialize-lua-in-pyhton/
[2]: https://ws4.sinaimg.cn/large/006tKfTcly1fms8s40359j30fx021q32.jpg
[3]: https://gist.github.com/justbilt/a5cfafe761b8571e7b5a9e7c22cc7c57