# 校园导航系统 — 本人的随记笔记：能学到新东西

## 一、LGraph

### （一）[LGraph.h里面](CampusNavigation/LGraph.h)

#### 1. namespace Graph 是 C++ 的命名空间，类似于给代码加一个"姓"。

- 避免命名冲突：如果别人的库也有一个叫 LGraph 的类，我的就叫 __Graph::LGraph__，不会打架。  
- 组织代码：所有跟图结构相关的类（LGraph, EdgeNode, LocationInfo 等）都放在同一个命名空间里，结构清晰。  
- 便于维护：一看就知道是图模块的东西。  

#### 2. `explicit` 关键字——防止隐式类型转换

```cpp
explicit LGraph(bool directed = false);
```

如果不加 `explicit`，可能出现这种奇怪的代码：
```cpp
LGraph g = true;  // 居然把 bool 悄悄变成了 LGraph(true)！
```

对于**单参数构造函数**，推荐加上 `explicit` 以避免意外的隐式类型转换。

#### 3. `std::move()`——移动语义，不拷贝数据

```cpp
EdgeNode(std::string f, std::string t, int d, int w, std::string s)
    : from_id(std::move(f)), to_id(std::move(t)),
      distance(d), walk_time(w), status(std::move(s)) {}
```

- `std::move()` **不拷贝数据**，而是把资源的所有权从一个对象**转移**到另一个对象
- 尤其对于长字符串（如 `display_name`），`from_id` 直接接管 `f` 的内部字符数组，不再额外拷贝
- **口诀**：能移动就移动，别拷贝。省时省内存

#### 4. 我 LGraph 中的双哈希索引设计原因

- __双哈希索引__ 指 LGraph 内部同时用了两个 unordered_map（vertices_, adj_）
[LGraph.h里面](CampusNavigation/LGraph.h)
```
// 选择理由：
//   1. vertices_: unordered_map<place_id, LocationInfo>
//      - 通过 place_id 查找顶点信息 O(1)
//      - 存储顶点的完整属性（名称、类别、时间窗等）
//
//   2. adj_: unordered_map<from_id, unordered_map<to_id, EdgeNode>>
//      - 邻接表：从任一顶点出发，O(1) 定位其邻接边集合
//      - 内层 map 允许 O(1) 查找特定边的信息
//      - 无向图中每条边存储两次（两个方向），保证双向查询对称
```

- unordered_map：C++ 标准库的哈希表容器，这样方便直接O(1)查询，既不会开很大的内存(如矩阵adj)，又可以通过顶点的id直接查找边或顶点信息（`vector<EdgeNode>`这种还要遍历找边，太慢；而`unordered_map<from_id, unordered_map<to_id, EdgeNode>>`先找`from_id`，再找`to_id`就能得到这条边`EdgeNode`的信息了）
- hash邻接表：空间 O(V+E)，增删改查均 O(1)，最适合本项目

### （二）[LGraph.cpp里面](CampusNavigation/LGraph.cpp)

#### 1. unordered_map 和 map 的区别？为什么不用 map？

![unordered_map 和 map 的区别](<imgs/unordered_map 和 map 的区别.png>)

#### 2. `static_cast<int>()`——C++ 显式类型转换

```cpp
return static_cast<int>(vertices_.size());
```

- `size()` 返回 `size_t`（无符号整型），函数返回类型是 `int`
- 用 `static_cast<int>` 转换，避免编译器警告类型不匹配
- 对于小规模数据是安全的
- 比 C 风格的 `(int)` 更安全，编译期能检查更多错误

#### 3. `unordered_map::find()` 的返回值约定

```cpp
auto it = vertices_.find(place_id);
if (it == vertices_.end()) {
    throw GraphException("place_not_found");
}
```

- `find()` 找到了 → 返回指向该元素的**迭代器**（类似指针）
- `find()` 没找到 → 返回 `end()`（特殊的"哨兵"迭代器）
- 所以判断存在用 `!= end()`，判断不存在用 `== end()`

#### 4. `std::stoi()`——字符串转整数

```cpp
info.stay_time = std::stoi(value);  // string to int
```

- **s**tring **to** **i**nt 的缩写
- 将 `"45"` 这种字符串直接转成 `int 45`
- 类似函数族：`stod`（→double）、`stol`（→long）、`stof`（→float）

#### 5. `vector.reserve()` + `push_back` vs `vector.resize(n)`

