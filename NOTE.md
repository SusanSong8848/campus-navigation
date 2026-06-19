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