//
// 动态图上的校园路线规划与设施分析系统
// LGraph.cpp - 图 ADT 实现（邻接表 + 双哈希索引）
//

#include "LGraph.h"
#include <algorithm>
#include <sstream>

namespace Graph {

    // ==================== 构造函数 ====================
    // 初始化空图：vertices_ 和 adj_ 均为空 unordered_map
    LGraph::LGraph(bool directed) : directed_(directed) {       //vertices_ 和 adj_ 不传参数，为空
    }

    // ==================== 基础信息 ====================

    // 返回当前图中存在的顶点数
    int LGraph::VertexCount() const {
        return static_cast<int>(vertices_.size());      /*C++ 的显式类型转换。
                                                        size() 返回 size_t（无符号整型），函数返回类型是 int，用 static_cast<int> 转换一下，避免编译器警告类型不匹配。
                                                        对于小规模数据是安全的。*/
    }

    // 返回当前图中存在的边数（无向图中每条边算一次）
    // 遍历所有邻接表，统计边数时避免重复计数
    // 方法：对每条边只计 from_id < to_id 的方向（无向图）（这样比起(总变数/2)也要快一点）
    int LGraph::EdgesCount() const {
        int count = 0;
        for (const auto &from_pair : adj_) {
            const std::string &from_id = from_pair.first;
            for (const auto &to_pair : from_pair.second) {      //auto& 为 unordered_map<to_id, EdgeNode>&
                const std::string &to_id = to_pair.first;
                if (directed_) {
                    // 有向图：每条边都计数
                    ++count;
                } else {
                    // 无向图：只计 from_id < to_id 的边，避免重复
                    if (from_id < to_id) {
                        ++count;
                    }
                }
            }
        }
        return count;
    }

    // 判断 place_id 是否存在于 vertices_ 中
    bool LGraph::exist_vertex(const std::string &place_id) const {
        return vertices_.find(place_id) != vertices_.end();         //若unordered_map::vertices_.find(place_id)没找到就会返回vertices_.end()
    }

    // 判断 from_id 与 to_id 之间是否存在边
    // 先在 adj_ 中找 from_id 的邻接表，再在其中找 to_id
    bool LGraph::exist_edge(const std::string &from_id, const std::string &to_id) const {
        auto from_it = adj_.find(from_id);
        if (from_it == adj_.end()) return false;        //起点不存在（但是我觉得应该不要也行，因为怎么会没有一个点呢？除非是from_id写错了）

        auto to_it = from_it->second.find(to_id);
        return to_it != from_it->second.end();
    }

    // ==================== 顶点操作 ====================

    // 插入顶点
    //   - 若 place_id 已存在，抛出 GraphException
    void LGraph::InsertVertex(const LocationInfo &vertex_info) {
        const std::string &id = vertex_info.place_id;       //先获取id名，方便vertices_，adj_插入
        if (vertices_.find(id) != vertices_.end()) {        
            throw GraphException("place_already_exists: " + id);    //place_id 已存在，抛出 GraphException
        }

        // 将顶点信息存入 vertices_
        vertices_[id] = vertex_info;        //更新vertices_

        // 同时为该顶点在 adj_ 中初始化一个空的邻接表（后续 InsertEdge 会往里加边）
        if (adj_.find(id) == adj_.end()) {
            adj_[id] = std::unordered_map<std::string, EdgeNode>();     //更新adj_
        }
    }

    // 删除顶点，并同步删除所有与该顶点相关的边
    //   - 若 place_id 不存在，抛出 GraphException
    void LGraph::DeleteVertex(const std::string &place_id) {
        auto vertex_it = vertices_.find(place_id);      //unordered_map.find()返回：Iterator pointing to sought-after element, or end() if not found.
        if (vertex_it == vertices_.end()) {
            throw GraphException("place_not_found: " + place_id);
        }

        // 步骤1：删除该顶点的所有关联边
        // 遍历该顶点的邻接表，对于每条邻边 (place_id → neighbor)：
        //   在 neighbor 的邻接表中也删掉 (neighbor → place_id) 这一条  //（即记得删两次）
        auto adj_it = adj_.find(place_id);
        if (adj_it != adj_.end()) {
            //在 neighbor 的邻接表中删neighbor → place_id
            for (const auto &neighbor_pair : adj_it->second) {
                const std::string &neighbor_id = neighbor_pair.first;

                // 在 neighbor 的邻接表中删除 place_id
                auto neighbor_adj_it = adj_.find(neighbor_id);
                if (neighbor_adj_it != adj_.end()) {
                    neighbor_adj_it->second.erase(place_id);
                }
            }
            // 清空该顶点的邻接表
            adj_.erase(adj_it);
        }

        // 步骤2：删除顶点本身
        vertices_.erase(vertex_it);
    }

