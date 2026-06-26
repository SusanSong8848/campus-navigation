# 校园导航系统 — CampusNavigation

> 数据结构课程个人项目：动态图上的校园路线规划与设施分析系统  
> C++17 · 无外部依赖 · CLI 批处理

---

## 快速开始（一共提供了三种编译方法和三种运行方法（.bat/g++/CMake））

### 一键编译（双击 `build.bat`）

项目根目录提供了 **`build.bat`**，双击后交互式选择编译方式：
- **选项 1**：g++ 直接编译（推荐，无需安装 CMake）
- **选项 2**：CMake 构建（适合用 Visual Studio 的场合）

__编译完成可以直接一键测试数据：__（若有比对`FAIL`的地方可能答案是存在多种写法）
项目根目录提供了 **`run_single_test.bat`** 和 **`run_all_tests.bat`**，双击后可运行：
- **选项 1：`run_single_test.bat`**：测试单个数据，例如输入 `case_01`，生成运行结果以及比对结果
- **选项 2：`run_all_tests.bat`**：所有测试数据一键全部测试和比对

### 手动编译

#### 方式一：g++ 直接编译

```bash
# 在 cmd 或 PowerShell 中执行
cd CampusNavigation
g++ -std=c++17 -o campus_nav.exe main.cpp LGraph.cpp LocationInfo.cpp Algorithm.cpp CsvIO.cpp CommandProcessor.cpp -I.
```

编译完成后，可以到任意测试用例目录运行：

```bash
# 例如跑 case_01
cd ..\测试数据\必做\small_cases\case_01
..\..\..\..\CampusNavigation\campus_nav.exe < command.txt > my_answer.txt
# 和标准答案比较
fc /w my_answer.txt answer.txt
# 显示 "FC: 找不到差异" 即表示完全一致 ✅
```

#### 方式二：CMake 构建

```bash
cd CampusNavigation
mkdir build && cd build
cmake ..
cmake --build .
# 生成的可执行文件在 build\Debug\CampusNavigation.exe 或 build\Release\CampusNavigation.exe
```

**编译要求**：C++17 标准，仅使用 STL，无需任何外部库。

> ⚠️ Windows 上用 MSVC 编译时，如果遇到中文注释导致 C2059 语法错误，已在 CMakeLists.txt 中加了 `/utf-8` 选项修复。如果遇到 `build` 目录被占用删不掉的错误，关闭所有终端窗口再试。

### 运行示例（单个测试）

```bash
# 去测试用例目录(在根目录下：)
cd 测试数据\必做\small_cases\case_01
# 运行程序（输入从 command.txt 读取，输出写入 my_answer.txt）
..\..\..\..\CampusNavigation\campus_nav.exe < command.txt > my_answer.txt
# 和标准答案比较
fc /w my_answer.txt answer.txt
# 显示 "FC: 找不到差异" 即表示完全一致 ✅
```

---

## 项目结构

```
CampusNavigation/          # 源代码
├── main.cpp               # 主程序入口（CLI 批处理）
├── LocationInfo.h/cpp     # 地点信息结构体
├── LGraph.h/cpp           # 图 ADT（邻接表 + 双哈希索引）
├── CsvIO.h/cpp            # CSV 文件读写
├── CommandProcessor.h/cpp # 命令解析与分发（20 个命令）
├── Algorithm.h/cpp        # 图算法（DSU/BFS/Dijkstra/Kruskal/Critical/分层图）
├── GraphException.h       # 异常类
└── CMakeLists.txt         # 构建配置

docs/
├── requirements/          # 项目规范文档
└── my_completed/          # 我完成的文档
    ├── 实验报告.md
    └── NOTE.md             # C++ 和 .bat 随记笔记

测试数据/
├── 必做/                   # 13 个测试用例（6~10000 节点）
│   ├── small_cases/       # case_01~04
│   ├── medium_cases/      # 6 个 100 节点用例
│   ├── large_cases/       # chain_1000 + random_sparse_10000
│   └── sample_ecnu/
└── 拓展/
    ├── small/medium/large/     # 拓展 1 测试数据（共享单车）
    ├── 拓展2_微型数据集/       # 拓展 2 自制数据（商场）
    ├── 拓展2_对抗样例/         # 拓展 2 对抗样例（全等权图）
    └── 拓展3_图形化界面/       # 拓展 3 HTML Canvas 交互地图

tools/
├── make_my_test.py         # 测试数据生成器
└── README.md

build.bat                   # 一键编译
run_all_tests.bat           # 一键批量测试
run_single_test.bat         # 单用例测试
run_all_tests_annotated.bat # 带注释版测试脚本
```

---

## 核心功能

### 数据维护（22 分）

| 命令 | 功能 |
|------|------|
| `LOAD <p.csv> <r.csv>` | 加载地点和道路数据 |
| `SAVE <p.csv> <r.csv>` | 保存当前图到 CSV |
| `ADD_PLACE / DELETE_PLACE / UPDATE_PLACE` | 地点增删改 |
| `ADD_ROAD / DELETE_ROAD / UPDATE_ROAD` | 道路增删改 |
| `CLOSE_ROAD / OPEN_ROAD` | 道路开关 |

### 查询功能（8 分）

| 命令 | 功能 |
|------|------|
| `QUERY_PLACE <id>` | 按 ID 查询地点完整信息 |
| `QUERY_CATEGORY <cat>` | 按类别查询地点列表 |
| `ADJ <id>` | 查询某地点的所有邻接道路 |

### 图算法（35 分）