```cpp
std::vector<EdgeNode> result;
result.reserve(adj_it->second.size());     // 只分配内存，不创建元素
for (...) {
    result.push_back(neighbor_pair.second); // 逐个填入
}
```

- `reserve(n)`：**只预留容量**（分配内存），不创建元素（`size` 不变）
- `resize(n)`：会先默认构造 n 个无用的占位对象，然后再用赋值覆盖，多了一步浪费
- **最佳实践**：已知元素数量的情况下，用 `reserve + push_back`，语义更清晰：我只是在收集元素，不是创建一堆占位再替换

#### 6. `GetEdge` 中不能复用 `exist_edge` 的原因

```cpp
// 这里最好不要复用 exist_edge(from_id, to_id)
// 因为如果复用，相当于 exist_edge 查找一次（找 from_id → to_id）
// 然后本函数又要查找一次（也要找 from_id → to_id），时间翻倍
auto from_it = adj_.find(from_id);
auto to_it = from_it->second.find(to_id);
```

- 这是一个**性能微优化**（我本来是想复用，但ai没有）：如果复用 `exist_edge`，需要两次哈希查找
- 直接自己查找，拿到了迭代器就直接访问，一次查找完成
- 类似"你帮我去房间看看有没有那本书，有的话拿过来"vs"你去看有没有，回来告诉我，我再去拿"

---

## 二、CsvIO

### （一）[CsvIO.h里面](CampusNavigation/CsvIO.h)

#### 1. CSV 是什么

```
//（说明）：CSV = Comma-Separated Values（逗号分隔值）。
//.csv 是一种纯文本文件格式：每行一条记录，字段之间用逗号分隔，类似简易表格，几乎所有数据处理软件都能读写。
```

#### 2. `(void)path`——显式标记未使用参数

```cpp
// 助教模板里给的"(void)path; 和 (void)graph..."：
// 这是显式标记"这个参数我暂时不用"，避免编译器警告。
```

- 当函数参数暂时没用到（如 TODO 阶段），写 `(void)参数名;` 
- 编译器不会报 "unused parameter" 警告
- 比直接删掉参数名更好：保留了接口签名，以后实现时知道有这个参数

### （二）[CsvIO.cpp里面](CampusNavigation/CsvIO.cpp)

#### 1. `std::istringstream`——把字符串当"内存中的文件"读

```cpp
std::istringstream iss(parsed);
// istringstream 是 C++ 标准库里的字符串输入流类。
// 通俗理解：把一坨字符串当成"内存中的文件"，然后像从键盘或文件读取一样逐个提取
iss >> place_id >> display_name >> category >> stay_time >> open_time >> close_time;
```

- 和 `cin >> x` 一模一样，只是数据源从键盘换成了内存中的字符串
- 用 `>>` 自动跳过空白、按空格分隔，非常适合解析 CSV

#### 2. `std::ifstream`——文件输入流的本质

```cpp
std::ifstream file(path);
while (std::getline(file, line)) { ... }
```

- `file` 本质是操作系统层面的一个**文件句柄**（读指针 + 缓冲区）
- `std::ifstream` 的构造函数内部调用了操作系统的 `open()` 系统调用，把磁盘文件关联到这个对象上
- `std::getline(file, line)` 每次从文件中读取一行到 `line`

#### 3. `std::string::npos`——"没找到"的特殊常量

```cpp
size_t start = s.find_first_not_of(" \t\r\n");
if (start == std::string::npos) return "";
```

- `std::string::npos` 是一个特殊常量，意思是**"没找到"**
- 如果整行全是空格/制表符/回车，就找不到任何非空白字符，返回 npos
- 等价于 `size_t` 的最大值（通常为 `18446744073709551615`）

#### 4. `find_first_not_of()` / `find_last_not_of()`——查找"不在集合中"的字符

```cpp
size_t start = s.find_first_not_of(" \t\r\n");   // 找到第一个不是空白符的地方
size_t end   = s.find_last_not_of(" \t\r\n");    // 找到最后一个不是空白符的地方
```

- `find_first_not_of(" \t\r\n")`：从前往后找，第一个**不在** `" \t\r\n"` 中的字符的位置
- `find_last_not_of(" \t\r\n")`：从后往前找，同理
- 两者配合 `substr(start, end-start+1)` 就能截掉首尾空白

#### 5. `emplace_back` vs `push_back`——原地构造 vs 拷贝

