//
//动态图上的校园路线规划与设施分析系统
//Algorithm.cpp - 图算法实现（BFS / Dijkstra / Kruskal / 关键分析）
//

#include "Algorithm.h"
#include <queue>          //queue（BFS）、priority_queue（Dijkstra 小顶堆）
#include <unordered_map>  //dist / prev / visited 的哈希表
#include <unordered_set>  //被屏蔽节点/边的快速查询
#include <algorithm>      //sort, reverse
#include <climits>        //INT_MAX                                                    //这里我归纳出了用到头文件的地方

namespace Graph {
    namespace Algorithm {

        // ================================================================
        // 辅助函数：获取边的权值，根据 PathMode 选择 distance 或 walk_time
        // ================================================================
        static int getWeight(const EdgeNode &edge, PathMode mode) {
            if (mode == PathMode::DIST) {
                return edge.distance;
            } else { // PathMode::TIME
                return edge.walk_time;
            }
        }

        // ================================================================
        // 辅助函数：BFS 计算连通分量（支持屏蔽指定节点和边）
        // ────────────────────────────────────────────
        //   blocked_nodes : 被"临时删除"的节点集合（用于关键节点分析）
        //   blocked_edges : 被"临时删除"的边集合，格式 "u|v"（用于关键边分析）
        //   仅遍历 status == "open" 的边
        //   返回每个分量的节点数（未排序）
        // ================================================================
        static std::vector<int> bfsComponents(
            const LGraph &graph,
            const std::unordered_set<std::string> &blocked_nodes,
            const std::unordered_set<std::string> &blocked_edges) {

            std::vector<int> component_sizes;                 //各分量的节点数
            std::unordered_set<std::string> visited;             //已访问节点

            // 辅助函数：判断边 (u,v) 是否被屏蔽
            // blocked_edges 中存 "u|v" 和 "v|u" 两种形式（其中 u < v）
            auto isEdgeBlocked = [&](const std::string &a, const std::string &b) {      //class lambda [](const std::string &a, const std::string &b)->bool         //auto 在这里不能写成 bool，是因为lambda 表达式本身是一个“可调用__对象__”，不是一个 bool 值。
                                                                                        /*这是 C++ 的 lambda 表达式（匿名函数）。
                                                                                        [&] 是捕获列表，表示这个匿名函数可以引用外部作用域的所有局部变量（比如 blocked_edges、graph、time 等），无需传参也能访问。*/
                                                                                /*[&]：以引用方式捕获所有外部变量，效率高（不拷贝），但要注意生命周期。
                                                                                (const std::string &a, const std::string &b)：参数列表，和普通函数一样。
                                                                                { ... }：函数体。*/
                // 统一顺序：id 较小的在前
                std::string key = (a < b) ? (a + "|" + b) : (b + "|" + a);
                return blocked_edges.count(key) > 0;        //blocked_edges里面有key
            };

            // 遍历所有顶点
            std::vector<std::string> all_ids = graph.AllPlaceIds();
            for (const auto &start_id : all_ids) {      //第一层循环：已到访过得continue，若是屏蔽点continue
                // 跳过被屏蔽的节点
                if (blocked_nodes.count(start_id)) continue;        //（第一种屏蔽类型：删除点）
                if (visited.count(start_id)) continue;

                // BFS 遍历该分量
                std::queue<std::string> q;
                q.push(start_id);
                visited.insert(start_id);
                int comp_size = 0;

                while (!q.empty()) {           //第二层循环：从这个点出发，已到访过得continue，若是屏蔽点continue，若是边被屏蔽continue
                    std::string cur = q.front();
                    q.pop();
                    comp_size++;        //当前联通分量的大小

                    // 遍历所有邻接边（仅 status == "open" 的才走）
                    std::vector<EdgeNode> adj_edges = graph.GetAdjacentEdges(cur);
                    for (const auto &edge : adj_edges) {
                        if (edge.status != "open") continue;           // 只走 open 边

                        const std::string &neighbor = edge.to_id;        //对端节点
                        if (blocked_nodes.count(neighbor)) continue;   //邻居被屏蔽（第一种屏蔽类型：删除点）
                        if (isEdgeBlocked(cur, neighbor)) continue;    //边被屏蔽（第二种屏蔽类型：删除边）
                        if (visited.count(neighbor)) continue;

                        visited.insert(neighbor);
                        q.push(neighbor);
                    }
                }

                component_sizes.push_back(comp_size);
            }

            return component_sizes;
        }

