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
#include "LocationInfo.h"
#include "GraphException.h"

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
            : from_id(std::move(f)), to_id(std::move(t)),
              distance(d), walk_time(w), status(std::move(s)) {}
    };

    class LGraph {
    private:
        // ==================================================
        // TODO: 在此处自行设计你的图数据结构
        // ==================================================
        // 可考虑的存储方案（不限于）：
        //   1. 邻接表：vector + list / vector + vector / unordered_map + list ...
        //   2. 邻接矩阵：二维 vector / 一维 vector 模拟二维
        //   3. 边表 + 顶点索引：单一 vector<EdgeNode> 配合 id→索引 的 map
        //   4. 以 place_id 为键的嵌套 hash map（string→string→EdgeNode）
        //   5. 其它你自创的组合
        //
        // 设计时请思考：
        //   - 如何快速通过 place_id 找到顶点？
        //   - 如何支持高效的增删边？
        //   - 删除顶点后，相关资源如何处理（立即回收 / 逻辑删除 / slot 回收）？
        //   - 无向图中同一条边如何避免重复存储，同时又能快速双向查询？
        // ==================================================

    public:
        explicit LGraph(bool directed = false);

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