```cpp
places.emplace_back(place_id, display_name, category,
                   stay_time, open_time, close_time);
```

- `push_back`：先在外面构造好一个临时对象，再**拷贝/移动**进 vector
- `emplace_back`：直接在 vector 内部**原地构造**对象，省去了一次拷贝/移动
- 传递构造函数参数时，几乎无脑选 `emplace_back`

#### 6. `std::cerr`——标准错误输出流

```cpp
std::cerr << "CsvIO::ReadPlaces: 无法打开文件 " << path << std::endl;
```

- `cerr` 和 `cout` 类似，但走的是**标准错误流**（stderr）而非标准输出流（stdout）
- 好处：即使用户把正常输出重定向到文件（`./program > output.txt`），错误信息仍会显示在屏幕上
- 写明 `CsvIO::ReadPlaces` 前缀，出问题时能第一时间定位到是哪个模块

#### 7. `std::ofstream`——析构时自动关闭文件

```cpp
// 4. 关闭文件（ofstream 析构时自动关闭，那我其实可以不写）
file.close();
```

- `ofstream` 对象在离开作用域时，析构函数会自动调用 `close()`
- 手动 `close()` 不是必须的，但明确写出能让读者知道"这里文件读写结束了"

---

## 三、Algorithm（图算法）

### （一）[Algorithm.h里面](CampusNavigation/Algorithm.h)

#### 1. DSU（并查集）——路径压缩 + 按秩合并

DSU 是 **Disjoint Set Union**（不交集合并）的缩写，用于 Kruskal 最小生成树算法。

**核心操作：**
- `find(x)`：找 x 的根节点 + **路径压缩**——把沿路所有节点直接挂到根上
- `unite(x, y)`：合并两个集合 + **按秩合并**——始终把矮树挂到高树下
- 两个优化合在一起，摊还复杂度 O(α(n)) ≈ O(1)

**rank（秩）**：并不是树的精确高度，而是"近似高度"。只有两棵树秩相等时合并，新树的秩才会 +1。这样保证树不会退化成长链。

#### 2. `enum`（枚举类型）——给整数常量取名字

```cpp
enum PathMode {
    DIST,   // 按距离最短  → 值为 0
    TIME    // 按步行时间最短 → 值为 1
};
```

- `enum` 把一组相关的整数常量打包成一个"类型"
- 代码中用 `PathMode::DIST` / `PathMode::TIME` 代替裸数字 0/1，可读性大幅提高
- 默认从 0 开始递增，也可以手动指定：`enum { A = 5, B = 10 }`

#### 3. `static` 函数只在当前 .cpp 文件内可见

```cpp
// Algorithm.cpp 里的两个 static 辅助函数
static int getWeight(const EdgeNode &edge, PathMode mode);
static std::vector<int> bfsComponents(...);
```

- 写在 `.cpp` 文件里的 `static` 函数，**只在当前编译单元（.cpp）内可见**
- 外部文件（如 `main.cpp`、`CommandProcessor.cpp`）完全看不到它们
- 作用：把内部实现细节藏起来，避免污染全局命名空间
- 和 `private` 成员函数类似，但这是文件级的"私有"

### （二）[Algorithm.cpp里面](CampusNavigation/Algorithm.cpp)

#### 1. `<climits>`——整数极限值

```cpp
#include <climits>  // INT_MAX
dist[id] = INT_MAX; // 初始距离设为"无穷大"
```

- `INT_MAX` 是 C++ 中 `int` 类型的最大值（通常为 2147483647）
- 用于 Dijkstra 初始化：把所有点距离设为"无穷大"，表示尚未到达
- 松弛条件 `cur_dist + weight < dist[neighbor]` 依赖 INT_MAX 来确保首次到达的任何路径都能更新

#### 2. Lambda 表达式（匿名函数）

```cpp
auto isEdgeBlocked = [&](const std::string &a, const std::string &b) {
    std::string key = (a < b) ? (a + "|" + b) : (b + "|" + a);
    return blocked_edges.count(key) > 0;
};
```

**Lambda 语法拆解：**
- `[&]`：**捕获列表**，`&` 表示以引用方式捕获外部作用域的所有局部变量（如 `blocked_edges`），无需传参也能访问
- `(const std::string &a, const std::string &b)`：参数列表，和普通函数一样
- `{ ... }`：函数体
- `auto` 不能写成 `bool`，因为 lambda 返回的是一个**可调用对象**，不是一个 `bool` 值