        // ================================================================
        // A. 连通分量分析
        // ────────────────────────────────────────────
        //   直接调用 bfsComponents（无屏蔽），返回分量数和各分量大小（降序）
        //   时间复杂度 O(V + E)
        // ================================================================
        ComponentsResult GetConnectedComponents(const LGraph &graph) {
            std::unordered_set<std::string> empty_nodes;
            std::unordered_set<std::string> empty_edges;        //这两个是为了调用bfsComponents()函数才构造的
            std::vector<int> sizes = bfsComponents(graph, empty_nodes, empty_edges);

            // 按降序排列
            std::sort(sizes.begin(), sizes.end(), std::greater<int>());

            ComponentsResult result;
            result.count = static_cast<int>(sizes.size());          /*.size() 返回 size_t，
                                                                    而后面要用 n 作为数组大小、循环边界等，习惯上用有符号的 int 更安全（避免无符号数运算可能带来的意外问题）。对于节点数在几千以内的情况完全安全。*/
            result.sizes = sizes;
            return result;
        }

        // ================================================================
        // B. 最短路径 — Dijkstra + 小顶堆
        // ────────────────────────────────────────────
        //   根据 mode 选择边权（DIST 用 distance，TIME 用 walk_time）
        //   仅走 status == "open" 的边
        //   blocked_nodes   : 额外屏蔽的节点（用于关键节点分析的变体，本函数不传）
        //   时间复杂度 O((V+E) log V)
        //
        //   实现要点：
        //   - 使用 priority_queue<pair<int,string>, vector<...>, greater<...>> 实现小顶堆
        //   - 用 unordered_map<string, int> 存 dist（字符串 ID 做键）
        //   - 用 unordered_map<string, string> 存 prev 前驱，用于回溯路径
        //   - 松弛条件：dist[cur] + weight < dist[neighbor]
        // ================================================================
        PathResult GetShortestPath(const LGraph &graph,
                                   const std::string &from_id,
                                   const std::string &to_id,
                                   PathMode mode) {
            PathResult result;

            // 预处理：判断起终点是否都存在（调用方保证，这里再检查一次）
            if (!graph.exist_vertex(from_id) || !graph.exist_vertex(to_id)) {
                return result; // reachable = false
            }

            // Dijkstra 初始化                              //都是用place_id 来对应的，所以要用map
            std::unordered_map<std::string, int> dist;        //place_id → 最短距离/时间
            std::unordered_map<std::string, std::string> prev;      //place_id → 前驱（回溯路径用）
            std::unordered_set<std::string> settled;        //已确定最短路的节点集合
                                                            /*不关心顺序，所以比起map，用 unordered_set 更合适（O(1) 查找）。*/

            // 小顶堆：pair<当前代价, place_id>
            // greater<pair<...>> 使堆顶是代价最小的元素
            using P = std::pair<int, std::string>;
            std::priority_queue<P, std::vector<P>, std::greater<P>> pq;     /*priority_queue 默认是大顶堆（最大的在堆顶）。
                                                                            第三个模板参数是比较器，传入 std::greater<P> 后，比较规则反转，变成小顶堆（最小的在堆顶）。
                                                                            std::greater<P> 是一个函数对象，它比较 pair 时先比 first（即距离/时间），再比 second。于是堆顶是当前 first 最小的那个 pair。*/

            // 所有地点初始距离为 INT_MAX
            std::vector<std::string> all_ids = graph.AllPlaceIds();
            for (const auto &id : all_ids) {
                dist[id] = INT_MAX;         //这个需使用<climits> 
            }

            // 起点距离 = 0
            dist[from_id] = 0;
            pq.push({0, from_id});

            // Dijkstra 主循环
            while (!pq.empty()) {
                auto [cur_dist, cur_id] = pq.top();  // C++17 结构化绑定（structured binding）
                pq.pop();

                // 如果已经确定过最短路径，跳过（惰性删除）
                if (settled.count(cur_id)) continue;        //返回：for unordered_set the result will either be 0 (not present) or 1 (present).
                                                            /*Dijkstra 算法的核心性质：所有边权非负时，第一次从小顶堆中弹出某个节点时，它当前的距离就是全局最短距离。*/
                settled.insert(cur_id);

                // 到达终点，可以提前退出（Dijkstra 的贪心性质保证首次弹出即为最短）
                if (cur_id == to_id) break;

                // 遍历邻接边进行松弛
                std::vector<EdgeNode> adj_edges = graph.GetAdjacentEdges(cur_id);
                for (const auto &edge : adj_edges) {
                    if (edge.status != "open") continue;            //只走 open 边
                    const std::string &neighbor = edge.to_id;

                    int weight = getWeight(edge, mode);          //根据模式取边权
                    // 检查是否可松弛
                    if (cur_dist != INT_MAX && cur_dist + weight < dist[neighbor]) {
                        dist[neighbor] = cur_dist + weight;
                        prev[neighbor] = cur_id;               // 记录前驱
                        pq.push({dist[neighbor], neighbor});
                    }
                }
            }

            // 判断是否可达
            if (dist[to_id] == INT_MAX) {
                return result;      //reachable = false
            }

            // 回溯路径：从 to_id 沿 prev 往前推到 from_id
            result.total_cost = dist[to_id];
            result.reachable = true;
            std::string cur = to_id;
            while (cur != from_id) {
                result.path.push_back(cur);
                cur = prev[cur];
            }
            result.path.push_back(from_id);
            // 目前是 to → from 的顺序，反转得到 from → to
            std::reverse(result.path.begin(), result.path.end());       //别忘了

            return result;
        }

