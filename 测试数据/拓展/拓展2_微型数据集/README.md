# 微型数据集说明：某商场购物中心

## 数据来源

自行构造。模拟一个典型双层购物中心的平面拓扑。

## 为什么这样建模

购物中心是一个高度结构化的室内环境：出入口固定、人流沿固定通道流动、不同店铺有不同的开放时间。这些特征天然适合用图来建模：

- 店铺 = 顶点（含类别、营业时间）
- 人行通道/扶梯 = 边（含步行距离和时间）
- 部分通道在非营业时段关闭 = `status = "closed"`

## 数据概览

| 项目 | 值 |
|------|-----|
| 地点数 | 16 |
| 道路数 | 24（23 open + 1 closed） |
| 类别 | Entrance, Parking, Catering, Retail, Entertainment, Service, Facility |

## 关键特点与检验目标

### 1. 混合时间窗

不同店铺营业时间不同（如 Starbucks 7:00 开、Haidilao 10:00 开、Cinema 23:30 关），可用于验证 `TIMED_SHORTEST` 在不同时段路径的正确性。

### 2. 封闭道路的影响

一条 closed 道路：`M007-M005`（美食广场 → Starbucks，closed）——模拟消防通道临时封闭。检验 `COMPONENTS` 在考虑/不考虑 closed 边时的差异。

### 3. 核心节点与冗余路径

- M010（Cinema）是连接一楼和二楼的关键节点
- M007-M005 的封闭创造了一个"绕路"场景

## 文件清单

| 文件 | 说明 |
|------|------|
| `places.csv` | 16 个地点（手写构造） |
| `roads.csv` | 24 条道路（手写构造） |
| `command.txt` | 预设测试命令 |
| `README.md` | 本文档 |

## 如何运行（在 cmd 中）

```bash
REM 1. 编译（确保 campus_nav.exe 存在）
cd CampusNavigation
g++ -std=c++17 -o campus_nav.exe main.cpp LGraph.cpp LocationInfo.cpp Algorithm.cpp CsvIO.cpp CommandProcessor.cpp -I.
cd ..

REM 2. 进入测试用例目录运行（必须 cd 进入，因为 LOAD 使用相对路径）
cd 测试数据\拓展\拓展2_微型数据集
..\..\..\CampusNavigation\campus_nav.exe < command.txt
```

## 测试命令详解

| 命令 | 预期结果 |
|------|---------|
| `LOAD places.csv roads.csv` | OK |
| `COMPONENTS` | 1 个连通分量（24 条边全部 open 或 M007-M005 closed 不影响连通性） |
| `SHORTEST M001 M010 DIST` | 找到南门→卫生间→Cinema 路径（150m） |
| `SHORTEST M001 M010 TIME` | 时间最短路径（3min） |
| `TIMED_SHORTEST M001 M010 09:00 DIST` | Haidilao 10:00 才开，09:00 不可达→走其他路径 |
| `TIMED_SHORTEST M001 M010 21:00 DIST` | 大部分店铺已关门→可能 NO_PATH |
| `MST` | 生成树连通所有节点 |
| `CRITICAL` | M010 应为关键节点（连接一二楼唯一通道） |