**捕获方式的变体：**
- `[&]`：引用捕获全部（效率高，不拷贝，但要注意生命周期——被捕获的变量不能先于 lambda 销毁）
- `[=]`：值捕获全部（安全，但会拷贝）
- `[&x, =y]`：x 引用捕获，y 值捕获

#### 3. `std::priority_queue` 实现小顶堆

```cpp
using P = std::pair<int, std::string>;
std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
```

- `priority_queue` **默认是大顶堆**（最大的在堆顶，等价于 `std::less<P>`）
- **第三个模板参数**是比较器，传入 `std::greater<P>` 后比较规则反转，变成**小顶堆**（最小的在堆顶）
- `std::greater<P>` 是一个函数对象，比较 `pair` 时先比 `first`（即距离/时间），再比 `second`
- 这样 `pq.top()` 始终返回当前 `first` 最小的那个 pair，恰好满足 Dijkstra 需要

**三个模板参数的含义：**

| 参数 | 含义 |
|------|------|
| `P` | 存储的元素类型 |
| `std::vector<P>` | 底层容器（默认就是 vector） |
| `std::greater<P>` | 比较规则（从小到大出队） |

#### 4. `using` 类型别名——现代版 `typedef`

```cpp
using P = std::pair<int, std::string>;
```

- `using P = ...` 给一个长类型起短名，方便后续多次使用
- 等价于 `typedef std::pair<int, std::string> P`
- **更推荐 `using`**：语法更直观（`新名字 = 旧类型`），且支持模板别名

#### 5. C++17 结构化绑定（Structured Binding）

```cpp
auto [cur_dist, cur_id] = pq.top();  // 把 pair 的两个元素解包到两个变量
```

- 取出 `pq.top()` 返回的 `pair<int, string>`，把 `first` 赋给 `cur_dist`，`second` 赋给 `cur_id`
- **需要 C++17 标准**（CMakeLists.txt 里设的 `CMAKE_CXX_STANDARD 17`）
- 比 `auto p = pq.top(); p.first` / `p.second` 写法更清晰

#### 6. `std::greater<int>()`——降序排序

```cpp
std::sort(sizes.begin(), sizes.end(), std::greater<int>());  // 降序
std::sort(ids.begin(), ids.end());                            // 默认升序
```

- `std::sort` 默认是**升序**（从小到大），第三个参数不写就是 `std::less<>`
- 传入 `std::greater<int>()` 则反转为**降序**（从大到小）
- `std::greater` 是 `<functional>` 头文件中预定义的函数对象，但很多编译器通过 `<algorithm>` 间接包含

#### 7. Dijkstra 算法的"惰性删除"

```cpp
// 如果已经确定过最短路径，跳过（惰性删除）
if (settled.count(cur_id)) continue;
settled.insert(cur_id);
```

- 优先队列里可能有**同一个节点的多个不同距离记录**（每次松弛都 push 新记录，不会删旧的）
- 当某个节点第一次从堆顶弹出时，它的当前距离就是全局最短距离（Dijkstra 的核心性质：所有边权非负）
- 之后再弹出这个节点时，跳过即可——这就是**惰性删除**（Lazy Deletion）

#### 8. HH:MM 字符串比较的巧妙之处

```cpp
auto isOpen = [&](const std::string &place_id) {
    LocationInfo info = graph.GetVertex(place_id);
    return info.open_time <= time && time <= info.close_time;
};
```

- 时间格式 `HH:MM`（如 `"08:00"`、`"22:00"`）中，**字符串字典序恰好等于时间先后的数值序**
- `"08:00" <= "12:00"` 为 true，`"12:00" <= "22:00"` 也为 true
- 前提：小时和分钟都必须是**两位数字**，零填充（01, 02, ..., 09）

---

## 四、CommandProcessor

### （一）[CommandProcessor.h里面](CampusNavigation/CommandProcessor.h)

#### 1. 类成员用引用 `LGraph &graph`——避免拷贝

```cpp
class CommandProcessor {
private:
    LGraph &graph;  // 注意这里 graph 是引用，因为都是操作同一幅图
    // ...
};
```

- `graph` 是**引用成员**，指向外部的图对象
- 不用拷贝：一张图可能有几百个节点，拷贝代价大
- 确保所有命令操作的都是同一幅图（`main.cpp` 中构造的 `LGraph graph(false)`)
- **注意**：引用成员必须在构造函数的初始化列表里初始化，不能延迟赋值