        // ================================================================
        // B'. 时刻约束最短路径
        // ────────────────────────────────────────────
        //   在给定时刻 time（HH:MM）下求最短路径
        //   除了普通 Dijkstra 的条件外，额外要求：
        //     - 路径上所有地点（含起止点）在 time 时必须开放
        //       （open_time <= time <= close_time，HH:MM 字符串天然支持字典序比较）
        //     - 若起点或终点此时不开放，直接返回不可达
        //
        //   实现：先检查起止点时间窗，再跑 Dijkstra，松弛时额外检查邻居的时间窗
        //   时间复杂度同 Dijkstra O((V+E) log V)
        // ================================================================
        PathResult GetTimedShortestPath(const LGraph &graph,
                                        const std::string &from_id,
                                        const std::string &to_id,
                                        const std::string &time,
                                        PathMode mode) {
            PathResult result;

            if (!graph.exist_vertex(from_id) || !graph.exist_vertex(to_id)) {
                return result;
            }

            //辅助 lambda：判断某地点在 time 时刻是否开放
            //HH:MM 格式下，字符串字典序比较等价于时间比较：
            //   "08:00" <= "12:00" <= "22:00" 天然成立
            auto isOpen = [&](const std::string &place_id) {        //bool isOpen(const std::string &place_id)
                LocationInfo info = graph.GetVertex(place_id);
                return info.open_time <= time && time <= info.close_time;
            };

            // 起点或终点此时不开放 → 直接不可达
            if (!isOpen(from_id) || !isOpen(to_id)) {
                return result;
            }

            //初始化                                   //都是用place_id 来对应的，所以要用map
            std::unordered_map<std::string, int> dist;      // place_id → 最短距离/时间
            std::unordered_map<std::string, std::string> prev;
            std::unordered_set<std::string> settled;

            using P = std::pair<int, std::string>;
            std::priority_queue<P, std::vector<P>, std::greater<P>> pq;

            std::vector<std::string> all_ids = graph.AllPlaceIds();
            for (const auto &id : all_ids) {
                dist[id] = INT_MAX;
            }

            dist[from_id] = 0;
            pq.push({0, from_id});

            while (!pq.empty()) {
                auto [cur_dist, cur_id] = pq.top();
                pq.pop();

                if (settled.count(cur_id)) continue;
                settled.insert(cur_id);

                if (cur_id == to_id) break;

                std::vector<EdgeNode> adj_edges = graph.GetAdjacentEdges(cur_id);
                for (const auto &edge : adj_edges) {
                    if (edge.status != "open") continue;
                    const std::string &neighbor = edge.to_id;

                    // ***时刻约束的核心：邻居在 time 时必须开放，才能走过去***
                    if (!isOpen(neighbor)) continue;

                    int weight = getWeight(edge, mode);
                    if (cur_dist != INT_MAX && cur_dist + weight < dist[neighbor]) {
                        dist[neighbor] = cur_dist + weight;
                        prev[neighbor] = cur_id;
                        pq.push({dist[neighbor], neighbor});
                    }
                }
            }

            if (dist[to_id] == INT_MAX) {
                return result;      //reachable = false
            }

            result.total_cost = dist[to_id];
            result.reachable = true;
            std::string cur = to_id;
            while (cur != from_id) {
                result.path.push_back(cur);
                cur = prev[cur];
            }
            result.path.push_back(from_id);
            std::reverse(result.path.begin(), result.path.end());

            return result;
        }

