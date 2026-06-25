# 对抗样例说明：全等权多环路图

## 为什么这组数据特殊

这组数据有 12 个节点、18 条边，**所有边的 distance 完全相同（都是 10），walk_time 完全相同（都是 1）**。

图中每个节点属于至少两个环——如果 BFS 实现不好，环路会导致无限循环或关键节点枚举超时。

## 它能卡住哪类错误实现

### 1. Dijkstra 未处理等权边多路径

如果 Dijkstra 在等权边上依赖邻接表遍历顺序，不同实现的路径会不同。这组数据检验 `SHORTEST A01 A12 DIST` 是否正确（所有路径总距离相同 = 40，但走法不同）。

### 2. 关键节点/边枚举的超时风险

图中有大量环路。如果 BFS 没有使用 `visited` 集合，会因环路陷入无限循环。同时等权边让 `AllEdges` 的高频调用暴露性能问题。

### 3. MST 等权边的排序稳定性

所有权相等时 Kruskal 的选择顺序可能因排序稳定性而不同。检验程序是否仍能生成 V-1 条边。

## 我的程序如何正确处理

| 对抗点 | 处理方式 |
|--------|---------|
| 等权边多路径 | Dijkstra 使用优先队列按最短距离松弛，与邻接边遍历顺序无关 |
| 环路防循环 | BFS 使用 `visited` 哈希集标记已访问节点，每个节点只入队一次 |
| 等权边 MST | Kruskal 对所有边按 distance 排序，同权边按字典序稳定排，MST 边数 = V-1 |
| 关键节点/边 | BFS 屏蔽实现不修改原图，惰性跳过不增加复杂度 |

## 文件清单

| 文件 | 说明 |
|------|------|
| `adversarial_places.csv` | 12 个节点（手写构造） |
| `adversarial_roads.csv` | 18 条边（全部 distance=10, walk_time=1） |
| `command.txt` | 测试命令 |
| `README.md` | 本文档 |

## 如何运行（在 cmd 中）

```bash
REM 1. 编译（确保 campus_nav.exe 存在）
cd CampusNavigation
g++ -std=c++17 -o campus_nav.exe main.cpp LGraph.cpp LocationInfo.cpp Algorithm.cpp CsvIO.cpp CommandProcessor.cpp -I.
cd ..

REM 2. 进入测试用例目录运行（必须 cd 进入，因为 LOAD 使用相对路径）
cd 测试数据\拓展\拓展2_对抗样例
..\..\..\CampusNavigation\campus_nav.exe < command.txt
```

## 测试命令详解

| 命令 | 预期结果 |
|------|---------|
| `LOAD adversarial_places.csv adversarial_roads.csv` | OK |
| `COMPONENTS` | 1 components（全连通图） |
| `SHORTEST A01 A12 DIST` | 总距离 40（4×10），路径不包含重复节点 |
| `SHORTEST A01 A12 TIME` | 总时间 4（4×1） |
| `MUST_PASS A01 A12 DIST 1 A06` | 经 A06 到达 A12，总距离 50（5×10） |
| `MST` | 11 条边，总距离 110 |
| `CRITICAL` | 0 关键节点（全环路无割点） |