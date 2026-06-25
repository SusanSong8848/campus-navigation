# make_my_test.py — 测试数据生成器使用指南

## 一句话概述

`make_my_test.py` 是一个命令行驱动的测试数据生成器。可以快速生成多种拓扑类型的测试数据（`places.csv` + `roads.csv` + `command.txt`），用于验证校园导航程序的正确性。

## 快速开始

```bash
# 在 tools/ 目录下运行
cd tools
python make_my_test.py example basic     # 预设的微型校园（5 节点，数据固定）
python make_my_test.py grid 5 5          # 5×5 网格（25 节点）
python make_my_test.py chain 20          # 20 节点链
python make_my_test.py cycle 10          # 10 节点环
python make_my_test.py star 8            # 星形图（1 中心 + 8 叶子）
python make_my_test.py random 50 80      # 50 节点 80 边随机连通图
```

所有生成的数据保存在 `tools/my_test/` 目录下，每次运行会**覆盖**上一次的结果。

## 完整流程：生成 → 运行 → 比对

```bash
# 1) 生成测试数据（加上 seed 让每次结果固定）
cd tools
python make_my_test.py grid 3 3 42       # seed=42，3x3 网格

# 2) 把生成的数据复制到 CampusNavigation
copy my_test\places.csv  ..\CampusNavigation\places.csv  /y
copy my_test\roads.csv   ..\CampusNavigation\roads.csv   /y
copy my_test\command.txt ..\CampusNavigation\command.txt /y

# 3) 编译（如果还没编译过）
cd ..\CampusNavigation
g++ -std=c++17 -o campus_nav.exe main.cpp LGraph.cpp LocationInfo.cpp Algorithm.cpp CsvIO.cpp CommandProcessor.cpp -I.

# 4) 回到项目根目录运行（注意路径！）
cd ..
CampusNavigation\campus_nav.exe < tools\my_test\command.txt > result1.txt

# 5) 手工验算：对照网格拓扑手算预期结果，和 result1.txt 对比
```

## 比对方法（无 answer.txt 时如何验证）

随机生成的数据没有预制的 `answer.txt`，比对策略如下：

### 方法一：用 seed 固定数据 + 人工验算（推荐）

```bash
python make_my_test.py grid 3 3 42    # 每次跑结果完全一样
```

在纸上画出 3x3 网格，手算验证 SHORTEST 和 MST 结果。

### 方法二：验证语义正确性（不看字符细节）

| 验证点 | 检查方法 |
|--------|---------|
| 图连通 | `COMPONENTS` 输出 `1 SIZES <V>` |
| MST 边数 | MST 边数 == 节点数 - 1 |
| SHORTEST 可达 | `PATH` 输出以起点开始、终点结束 |
| 路径无重复 | NODES 列表中没有重复节点 |
| 链的关键节点 | 链图中，除两端外所有节点都是关键节点 |
| 环的关键节点 | 环图中，关键节点数为 0 |

### 方法三：两次运行一致性检查

```bash
# 在项目根目录下运行
python tools\make_my_test.py random 20 30 42
CampusNavigation\campus_nav.exe < tools\my_test\command.txt > result1.txt

python tools\make_my_test.py random 20 30 42
CampusNavigation\campus_nav.exe < tools\my_test\command.txt > result2.txt

fc result1.txt result2.txt               # 应该完全相同（逐字一致）       FC: 找不到差异
```

## 支持的拓扑类型

| 类型 | 命令 | 说明 |
|------|------|------|
| grid | `grid <rows> <cols>` | rows x cols 网格图 |
| chain | `chain <n>` | n 个节点的链（每个节点只连前后邻居） |
| cycle | `cycle <n>` | n 个节点的环（链 + 首尾相连） |
| star | `star <leaves>` | 1 个中心 + leaves 个叶子的星形图 |
| random | `random <n> <m>` | n 节点 m 边的随机连通图 |
| example | `example <name>` | 预设示例（basic / timed / modify） |

## Seed（随机种子）

不带 seed 时每次跑生成的数据都不同（距离、类别、停留时间随机）。带 seed 时每次跑完全一致：

```bash
python make_my_test.py grid 5 5 42    # seed=42，每次跑结果一样
python make_my_test.py random 50 80 7 # seed=7
```

## 预设示例（example）

| 示例 | 特点 |
|------|------|
| `basic` | 5 个地点的微型校园（宿舍/教学楼/食堂/图书馆/运动场），数据完全固定，可手算验证 |
| `timed` | 4 个节点的时刻约束测试（宿舍/食堂/夜宵店/教学楼），含三个不同时刻的 TIMED_SHORTEST |
| `modify` | 3 个节点的动态增删改测试（环 + 增加节点 + 增加道路 + 关闭道路） |

## 随机化属性

当 `TestBuilder(randomize=True)` 时，以下属性会自动随机填充：

| 属性 | 随机范围 |
|------|----------|
| category | Teaching / Dining / Dormitory / Sports / Medical / Other |
| stay_time | 0 ~ 60 分钟 |
| open_time / close_time | 7 种时间窗（含全天/白天/夜间） |
| distance | 50 ~ 300 |

## 输出格式

生成的三个文件均严格符合本项目的 CSV schema：

- `places.csv`：`place_id,display_name,category,stay_time,open_time,close_time`（带表头）
- `roads.csv`：`from_id,to_id,distance,walk_time,status`（带表头）
- `command.txt`：预设的一组测试命令序列（含 LOAD / COMPONENTS / SHORTEST / MST / CRITICAL / QUIT）

## 生成的数据位置

所有生成的数据在当前目录的 `my_test/` 子目录下。每次运行会**覆盖**上一次的结果，如需保留请手动改名目录。