        // ================================================================
        // C. 必经点路径规划
        // ────────────────────────────────────────────
        //   将路径拆成多段：from → w1 → w2 → ... → wk → to
        //   每段调用 GetShortestPath，拼接结果
        //   注意：拼接时去掉重复的中间衔接点（前一段终点 = 后一段起点）
        //   任一段不可达则整体不可达
        //   时间复杂度 O(k · (V+E) log V)，k 为必经点个数
        // ================================================================
        PathResult GetMustPassPath(const LGraph &graph,
                                   const std::string &from_id,
                                   const std::string &to_id,
                                   PathMode mode,
                                   const std::vector<std::string> &waypoints) {
            PathResult result;

            // 构建完整访问序列：起点 + 必经点 + 终点
            std::vector<std::string> stops;
            stops.push_back(from_id);
            for (const auto &wp : waypoints) {
                stops.push_back(wp);
            }
            stops.push_back(to_id);     //得到所有一定得到的点

            result.reachable = true;        //先设成可到达，如果后面分段调用的时候不行，则再变成false
            result.total_cost = 0;

            // 逐段跑 Dijkstra
            for (size_t i = 0; i + 1 < stops.size(); ++i) {
                const std::string &seg_from = stops[i];
                const std::string &seg_to   = stops[i + 1];

                PathResult seg_result = GetShortestPath(graph, seg_from, seg_to, mode);
                if (!seg_result.reachable) {
                    // 某一段不可达 → 整个路径不可达
                    result.reachable = false;
                    result.total_cost = 0;
                    result.path.clear();
                    return result;
                }

                // 累加代价
                result.total_cost += seg_result.total_cost;

                //拼接路径：去掉重复的衔接点（当前段起点 = 前一段终点）
                //除了第一段，后续段跳过第一个节点
                size_t start_idx = (i == 0) ? 0 : 1;        //从当前节点组（路径经过的点）第一个开始还是从第二个开始
                for (size_t j = start_idx; j < seg_result.path.size(); ++j) {
                    result.path.push_back(seg_result.path[j]);
                }
            }

            return result;
        }