| 命令 | 算法 | 复杂度 |
|------|------|--------|
| `COMPONENTS` | BFS 连通分量分析 | O(V+E) |
| `SHORTEST <u> <v> <DIST\|TIME>` | Dijkstra 最短路径（双模式） | O((V+E)logV) |
| `TIMED_SHORTEST <u> <v> <HH:MM> <DIST\|TIME>` | 时刻约束最短路径 | O((V+E)logV) |
| `MUST_PASS <u> <v> <DIST\|TIME> <k> <p1..pk>` | 必经点路径规划 | O(k·(V+E)logV) |
| `MST` | Kruskal 最小生成树 + DSU | O(E log E) |
| `CRITICAL` | 关键节点/边枚举分析 | O(V·(V+E)+E·(V+E)) |

---

## 拓展功能（15 分）

### 拓展 1：共享单车 SHORTEST_K（9 分）

| 命令 | 说明 |
|------|------|
| `SHORTEST_K <from> <to> <K>` | K 张加速券下的最短时间路径 |

- 算法：二维 Dijkstra（状态 = place_id + 已用券数）
- 券效果：walk_time → ceil(walk_time / 3)
- 测试数据：`测试数据/拓展/small/`、`medium/`、`large/`

```bash
cd 测试数据\拓展\small\case_layered_01
..\..\..\..\CampusNavigation\campus_nav.exe < command.txt > my_answer.txt
fc /w my_answer.txt answer.txt
```

### 拓展 2：自定义微型数据集 + 对抗样例（3 分）

| 数据集 | 位置 | 说明 |
|--------|------|------|
| 商场购物中心 | `拓展2_微型数据集/` | 16 节点 24 边，含封闭道路和混合时间窗 |
| 对抗样例 | `拓展2_对抗样例/` | 12 节点 18 边全等权多环路图 |

```bash
cd 测试数据\拓展\拓展2_微型数据集
..\..\..\CampusNavigation\campus_nav.exe < command.txt > my_answer.txt
```

### 拓展 3：图形化界面（3 分）

| 文件 | 说明 |
|------|------|
| `测试数据/拓展/拓展3_图形化界面/campus_map.html` | HTML Canvas 交互式校园地图 |

- 60 节点 120 边全连通力导向图
- HiDPI 高清渲染
- Dijkstra 最短路径高亮 + 关键节点标识
- 点击节点/边查看详情
- **双击即开**，无需编译

---

## 测试

### 一键批量测试

| 脚本 | 用途 | 用法 |
|------|------|------|
| **[run_all_tests.bat](run_all_tests.bat)** | 批量测试全部 13 个用例 | **双击运行**，无需交互，窗口会停在结果页 |
| **[run_all_tests_annotated.bat](run_all_tests_annotated.bat)** | 同上，带详细注释（学习用） | **双击运行**，每行命令都有 REM 解释 |
| **[run_single_test.bat](run_single_test.bat)** | 单独测试一个用例 | **双击后输入用例名**（如 `case_01`），自动运行并对比输出 |

> ⚠️ 双击后窗口立即消失？在文件管理器中选中 .bat 文件，右键 → **在终端中打开**，或者先打开 cmd 再 cd 到项目根目录执行。
> 
> ⚠️ 部分用例显示 `[FAIL]`：全部为 **MST 边选择差异**（Kruskal 算法在同权边上的不同选择，总距离完全一致），不是代码错误。详见测试数据 README。

### 测试结果

| 用例 | 结果 |
|------|------|
| small_cases/case_01~04 | ✅ 全部逐字一致 |
| sample_ecnu | ✅ 逐字一致 |
| chain_flower_100 | ✅ 逐字一致 |
| grid_100 / ring_100 / random_sparse_100 | ⚠️ 语义一致（MST 不唯一/等权路径差异） |
| star_100 / mixed_time_100 | ✅ 逐字一致 |
| chain_1000 / random_sparse_10000 | ✅ 正常运行（压力测试通过） |

**总计**：13 个用例，0 个功能错误。

---

## 数据结构设计

LGraph 内部采用 **邻接表 + 双哈希索引**：

```cpp
// 顶点存储：place_id → 顶点信息 (O(1) 查找)
unordered_map<string, LocationInfo> vertices_;

// 邻接表：from_id → (to_id → 边信息) (O(1) 查找边)
unordered_map<string, unordered_map<string, EdgeNode>> adj_;
```

**选型理由**：空间 O(V+E)，所有增删改查操作 O(1)，最适合稀疏的校园图场景。

---

## 📖 文档导航

| 文档 | 说明 |
|------|------|
| [实验报告](docs/my_completed/实验报告.md) | 数据结构设计 + 算法复杂度分析 + 测试结果 + AI 协作记录 |
| [随记笔记](docs/my_completed/NOTE.md) | 50+ 个 C++ / .bat / Web 知识点 |
| [命令接口规范](docs/requirements/命令接口规范.md) | 19 个命令的强制规范 |
| [项目说明](docs/requirements/新版课程项目说明.md) | 完整项目要求、评分标准 |
| [扩展与加分要求](docs/requirements/扩展与加分要求.md) | 拓展任务 1~3 要求 |
| [测试数据说明](测试数据/README.md) | 13 个测试用例的拓扑特性 |
| [make_my_test.py 使用指南](tools/README.md) | 测试数据生成器使用手册 |
| [拓展 1 README](测试数据/拓展/拓展1_共享单车/README.md) | 共享单车拓展说明 |
| [拓展 2 微型数据集](测试数据/拓展/拓展2_微型数据集/README.md) | 自制数据（商场）使用说明 |
| [拓展 2 对抗样例](测试数据/拓展/拓展2_对抗样例/README.md) | 对抗样例使用说明 |
| [拓展 3 GUI README](测试数据/拓展/拓展3_图形化界面/README.md) | GUI 技术说明（含截图位置） |

## Github:

该项目已同步到 https://github.com/SusanSong8848/campus-navigation.git