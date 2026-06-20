//
// 动态图上的校园路线规划与设施分析系统
// Algorithm.h - 图算法接口（含 DSU 并查集实现）
// 我觉得结构应该是一个算法要用到的DSU类，三种返回类型（PathResult，ComponentsResult，CriticalResult），一种模式枚举PathMode
//

#ifndef CAMPUSNAVIGATION_ALGORITHM_H
#define CAMPUSNAVIGATION_ALGORITHM_H

#include "LGraph.h"
#include <vector>
#include <string>
#include <utility>

namespace Graph {
    namespace Algorithm {

        // ==================== 并查集（DSU） ====================
        // 用于 Kruskal MST 算法，支持路径压缩 + 按秩合并
        class DSU {
        private:
            std::vector<int> parent;   // parent[x] = 父节点编号，根节点的 parent[x] == x
            std::vector<int> rank_;     // 秩（近似高度），用于按秩合并（unite()里）

        public:
            // 构造函数：n 个独立元素 0..n-1，每个都是自己的根
            explicit DSU(int n) {
                parent.resize(n);
                rank_.resize(n, 0);
                for (int i = 0; i < n; ++i) {
                    parent[i] = i;               // 初始化：每个人都指向自己
                }
            }

            // 查找根 + 路径压缩
            // 把沿路所有节点直接挂到根上，摊还 O(α(n)) ≈ O(1)
            int find(int x) {
                if (parent[x] != x) {
                    parent[x] = find(parent[x]);                        // 递归向上找根，回来时把整条链压缩
                }
                return parent[x];
            }

            // 合并两个集合（按秩合并）
            // 始终把矮树挂到高树下，保持整体高度不增加
            void unite(int x, int y) {
                int rx = find(x);
                int ry = find(y);
                if (rx == ry) return;            // 已在同一集合，直接返回

                // 矮的挂到高的下面
                if (rank_[rx] < rank_[ry]) {
                    parent[rx] = ry;
                } else if (rank_[rx] > rank_[ry]) {
                    parent[ry] = rx;
                } else {
                    parent[ry] = rx;           // 高度相同，随便挂
                    rank_[rx]++;          // 新树高度 +1
                }
            }

            // 判断两个元素是否属于同一集合
            bool same(int x, int y) {
                return find(x) == find(y);
            }
        };

        // ==================== 路径模式枚举 ====================
        enum PathMode {
            DIST,   // 按距离最短
            TIME    // 按步行时间最短
        };

        // ==================== 最短路径结果 ====================
        struct PathResult {
            int total_cost;                     // 总代价（距离或时间）
            std::vector<std::string> path;        // 完整路径的 place_id 序列
            bool reachable;                      // 是否可达

            PathResult() : total_cost(0), reachable(false) {}
        };

        // ==================== 连通分量结果 ====================
        struct ComponentsResult {
            int count;                     // 连通分量个数
            std::vector<int> sizes;        // 每个连通分量的规模（降序排列）
        };

        // ==================== 关键节点 / 关键边分析结果 ====================
        struct CriticalResult {
            std::vector<std::string> critical_nodes;
            std::vector<std::pair<std::string, std::string>> critical_edges;
        };

        // ==================== 算法函数声明 ====================
        /*这里说明一下，我自己在Algotithm.cpp里写了一个函数：
        static std::vector<int> bfsComponents(
            const LGraph &graph,
            const std::unordered_set<std::string> &blocked_nodes,
            const std::unordered_set<std::string> &blocked_edges)
        用于被A. 连通分量分析，E. 关键节点与关键边分析调用
        （因为这个函数是可以排除&blocked_nodes（临时阻断点）和&blocked_edges（临时阻断边的））*/

        // A. 连通分量分析（BFS，仅遍历 open 边）
        ComponentsResult GetConnectedComponents(const LGraph &graph);

        // B. 最短路径（Dijkstra + 小顶堆，DIST / TIME 双模式）
        PathResult GetShortestPath(const LGraph &graph,
                                   const std::string &from_id,
                                   const std::string &to_id,
                                   PathMode mode);

        // B'. 时刻约束最短路径（在给定时刻 HH:MM 下过滤不可用地点后跑 Dijkstra）
        PathResult GetTimedShortestPath(const LGraph &graph,
                                        const std::string &from_id,
                                        const std::string &to_id,
                                        const std::string &time,
                                        PathMode mode);

        // C. 必经点路径规划（按序拼接多段最短路）
        PathResult GetMustPassPath(const LGraph &graph,
                                   const std::string &from_id,
                                   const std::string &to_id,
                                   PathMode mode,
                                   const std::vector<std::string> &waypoints);

        // D. 最小生成树（Kruskal + DSU，按 distance）
        std::vector<EdgeNode> MinimumSpanningTree(const LGraph &graph);

        // E. 关键节点与关键边分析（暴力枚举法，逐个"屏蔽"后重算连通分量）
        //    复杂度 O(V·(V+E) + E·(V+E))
        CriticalResult FindCriticalNodesAndEdges(const LGraph &graph);
    }
}
#endif //CAMPUSNAVIGATION_ALGORITHM_H