        // ================================================================
        // D. 最小生成树 — Kruskal + DSU
        // ────────────────────────────────────────────
        //   选择 Kruskal 而非 Prim 的理由（写进实验报告）：
        //     1. Kruskal 只需要对所有边排序一次 O(E log E)，然后逐边判断
        //     2. DSU 操作近乎 O(1)，整体非常简洁
        //     3. 不需要像 Prim 那样维护"未加入顶点到当前树的最短距离"
        //     4. 对于稀疏图（E 接近 V），Kruskal 更优
        //
        //   实现步骤：
        //     1. 收集所有 open 边，按 distance 升序排序
        //     2. 给每个地点分配一个整数编号（DSU 需要整数索引）
        //     3. 从小到大尝试每条边 → 若两端不在同一集合，则加入 MST
        //     4. 最后检查 MST 边数 == V-1，否则图不连通
        //   时间复杂度 O(E log E)
        // ================================================================
        std::vector<EdgeNode> MinimumSpanningTree(const LGraph &graph) {
            // 1. 收集所有 open 边
            std::vector<EdgeNode> open_edges = graph.AllEdges(true); // only_open = true

            // 2. 按 distance 升序排序
            std::sort(open_edges.begin(), open_edges.end(),
                      [](const EdgeNode &a, const EdgeNode &b) {
                          return a.distance < b.distance;
                      });

            // 3. 建编号映射：place_id → 0,1,2,...（DSU 要用整数）
            std::vector<std::string> all_ids = graph.AllPlaceIds();             //因为我写的DSU算法里面parent，rank都是用的int
            int n = static_cast<int>(all_ids.size());
            std::unordered_map<std::string, int> id_to_idx;
            for (int i = 0; i < n; ++i) {
                id_to_idx[all_ids[i]] = i;
            }

            // 4. Kruskal 主循环
            DSU dsu(n);
            std::vector<EdgeNode> mst;

            for (const auto &edge : open_edges) {
                int u = id_to_idx[edge.from_id];
                int v = id_to_idx[edge.to_id];

                //若两端不在同一集合中，则这条边不会形成环，加入 MST
                if (!dsu.same(u, v)) {
                    dsu.unite(u, v);
                    mst.push_back(edge);
                }

                //提前终止：MST 边数 = V-1 即完成
                if (static_cast<int>(mst.size()) == n - 1) {
                    break;
                }
            }

            // 5. 检查连通性：MST 边数 != V-1 说明图不连通
            if (static_cast<int>(mst.size()) != n - 1) {
                return {};  // 返回空 vector → DISCONNECTED
            }

            return mst;
        }

        // ================================================================
        // E. 关键节点与关键边分析
        // ────────────────────────────────────────────
        //   关键节点：删去该顶点（及其所有邻接边）后，连通分量数增加
        //   关键边  ：删去该边后，连通分量数增加
        //
        //   实现方法（暴力枚举）：
        //     1. 先算 baseline（原图连通分量数）
        //     2. 关键节点：逐个"屏蔽"节点后重算分量数 → > baseline 则为关键节点
        //     3. 关键边  ：逐条"屏蔽"open 边后重算分量数 → > baseline 则为关键边
        //
        //   复杂度 O(V·(V+E) + E·(V+E))
        //   对于 1000 节点规模可接受
        // ================================================================
        CriticalResult FindCriticalNodesAndEdges(const LGraph &graph) {
            CriticalResult result;

            // 1. 计算原图的 baseline 连通分量数
            ComponentsResult baseline = GetConnectedComponents(graph);
            int baseline_count = baseline.count;

            std::vector<std::string> all_ids = graph.AllPlaceIds();

            // 2. 关键节点：逐个屏蔽每个节点
            for (const auto &node_id : all_ids) {
                std::unordered_set<std::string> blocked_nodes;
                blocked_nodes.insert(node_id);               // 屏蔽此节点及其所有邻边
                std::unordered_set<std::string> empty_edges;

                std::vector<int> sizes = bfsComponents(graph, blocked_nodes, empty_edges);
                int new_count = static_cast<int>(sizes.size());

                // 分量数增加 → 关键节点
                if (new_count > baseline_count) {
                    result.critical_nodes.push_back(node_id);
                }
            }

            // 3. 关键边：逐条屏蔽每条 open 边
            std::vector<EdgeNode> open_edges = graph.AllEdges(true);
            for (const auto &edge : open_edges) {
                std::unordered_set<std::string> empty_nodes;
                std::unordered_set<std::string> blocked_edges;

                // 构造屏蔽 key："u|v"（u < v）
                std::string key = (edge.from_id < edge.to_id)
                    ? (edge.from_id + "|" + edge.to_id)
                    : (edge.to_id + "|" + edge.from_id);
                blocked_edges.insert(key);

                std::vector<int> sizes = bfsComponents(graph, empty_nodes, blocked_edges);
                int new_count = static_cast<int>(sizes.size());

                // 分量数增加 → 关键边
                if (new_count > baseline_count) {
                    result.critical_edges.push_back({edge.from_id, edge.to_id});
                }
            }

            return result;
        }

    } //namespace Algorithm
} //namespace Graph