# 拓展 1：共享单车（分层图最短路径）— 使用说明

## 测试数据

拓展 1 的测试数据已包含在 `测试数据/拓展/` 下：

| 目录 | 规模 | 说明 |
|------|------|------|
| `small/case_layered_01/` | 5 节点 | 覆盖 K=0/1/2/3 多种券数 |
| `medium/grid_100/` | 100 节点网格 | K=0/3/8 |
| `medium/random_sparse_100/` | 100 节点随机稀疏 | K=0/3/5/10 |
| `large/random_sparse_1000/` | 1000 节点 | 大规模压力测试 |

## 如何运行

```bash
REM 1. 编译（在项目根目录下执行）
cd CampusNavigation
g++ -std=c++17 -o campus_nav.exe main.cpp LGraph.cpp LocationInfo.cpp Algorithm.cpp CsvIO.cpp CommandProcessor.cpp -I.
cd ..

REM 2. 运行并保存输出（在测试用例目录下执行）
REM    注意：small 用例深 4 层，medium/large 用例深 3 层
cd 测试数据\拓展\small\case_layered_01
..\..\..\..\CampusNavigation\campus_nav.exe < command.txt > my_answer.txt

REM 3. 和标准答案比对
fc /w my_answer.txt answer.txt
```

## 命令格式

```
SHORTEST_K <from_id> <to_id> <K>
```

输出格式：
```
PATH <total_time> K_USED <k_used> NODES <id1> <id2> ... FAST <count> [<u1>-<v1> ...]
```

- K 为最大可用券数（0 ≤ K ≤ 10）
- 每张券将一条边的 walk_time 缩短为 ceil(walk_time / 3)

## 验证方法

```bash
REM 每个测试用例目录都有 answer.txt
cd 测试数据\拓展\small\case_layered_01
..\..\..\..\CampusNavigation\campus_nav.exe < command.txt > my_answer.txt
fc /w my_answer.txt answer.txt
```

显示 "FC: 找不到差异" 即表示通过。

## 算法复杂度

- 时间复杂度：O(K·(V+E) log(K·V))
- 空间复杂度：O(K·V)
- K ≤ 10 时与普通 Dijkstra 同阶