title: quickx-tableview
tags:
---

1). 解决 lua 出错导致 player 崩溃

TableView 回调 `tableCellAtIndex` 在lua这边的实现一旦出错, 就会在 c++ 那边收到一个 NULL 的 cell, 因为没有判空, 下面对 cell 的操作就会导致 Plaer 崩溃. 对应修改如下:

```c++
     cell = _dataSource->tableCellAtIndex(this, idx);
-    this->_setIndexForCell(idx, cell);
-    this->_addCellIfNecessary(cell);
+    if(cell)
+    {
+        this->_setIndexForCell(idx, cell);
+        this->_addCellIfNecessary(cell);
+    }
```

这时候, 不会崩溃了, 但是也看不到 lua 那边出的什么错误, 经过一番追踪, 定位到了 `LuaStack::executeFunction` 函数:

```c++
        if (error)
        {
            if (traceCallback == 0)
            {
                CCLOG("[LUA ERROR] %s", lua_tostring(_state, - 1));        /* L: ... error */
                lua_pop(_state, 1);                                        // remove error message from stack
            }
            else                                                           /* L: ... G error */
            {
+               CCLOG("[LUA ERROR] %s", lua_tostring(_state, - 2));        /* L: ... error */
                lua_pop(_state, 2);                                        // remove __G__TRACKBACK__ and error message from stack
            }
            return 0;
        }
```

虽然出错了, 但是 `traceCallback` 的值并不等于 0, 所以没有进入输出错误的逻辑, 具体用意并不明白. 我的添加了带 `+` 号的哪一行, 输出了下就可以看到 lua 的错误了.

2). 




---

# 渲染批次和顶点数量

正如刚才提到的, 我们的一个 主基地+UI 的渲染批次就到了 250, 顶点数量更是惊人的到了 4000+ , 因此在低端机上, 无理论做什么都卡卡的.

| 节点类型 | 顶点数量 | 渲染批次 | 批量渲染 |
| --------   | -----:  | :----:  | :----: |
| cc.Node | 0 | 0 |
| cc.Layer | 0 | 0 |
| cc.LayerColor | 4 | 1 | × |
| cc.Sprite | 6 | 1 | √ |
| cc.Scale9Sprite | 6 | 1 | √ |
| cc.TTFLabel | 6 | 1 | × |
| cc.BMFontLabel | 6*n | 1 | × |
| cc.ParticleSystemQuad | 6*n | 1 | √ |

## 顶点数量

经过大致的测试, 我们发现导致顶点数据飙升的原因竟是 `Scale9Sprite`, 