#### 2. `istringstream` 作为引用参数传递

```cpp
void cmdLoad(std::istringstream &args);
void cmdShortest(std::istringstream &args);
```

- `args` 是 `ProcessCommand` 中的 `iss` 的引用
- 因为是**引用传递**，`args` 的读取位置会改变，调用方 `iss` 的剩余内容同步减少
- 如果用值传递，会拷贝整个流，而且读位置的变化不会影响原流

### （二）[CommandProcessor.cpp里面](CampusNavigation/CommandProcessor.cpp)

#### 1. `try-catch` 异常处理模式——谁调用谁负责

```cpp
try {
    graph.InsertVertex(info);
    std::cout << "OK" << std::endl;
} catch (const GraphException &e) {
    std::string what(e.what());
    if (what.find("place_already_exists") != std::string::npos) {
        std::cout << "ERROR place_already_exists" << std::endl;
    } else {
        std::cout << "ERROR " << what << std::endl;
    }
}
```

- **原则**：谁调用函数谁承担后果——被调用函数抛出异常，调用者必须用 try-catch 处理
- `e.what()` 返回异常对象中存储的错误消息（C 风格字符串）
- 用 `std::string(e.what())` 转成 `std::string` 后，用 `.find()` 搜索错误类型关键词
- 这样底层（LGraph）只需要抛出一个详细异常，上层（CommandProcessor）统一解析并输出标准格式的错误信息

#### 2. `std::string::find()`——字符串子串查找

```cpp
if (what.find("place_not_found") != std::string::npos) {
    // 找到了 "place_not_found" 子串
}
```

- `find(sub)` 在字符串中查找子串 `sub`
- 找到 → 返回首次出现的位置（0, 1, 2, ...）
- 没找到 → 返回 `std::string::npos`（一个特殊常量，值约 18 亿亿）
- 与前面 CsvIO 中学的 `npos` 是同一个常量

#### 3. 排序放在 cmd 函数里而非被调用函数里

```cpp
// QUERY_CATEGORY 里
std::vector<std::string> ids = graph.GetPlacesByCategory(category);
std::sort(ids.begin(), ids.end());  // 排序放在这里处理
```

- 不把排序硬编码在 `GetPlacesByCategory()` 里面，因为有些调用场景可能不需要排序
- 只在 cmd 函数里（命令行输出要求）才排序，其他场景（如算法内部使用）免去 O(V log V) 的开销
- 这是"关注点分离"的体现：数据提供者只负责提供数据，格式要求由使用者处理

#### 4. `istringstream >>` 的返回值可以用来判断读取是否成功

```cpp
if (!(args >> places_path >> roads_path)) {
    std::cout << "ERROR invalid_arguments" << std::endl;
    return;
}
```

- `istringstream::operator>>` 返回对流的引用
- 当读取失败（参数不够/类型不匹配）时，流进入 fail 状态，`!stream` 返回 true
- 所以 `if (!(iss >> x))` 的意思是"如果读取 x 失败了"

---

## 五、测试数据（.bat文件）

### （一）[run_all_tests.bat里面](run_all_tests.bat)

#### 1. 批处理文件（.bat）是什么

- **本质**：一系列 Windows 命令的文本文件，双击即可自动逐行执行，无需手动一条条敲。
- 由 `cmd.exe` 解释运行（不是 C++ 那样的编译型程序）。
- 本脚本用于**自动遍历所有测试用例文件夹**，运行 `campus_nav.exe`，对比输出和标准答案，统计通过/失败。

#### 2. `echo`——屏幕输出

```batch
echo 文字       →  在屏幕上显示文字
echo.           →  输出一个空行
@echo off       →  放在开头，禁止显示每条命令本身，只显示命令的输出，让界面干净
```

- `@echo off` 中的 `@` 也作用于这一行自身，连 `echo off` 这条命令也不会显示。

#### 3. 变量

##### 普通变量
```batch
set EXE=C:\myapp.exe     →  定义变量 EXE
%EXE%                    →  使用时用 %变量名% 包裹
```

##### 带 `/a` 的算术运算
```batch
set /a PASS+=1           →  /a 表示算术运算（Arithmetic），把 PASS 的值加 1
```

