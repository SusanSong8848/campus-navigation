//
// 动态图上的校园路线规划与设施分析系统
// LGraph.h - 图 ADT 公共接口
//
// 本头文件只声明图应当支持的公共操作，**不规定内部存储结构**。
// 请在 private 区自行设计你的数据结构，并在报告中说明：
//   - 你为什么选这种结构？
//   - 考虑过哪些替代方案？权衡是什么？
//   - 各个操作（增删改查、遍历、查邻接）的时间/空间复杂度？
//

#ifndef LGRAPH_LGRAPH_H
#define LGRAPH_LGRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include "LocationInfo.h"
#include "GraphException.h"

//这里我先说一下为什么要在namespace Graph里面写所有的代码和类：因为可以避免命名冲突：如果别人的库也有一个叫 LGraph 的类，我的就叫 __Graph::LGraph__，不会打架。
//namespace Graph 是 C++ 的命名空间，类似于给代码加一个“姓”
namespace Graph {

    /* 边（道路）信息 —— 纯数据载体，供公共接口返回使用。
       内部存储形式由你决定（可以用这个 struct，也可以用你自己的表示）。*/
    struct EdgeNode {
        std::string from_id;
        std::string to_id;
        int distance;
        int walk_time;
        std::string status;          /* "open" 或 "closed" */

        EdgeNode() : distance(0), walk_time(0), status("open") {}
        EdgeNode(std::string f, std::string t, int d, int w, std::string s)
            : from_id(std::move(f)), to_id(std::move(t)),       //std::move(),不拷贝数据，而是把资源的所有权从一个对象“转移”到另一个对象。
              distance(d), walk_time(w), status(std::move(s)) {}        //尤其对于长字符串。from_id 直接接管 f 的内部字符数组，不再额外拷贝。
    };                                                                  //能移动就移动，别拷贝。

    class LGraph {
    private:
        // ==================================================
        // 数据结构设计：邻接表 + 双哈希索引
        // ==================================================
        // 
        // 选择理由：
        //   1. vertices_: unordered_map<place_id, LocationInfo>
        //      - 通过 place_id 查找顶点信息 O(1)
        //      - 存储顶点的完整属性（名称、类别、时间窗等）
        //
        //   2. adj_: unordered_map<from_id, unordered_map<to_id, EdgeNode>>
        //      - 邻接表：从任一顶点出发，O(1) 定位其邻接边集合
        //      - 内层 map 允许 O(1) 查找特定边的信息
        //      - 无向图中每条边存储两次（两个方向），保证双向查询对称
        //
        // 替代方案权衡：
        //   - 邻接矩阵 O(V²)：校园图稀疏（87节点仅26边），空间浪费严重
        //   - 纯边表 vector<EdgeNode>：查特定边需 O(E) 遍历，Dijkstra 效率低
        //   - hash邻接表：空间 O(V+E)，增删改查均 O(1)，最适合本项目
        //
        // 各操作复杂度：
        //   exist_vertex / GetVertex: O(1)
        //   exist_edge / GetEdge: O(1)
        //   InsertVertex / DeleteVertex: O(1) / O(degree度)（需清理邻接表）
        //   InsertEdge / DeleteEdge: O(1)（双向各一次）
        //   遍历 AllEdges / AllPlaceIds: O(E) / O(V)
        // ==================================================

        bool directed_;  // 是否为有向图（本项目默认 false，无向图）

        // 顶点存储：place_id → 顶点完整信息
        std::unordered_map<std::string, LocationInfo> vertices_;        //vertices是vertex的复数

        // 邻接表：from_id → (to_id → 边信息)
        // 无向图中每条边在 adj_[u][v] 和 adj_[v][u] 各存一份
        std::unordered_map<std::string, std::unordered_map<std::string, EdgeNode>> adj_;

    public:
        explicit LGraph(bool directed = false);     /*explicit 防止隐式类型转换：如果不加，可能会出现这种奇怪的代码：
                                                        LGraph g = true;  // 居然把 bool 悄悄变成了 LGraph(true)
                                                        对于单参数构造函数，推荐加上 explicit 以避免意外*/

        // ==================== 基础信息 ====================
        int VertexCount() const;
        int EdgesCount() const;
        bool exist_vertex(const std::string &place_id) const;
        bool exist_edge(const std::string &from_id, const std::string &to_id) const;

        // ==================== 顶点操作 ====================
        void InsertVertex(const LocationInfo &vertex_info);
        void DeleteVertex(const std::string &place_id);
        void UpdateVertex(const std::string &place_id,
                          const std::string &field, const std::string &value);
        LocationInfo GetVertex(const std::string &place_id) const;

        // ==================== 边操作 ====================
        void InsertEdge(const std::string &from_id, const std::string &to_id,
                        int distance, int walk_time, const std::string &status);
        void DeleteEdge(const std::string &from_id, const std::string &to_id);
        void UpdateEdge(const std::string &from_id, const std::string &to_id,
                        const std::string &field, const std::string &value);
        EdgeNode GetEdge(const std::string &from_id, const std::string &to_id) const;

        // ==================== 道路状态 ====================
        void CloseRoad(const std::string &from_id, const std::string &to_id);
        void OpenRoad(const std::string &from_id, const std::string &to_id);

        // ==================== 遍历 / 高级查询 ====================
        // 下面这些方法服务于 Algorithm / CsvIO 等模块，避免它们直接访问你的私有存储。
        std::vector<std::string> AllPlaceIds() const;
            // 返回当前图中所有存在的地点 id（顺序由你决定）

        std::vector<EdgeNode> AllEdges(bool only_open = true) const;
            // 返回当前图中所有边（无向图中每条边只出现一次）
            // only_open = true 时仅返回 status == "open" 的边

        std::vector<EdgeNode> GetAdjacentEdges(const std::string &place_id) const;
            // 返回某地点的所有邻接边（详细信息，包括 status）

        std::vector<std::string> GetPlacesByCategory(const std::string &category) const;
            // 返回某类别下的所有 place_id
    };
}
#endif  // LGRAPH_LGRAPH_H