    // 按字段名更新顶点信息：UpdateVertex(地点名，要更新什么信息，要更的新值)
    //   支持的字段：display_name, category, stay_time, open_time, close_time
    //   - stay_time 需要将 value 转为 int
    //   - place_id 不存在 → GraphException
    //   - field 不支持 → GraphException
    void LGraph::UpdateVertex(const std::string &place_id,
                              const std::string &field, const std::string &value) {
        auto vertex_it = vertices_.find(place_id);          //vertex_it是vertex_iterator的意思，迭代器，类似指针
                                                            //再写一次加深印象：如果unordered_map.find()找到了就返回：Iterator pointing to sought-after element
        if (vertex_it == vertices_.end()) {
            throw GraphException("place_not_found: " + place_id);
        }

        LocationInfo &info = vertex_it->second;     //vertex_iterator指向vertex里的元素，类型是<std::string, LocationInfo>
        // 根据字段名更新对应属性
        if (field == "display_name") {
            info.display_name = value;
        } else if (field == "category") {
            info.category = value;
        } else if (field == "stay_time") {
            // stay_time 是 int 类型，需要转换
            info.stay_time = std::stoi(value);      //从string -> int（stoi 是 string to int 的缩写）
        } else if (field == "open_time") {
            info.open_time = value;
        } else if (field == "close_time") {
            info.close_time = value;
        } else {
            throw GraphException("invalid_field: " + field);
        }
    }

    // 返回 place_id 对应的顶点完整信息，返回一个LocationInfo
    LocationInfo LGraph::GetVertex(const std::string &place_id) const {
        auto it = vertices_.find(place_id);
        if (it == vertices_.end()) {
            throw GraphException("place_not_found: " + place_id);
        }
        return it->second;
    }

    // ==================== 边操作 ====================

    // 插入边
    //   - 两端顶点必须都存在，否则抛异常
    //   - 边已存在则抛 GraphException
    //   - 无向图中注意反向关联
    void LGraph::InsertEdge(const std::string &from_id, const std::string &to_id,
                            int distance, int walk_time, const std::string &status) {
        if (!exist_vertex(from_id)) {
            throw GraphException("place_not_found: " + from_id);
        }
        if (!exist_vertex(to_id)) {
            throw GraphException("place_not_found: " + to_id);
        }
        if (exist_edge(from_id, to_id)) {
            throw GraphException("road_already_exists: " + from_id + " -> " + to_id);
        }

        // 创建边对象
        EdgeNode edge(from_id, to_id, distance, walk_time, status);

        // 在邻接表中插入正向边：from_id → to_id
        adj_[from_id][to_id] = edge;

        if (!directed_) {
            // 无向图：同时插入反向边 to_id → from_id
            // 注意反向边的 from_id/to_id 也需要交换
            EdgeNode reverse_edge(to_id, from_id, distance, walk_time, status);
            adj_[to_id][from_id] = reverse_edge;
        }
    }

    // 删除边
    //   - 边不存在 → GraphException
    //   - 无向图中反向边同步删除
    void LGraph::DeleteEdge(const std::string &from_id, const std::string &to_id) {
        if (!exist_edge(from_id, to_id)) {
            throw GraphException("road_not_found: " + from_id + " -> " + to_id);
        }

        // 删除正向边
        adj_[from_id].erase(to_id);

        if (!directed_) {
            // 无向图：同时删除反向边
            adj_[to_id].erase(from_id);
        }
    }

    // 按字段名更新边
    //   支持字段：distance, walk_time, status
    //   - distance / walk_time 需要转为 int
    //   - status 只能是 "open" 或 "closed"
    //   - 无向图中两个方向需同时更新
    void LGraph::UpdateEdge(const std::string &from_id, const std::string &to_id,
                            const std::string &field, const std::string &value) {
        if (!exist_edge(from_id, to_id)) {
            throw GraphException("road_not_found: " + from_id + " -> " + to_id);
        }

        // 获取边引用
        EdgeNode &edge = adj_[from_id][to_id];

        // 根据字段名更新
        if (field == "distance") {
            int new_dist = std::stoi(value);
            edge.distance = new_dist;
            // 无向图：另一方向也要更新
            if (!directed_) {
                adj_[to_id][from_id].distance = new_dist;
            }
        } else if (field == "walk_time") {
            int new_time = std::stoi(value);
            edge.walk_time = new_time;
            if (!directed_) {
                adj_[to_id][from_id].walk_time = new_time;
            }
        } else if (field == "status") {
            if (value != "open" && value != "closed") {     //处理参数错误（status 只能是 "open" 或 "closed"）
                throw GraphException("invalid_field: status must be 'open' or 'closed'");
            }
            edge.status = value;
            if (!directed_) {
                adj_[to_id][from_id].status = value;
            }
        } else {
            throw GraphException("invalid_field: " + field);    //处理参数错误（根本不是EdgeNode的字段）
        }
    }