- 类似 `+=`、`-=`、`*=`、`/=` 都可用。
- 如果没有 `/a`，`set PASS=%PASS%+1` 只会把变量赋值为字符串 `"0+1"`，不会算出 1。

##### 延迟展开 `!变量名!`
```batch
setlocal enabledelayedexpansion    →  启用延迟环境变量展开
!PASS!                             →  在循环体内部获取实时更新的值
```

- 若用 `%PASS%`，在 `for` 循环中可能一直读到循环**之前**的旧值（因为 `%VAR%` 在循环开始前就被替换了）。
- `!PASS!` 则在**每次执行到这一行时**才展开，能读到最新值。

##### `for` 循环的迭代变量 `%%d`
```batch
for /d %%d in ("路径\*") do (
    echo %%~nxd
)
```

- `%%d`：`for` 循环的迭代变量，每次循环中代表当前找到的文件夹名。
- `%%` 加单个字母（`%%d`、`%%i`），区分大小写（`%%D` 和 `%%d` 不同）。
- **注意**：在命令行直接敲 `for` 命令时用一个 `%`（如 `%d`），但在 `.bat` 文件中必须用 `%%d`。
- `%%~nxd`：扩展语法，只取文件夹名（不含路径）。

#### 4. 常用参数含义

| 参数 | 含义 | 示例 |
|------|------|------|
| `/d` | `for /d` 只匹配**目录（文件夹）**，不匹配普通文件 | `for /d %%d in (*)` |
| `/a` | `set /a` 后面是**算术运算** | `set /a TOTAL+=1` |
| `/w` | `fc /w` 比较文件时**忽略空白字符**（空格、制表符） | `fc /w my.txt answer.txt` |

#### 5. 输入输出重定向 `<`、`>`、`2>nul`

| 符号 | 含义 |
|------|------|
| `程序 < 文件` | 将文件内容**作为程序的键盘输入**（stdin） |
| `程序 > 文件` | 将程序的**屏幕输出保存到文件**（覆盖原文件） |
| `程序 2>nul` | 把**标准错误输出流**（stderr，文件描述符=2）丢进空设备 |

- `nul`：Windows 的空设备，相当于"黑洞"——写入它的数据直接消失，不会显示。
- 为什么不写 `1>nul`？`1` 是 stdout，`>` 默认就重定向 stdout，所以 `>nul 2>nul` 会同时隐藏正常输出和错误输出。

#### 6. `pushd` / `popd`——临时切换目录

```batch
pushd "目标文件夹"    →  保存当前目录，然后切换到目标文件夹
popd                  →  回到 pushd 之前保存的目录
```

- 比 `cd` 更安全：不需要记住从哪来的，`popd` 自动恢复。
- 用一个**栈结构**保存路径，支持嵌套。

#### 7. `fc`——文件比较命令

```batch
fc /w my_answer.txt answer.txt
```

- `fc` = **F**ile **C**ompare，比较两个文件的内容差异。
- `/w`：忽略空白字符（空格、制表符等），只比较"实质性内容"。
- 返回 `errorlevel=0` 表示文件**完全一致**，非零表示有差异。

#### 8. `errorlevel`——上一条命令的退出码

```batch
if !errorlevel!==0 (
    echo [PASS]
) else (
    echo [FAIL]
)
```

- `errorlevel` 是 `cmd.exe` 内置变量，记录**上一条命令的退出码**。
- 惯例：**0** 表示成功/无差异，**非0** 表示失败/有差异。
- 在 `setlocal enabledelayedexpansion` 下，需要用 `!errorlevel!` 才能在循环内读到实时值。

#### 9. `endlocal`——结束本地变量作用域

```batch
setlocal enabledelayedexpansion
...
endlocal
```

- `setlocal` 开启一个"本地环境"，里面定义的变量**不会污染外面的环境**。
- `endlocal` 结束本地环境，恢复 `setlocal` 之前的变量状态。
- 成对使用，防止脚本中的临时变量影响命令行后续操作。

#### 10. 整行详解：`"%EXE%" < command.txt > my_answer.txt 2>nul`

```batch
"%EXE%" < command.txt > my_answer.txt 2>nul
```

逐部分拆解：

| 部分 | 含义 |
|------|------|
| `"%EXE%"` | 用双引号包裹，防止路径中有空格时出错 |
| `< command.txt` | 把 `command.txt` 的内容作为程序的**键盘输入（stdin）** |
| `> my_answer.txt` | 把程序的**正常输出（stdout）保存到文件** `my_answer.txt`（覆盖原有文件） |
| `2>nul` | 把**标准错误输出流（stderr，文件描述符=2）丢进空设备** `nul`（黑洞），不显示在屏幕上 |

#### 11. `fc` 的详细返回值（errorlevel）

```batch
fc /w my_answer.txt answer.txt >nul 2>nul
```

- `>nul 2>nul`：fc 不仅返回退出码，还会**打印差异内容**到 stdout。这里用双重重定向把比较结果和错误都隐藏掉，**只靠返回码判断**。
- 返回码含义：

| errorlevel | 含义 |
|-----------|------|
| 0 | 两文件**完全相同** |
| 1 | 文件不同 |
| >1 | 比较过程出错（如文件不存在） |

- `if !errorlevel!==0` 就是判断两文件是否完全一致。

#### 12. `%%~nxd`——扩展语法，只取文件夹名

```batch
echo [PASS] %%~nxd
```

- `%%d` 是一个完整路径字符串（如 `D:\...\测试数据\必做\small_cases\case_01`）。
- `%%~nxd` 是 **for 变量的扩展语法**，只提取**文件名和扩展名**（不含路径）。
- 显示时只看到 `case_01`，而不是完整路径。

---

### （二）脚本逐块功能解读

| 脚本段 | 作用 |
|--------|------|
| `set EXE=...` | 指定你的可执行程序路径（`campus_nav.exe`） |
| `set TEST_DIR=...` | 指定测试数据根目录（如 `测试数据\必做`） |
| `set PASS=0` 等 | 初始化计数器（通过、失败、总数） |
| `for /d %%d in (...)` | 遍历指定目录下的每一个子文件夹（每个子文件夹是一个测试用例） |
| `set /a TOTAL+=1` | 测试用例总数加 1 |
| `pushd "%%d"` | 进入该用例文件夹 |
| `"%EXE%" < command.txt > my_answer.txt` | 用用例的 `command.txt` 运行程序，生成 `my_answer.txt` |
| `fc /w my_answer.txt answer.txt` | 比较学生输出与标准答案 |
| `if !errorlevel!==0 ...` | 根据比较结果输出 [PASS] 或 [FAIL] |
| `popd` | 返回上级目录，准备处理下一个用例 |
| 末尾 `echo 测试结果: ...` | 打印总结：通过/失败数量 |

---

### （三）自己使用时的修改步骤

1. **修改可执行文件路径**
   ```batch
   set EXE=你的campus_nav.exe的完整路径
   ```
   （在你的项目中是 `CampusNavigation\campus_nav.exe`）

2. **修改测试数据根目录**
   ```batch
   set TEST_DIR=你的测试数据\必做 的路径
   ```

3. **确保每个测试用例文件夹结构正确**
   每个文件夹内必须有：
   - `command.txt`（程序输入）
   - `answer.txt`（期望输出）
   - 如果程序还需要 `places.csv`、`roads.csv` 等数据文件，也需放在同目录。

4. **保存为 .bat 文件，双击运行**，或在命令行执行 `脚本名.bat`。

---

### （四）常见问题

**Q1: 为什么用 `!PASS!` 而不是 `%PASS%`？**

因为在 `for` 循环内部，`%PASS%` 会被**提前展开**为循环开始前的值，导致显示不更新。开启 `enabledelayedexpansion` 后使用 `!PASS!` 可获取每次循环后的最新值。

**Q2: 脚本执行后看不到 `my_answer.txt`？**

正常情况下，`my_answer.txt` 会在测试结束后仍然留在用例文件夹里，可以直接打开查看。如果没有，可能是程序运行出错，可尝试**去掉 `2>nul`** 查看错误信息。

**Q3: `fc` 比较太严格，因为 MST 不唯一导致失败的怎么办？**

可以将 `fc` 替换为自定义的"语义比较"脚本（如 Python 脚本），只提取关键数值（总距离、连通性等）进行比对，而非逐字符对比。在 README 中也说明了 `answer.txt` 不要求字符级一致。

**Q4: `%%d` 和 `%d` 区别？**

在 `.bat` 文件中必须用 `%%d`；如果在命令行直接敲 `for` 命令，用一个 `%`（如 `%d`）。

**Q5: `2>nul` 可以去掉吗？**

可以，但测试时会在屏幕上输出大量无关错误信息。建议保留，出错时可暂时去掉 `2>nul` 来调试。