    // 返回边的完整信息
    EdgeNode LGraph::GetEdge(const std::string &from_id, const std::string &to_id) const {
        /*这里最好不要复用exist_edge(from_id, to_id)，因为这样得不到from_id，to_id，相当于复用的函数查找一次，本函数又查找一次，时间翻倍*/
        auto from_it = adj_.find(from_id);
        if (from_it == adj_.end()) {
            throw GraphException("road_not_found: " + from_id + " -> " + to_id);
        }
        auto to_it = from_it->second.find(to_id);
        if (to_it == from_it->second.end()) {
            throw GraphException("road_not_found: " + from_id + " -> " + to_id);
        }
        return to_it->second;
    }

    // ==================== 道路状态 ====================

    // 将边 status 设为 "closed"（复用 UpdateEdge）
    void LGraph::CloseRoad(const std::string &from_id, const std::string &to_id) {
        UpdateEdge(from_id, to_id, "status", "closed");
    }

    // 将边 status 设为 "open"（复用 UpdateEdge）
    void LGraph::OpenRoad(const std::string &from_id, const std::string &to_id) {
        UpdateEdge(from_id, to_id, "status", "open");
    }

    // ==================== 遍历 / 高级查询 ====================

    /* 返回当前图中所有存在的地点 id
     顺序：按 place_id 字典序排序，便于输出和测试*/
    std::vector<std::string> LGraph::AllPlaceIds() const {
        std::vector<std::string> ids;
        ids.reserve(vertices_.size());      // 提前申请好足够内存，push_back 时不会重新分配内存
        for (const auto &kv : vertices_) {
            ids.push_back(kv.first);
        }
        // 排序以保证确定性输出
        std::sort(ids.begin(), ids.end());      //字典序
        return ids;
    }

    // 返回当前图中所有边
    //   - 无向图中每条边只出现一次
    //   - only_open = true 时仅返回 status == "open" 的边
    std::vector<EdgeNode> LGraph::AllEdges(bool only_open) const {
        std::vector<EdgeNode> result;

        for (const auto &from_pair : adj_) {
            const std::string &from_id = from_pair.first;
            for (const auto &to_pair : from_pair.second) {
                const std::string &to_id = to_pair.first;

                if (directed_) {
                    // 有向图：每条边都算
                    if (!only_open || to_pair.second.status == "open") {
                        result.push_back(to_pair.second);
                    }
                } else {
                    // 无向图：只取 from_id < to_id 的边，避免重复
                    if (from_id < to_id) {
                        if (!only_open || to_pair.second.status == "open") {
                            result.push_back(to_pair.second);
                        }
                    }
                }
            }
        }

        return result;
    }

    // 返回某地点的所有邻接边完整信息
    //   place_id 不存在 → GraphException
    std::vector<EdgeNode> LGraph::GetAdjacentEdges(const std::string &place_id) const {
        auto adj_it = adj_.find(place_id);      //adj_it指向adj里的元素，类型是<std::string, std::unordered_map<std::string, EdgeNode>>
        if (adj_it == adj_.end()) {
            throw GraphException("place_not_found: " + place_id);
        }

        std::vector<EdgeNode> result;
        result.reserve(adj_it->second.size());          /*会先默认构造 n 个对象（EdgeNode 有默认构造函数，但毕竟是无用的占位），然后再用赋值覆盖，多了一步浪费的操作。
                                                        而用 reserve + push_back：更高效、语义更清晰：我只是在收集元素，而不是创建一堆占位对象再替换。*/
        for (const auto &neighbor_pair : adj_it->second) {
            result.push_back(neighbor_pair.second);
        }

        return result;
    }

    // 返回某类别下所有地点 id
    // 遍历所有顶点，筛选 category 匹配的
    std::vector<std::string> LGraph::GetPlacesByCategory(const std::string &category) const {
        std::vector<std::string> result;
        for (const auto &kv : vertices_) {
            if (kv.second.category == category) {
                result.push_back(kv.first);
            }
        }
        return result;
    }

}