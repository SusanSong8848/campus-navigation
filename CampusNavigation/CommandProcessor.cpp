//
// 动态图上的校园路线规划与设施分析系统
// CommandProcessor.cpp - 命令解析与分发实现（全部 19 个命令）
//

#include "CommandProcessor.h"
#include "Algorithm.h"
#include <iostream>
#include <algorithm>

namespace Graph {

    // ==================== 命令主循环 ====================

    // 解析单条命令字符串，分发到对应的 cmdXxx() 处理函数
    // 返回值：true 继续读取下一条，false 退出程序（收到 QUIT 时）
    // 每个 cmdXxx 内部自行处理参数校验、异常捕获和结果输出
    bool CommandProcessor::ProcessCommand(const std::string &line) {
        std::istringstream iss(line);
        std::string cmd;
        if (!(iss >> cmd)) {                    //iss是命令行的第一个指令
            return true;  // 空行，跳过
        }

        // ---- 调度：按命令名分发到对应处理函数 ----
        if (cmd == "QUIT") {
            return false;
        } else if (cmd == "LOAD") {
            cmdLoad(iss);
        } else if (cmd == "SAVE") {
            cmdSave(iss);
        } else if (cmd == "QUERY_PLACE") {
            cmdQueryPlace(iss);
        } else if (cmd == "QUERY_CATEGORY") {
            cmdQueryCategory(iss);
        } else if (cmd == "ADJ") {
            cmdAdj(iss);
        } else if (cmd == "ADD_PLACE") {
            cmdAddPlace(iss);
        } else if (cmd == "DELETE_PLACE") {
            cmdDeletePlace(iss);
        } else if (cmd == "UPDATE_PLACE") {
            cmdUpdatePlace(iss);
        } else if (cmd == "ADD_ROAD") {
            cmdAddRoad(iss);
        } else if (cmd == "DELETE_ROAD") {
            cmdDeleteRoad(iss);
        } else if (cmd == "UPDATE_ROAD") {
            cmdUpdateRoad(iss);
        } else if (cmd == "CLOSE_ROAD") {
            cmdCloseRoad(iss);
        } else if (cmd == "OPEN_ROAD") {
            cmdOpenRoad(iss);
        } else if (cmd == "COMPONENTS") {
            cmdComponents();
        } else if (cmd == "SHORTEST") {
            cmdShortest(iss);
        } else if (cmd == "TIMED_SHORTEST") {
            cmdTimedShortest(iss);
        } else if (cmd == "MUST_PASS") {
            cmdMustPass(iss);
        } else if (cmd == "MST") {
            cmdMst();
        } else if (cmd == "CRITICAL") {
            cmdCritical();
        } else {
            std::cout << "ERROR unknown_command" << std::endl;
        }

        return true;
    }

    // ==================== LOAD ====================
    // 从两个 CSV 文件中加载地点和道路数据，重建整个图
    // 流程：清空旧图 → 逐条插入顶点 → 逐条插入边
    // 任何一步失败（如文件格式错误、顶点重复）都会输出 ERROR 并中止
    void CommandProcessor::cmdLoad(std::istringstream &args) {
        // LOAD <places.csv> <roads.csv>
        std::string places_path, roads_path;
        if (!(args >> places_path >> roads_path)) {             /*cmdLoad 接收的是 std::istringstream &args，它就是原 iss 的剩余部分。
                                                                因为是引用& 传递，args 的状态（读位置）会改变*/
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }

        // 读取地点数据
        std::vector<LocationInfo> places = CsvIO::ReadPlaces(places_path);
        if (places.empty()) {
            // 文件可能为空或读取失败，但仍尝试继续（可能是空数据集）
        }

        // 读取道路数据
        std::vector<RoadRecord> roads = CsvIO::ReadRoads(roads_path);

        // ------------------ 重建图 ------------------
        // 先清空旧图（直接调用 Clear 比逐个 DeleteVertex 更简洁高效）
        graph.Clear();

        // 依次插入所有地点
        for (const auto &place : places) {
            try {
                graph.InsertVertex(place);
            } catch (const GraphException &e) {
                std::cout << "ERROR " << e.what() << std::endl;
                return;
            }
        }

        // 依次插入所有道路（边）
        for (const auto &road : roads) {
            try {
                graph.InsertEdge(road.from_id, road.to_id,
                                road.distance, road.walk_time, road.status);
            } catch (const GraphException &e) {
                std::cout << "ERROR " << e.what() << std::endl;
                return;
            }
        }

        std::cout << "OK" << std::endl;
    }

    // ==================== SAVE ====================
    // 将当前图中的所有地点和道路数据持久化到两个 CSV 文件
    // 分别调用 CsvIO::WritePlaces 和 CsvIO::WriteRoads
    void CommandProcessor::cmdSave(std::istringstream &args) {
        // SAVE <places_out.csv> <roads_out.csv>
        std::string places_path, roads_path;
        if (!(args >> places_path >> roads_path)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }

        CsvIO::WritePlaces(places_path, graph);
        CsvIO::WriteRoads(roads_path, graph);

        std::cout << "OK" << std::endl;
    }

    // ==================== QUERY_PLACE ====================
    // 按 place_id 查询某个地点的全部信息
    // 输出：PLACE <id> <name> <category> <stay_time> <open_time> <close_time>
    void CommandProcessor::cmdQueryPlace(std::istringstream &args) {
        // QUERY_PLACE <place_id>
        std::string place_id;
        if (!(args >> place_id)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }

        try {
            LocationInfo info = graph.GetVertex(place_id);
            // 按规范格式输出地点全部字段
            std::cout << "PLACE " << info.place_id << " "
                      << info.display_name << " "
                      << info.category << " "
                      << info.stay_time << " "
                      << info.open_time << " "
                      << info.close_time << std::endl;
        } catch (const GraphException &e) {
            // 解析异常消息，提取 reason 关键词
            std::string what(e.what());
            if (what.find("place_not_found") != std::string::npos) {        /*graph.GetVertex 抛出的异常信息是："place_not_found: P0001"（带具体 ID）
                                                                            graph.GetVertex 抛出的异常信息是："place_not_found: P0001"（带具体 ID）*/
                std::cout << "ERROR place_not_found" << std::endl;
            } else {
                std::cout << "ERROR " << what << std::endl;
            }
        }
    }

    // ==================== QUERY_CATEGORY ====================
    // 按类别名查询属于该类别下的所有地点 ID
    // 输出：CATEGORY <category> <count> <id1> <id2> ...（按字典序）
    void CommandProcessor::cmdQueryCategory(std::istringstream &args) {
        // QUERY_CATEGORY <category>
        std::string category;
        if (!(args >> category)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }

        std::vector<std::string> ids = graph.GetPlacesByCategory(category);

        // 按字典序升序排序
        std::sort(ids.begin(), ids.end());          //排序放在cmd函数里再处理很合理：有些时调用的时候不需要排序，而只有cmd里面才硬性要求，如果直接硬性写在被调用函数里面会平白浪费开销

        // 输出：类别名 + 个数 + 逐个 ID
        std::cout << "CATEGORY " << category << " " << ids.size();
        for (const auto &id : ids) {
            std::cout << " " << id;
        }
        std::cout << std::endl;
    }

    // ==================== ADJ ====================
    // 查询某地点的所有邻接边信息（通往哪个地点、距离、步行时间、道路状态）
    // 输出：ADJ <place_id> <edge_count> <to_id>:<dist>:<time>:<status> ...
    void CommandProcessor::cmdAdj(std::istringstream &args) {
        // ADJ <place_id>
        std::string place_id;
        if (!(args >> place_id)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }

        try {
            std::vector<EdgeNode> adj_edges = graph.GetAdjacentEdges(place_id);

            // 按邻接点 place_id 字典序升序排序
            std::sort(adj_edges.begin(), adj_edges.end(),
                      [](const EdgeNode &a, const EdgeNode &b) {
                          return a.to_id < b.to_id;
                      });

            // 输出邻接边数量及每条边的详细信息
            std::cout << "ADJ " << place_id << " " << adj_edges.size();
            for (const auto &edge : adj_edges) {
                std::cout << " " << edge.to_id << ":"
                          << edge.distance << ":"
                          << edge.walk_time << ":"
                          << edge.status;
            }
            std::cout << std::endl;
        } catch (const GraphException &e) {
            std::string what(e.what());
            if (what.find("place_not_found") != std::string::npos) {
                std::cout << "ERROR place_not_found" << std::endl;
            } else {
                std::cout << "ERROR " << what << std::endl;
            }
        }
    }

    // ==================== ADD_PLACE ====================
    // 向图中新增一个地点（顶点）
    // 参数依次为：地点ID 名称 类别 停留时间(min) 开门时间 关门时间
    // 已存在同 ID 地点时输出 ERROR place_already_exists
    void CommandProcessor::cmdAddPlace(std::istringstream &args) {
        std::string place_id, display_name, category, open_time, close_time;
        int stay_time = 0;
        // 从参数流中依次读取 6 个字段
        if (!(args >> place_id >> display_name >> category >> stay_time >> open_time >> close_time)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }
        try {
            // 组装 LocationInfo 对象并插入图
            LocationInfo info(place_id, display_name, category, stay_time, open_time, close_time);
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
    }

    // ==================== DELETE_PLACE ====================
    // 删除指定地点及其所有关联边（级联删除）
    // 地点不存在时输出 ERROR place_not_found
    void CommandProcessor::cmdDeletePlace(std::istringstream &args) {
        std::string place_id;
        if (!(args >> place_id)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }
        try {
            // DeleteVertex 内部会级联删除所有邻接边
            graph.DeleteVertex(place_id);
            std::cout << "OK" << std::endl;
        } catch (const GraphException &e) {
            std::string what(e.what());
            if (what.find("place_not_found") != std::string::npos) {
                std::cout << "ERROR place_not_found" << std::endl;
            } else {
                std::cout << "ERROR " << what << std::endl;
            }
        }
    }

    // ==================== UPDATE_PLACE ====================
    // 更新指定地点的某个属性字段
    // 支持的 field：display_name / category / stay_time / open_time / close_time
    void CommandProcessor::cmdUpdatePlace(std::istringstream &args) {
        std::string place_id, field, value;
        if (!(args >> place_id >> field >> value)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }
        try {
            graph.UpdateVertex(place_id, field, value);
            std::cout << "OK" << std::endl;
        } catch (const GraphException &e) {
            std::string what(e.what());
            if (what.find("place_not_found") != std::string::npos) {
                std::cout << "ERROR place_not_found" << std::endl;
            } else if (what.find("invalid_field") != std::string::npos) {
                std::cout << "ERROR invalid_field" << std::endl;
            } else {
                std::cout << "ERROR " << what << std::endl;
            }
        }
    }

    // ==================== ADD_ROAD ====================
    // 在两个地点之间新增一条道路（边）
    // 无向图中自动在反向也创建一条等权边
    // 可能错误：起点/终点不存在 → place_not_found；边已存在 → road_already_exists
    void CommandProcessor::cmdAddRoad(std::istringstream &args) {
        std::string from_id, to_id, status;
        int distance = 0, walk_time = 0;
        if (!(args >> from_id >> to_id >> distance >> walk_time >> status)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }
        try {
            graph.InsertEdge(from_id, to_id, distance, walk_time, status);
            std::cout << "OK" << std::endl;
        } catch (const GraphException &e) {
            std::string what(e.what());
            if (what.find("place_not_found") != std::string::npos) {        //找到了what信息里面有"place_not_found"
                std::cout << "ERROR place_not_found" << std::endl;
            } else if (what.find("road_already_exists") != std::string::npos) {
                std::cout << "ERROR road_already_exists" << std::endl;
            } else {
                std::cout << "ERROR " << what << std::endl;
            }
        }
    }

    // ==================== DELETE_ROAD ====================
    // 删除两个地点之间的一条道路（边）
    // 无向图中自动同步删除反向边
    // 边不存在时输出 ERROR road_not_found
    void CommandProcessor::cmdDeleteRoad(std::istringstream &args) {
        std::string from_id, to_id;
        if (!(args >> from_id >> to_id)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }
        try {
            graph.DeleteEdge(from_id, to_id);
            std::cout << "OK" << std::endl;
        } catch (const GraphException &e) {
            std::string what(e.what());
            if (what.find("road_not_found") != std::string::npos) {
                std::cout << "ERROR road_not_found" << std::endl;
            } else {
                std::cout << "ERROR " << what << std::endl;
            }
        }
    }

    // ==================== UPDATE_ROAD ====================
    // 更新一条道路的某个属性（距离 / 步行时间 / 状态）
    // 无向图中自动同步更新反向边的同一属性
    // 支持的 field：distance, walk_time, status
    void CommandProcessor::cmdUpdateRoad(std::istringstream &args) {
        std::string from_id, to_id, field, value;
        if (!(args >> from_id >> to_id >> field >> value)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }
        try {
            graph.UpdateEdge(from_id, to_id, field, value);
            std::cout << "OK" << std::endl;
        } catch (const GraphException &e) {
            std::string what(e.what());
            if (what.find("road_not_found") != std::string::npos) {
                std::cout << "ERROR road_not_found" << std::endl;
            } else if (what.find("invalid_field") != std::string::npos) {
                std::cout << "ERROR invalid_field" << std::endl;
            } else {
                std::cout << "ERROR " << what << std::endl;
            }
        }
    }

    // ==================== CLOSE_ROAD ====================
    // 封闭一条道路（status = "closed"），使其在路径规划中不可用
    // 实际上是对 UPDATE_ROAD status closed 的快捷封装
    void CommandProcessor::cmdCloseRoad(std::istringstream &args) {
        std::string from_id, to_id;
        if (!(args >> from_id >> to_id)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }
        try {
            graph.CloseRoad(from_id, to_id);        //CommandProcessor::cmdCloseRoad -> LGraph::CloseRoad -> LGraph::UpdateEdge
            std::cout << "OK" << std::endl;
        } catch (const GraphException &e) {
            std::cout << "ERROR road_not_found" << std::endl;
        }
    }

    // ==================== OPEN_ROAD ====================
    // 开放一条道路（status = "open"），使其恢复可通行
    // 实际上是对 UPDATE_ROAD status open 的快捷封装
    void CommandProcessor::cmdOpenRoad(std::istringstream &args) {
        std::string from_id, to_id;
        if (!(args >> from_id >> to_id)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }
        try {
            graph.OpenRoad(from_id, to_id);
            std::cout << "OK" << std::endl;
        } catch (const GraphException &e) {
            std::cout << "ERROR road_not_found" << std::endl;
        }
    }

    // ==================== COMPONENTS ====================
    // 计算图的连通分量（连通块）
    // 调用 Algorithm::GetConnectedComponents，输出分量数和各分量大小（降序）
    // 输出格式：COMPONENTS <count> SIZES <s1> <s2> ...（降序）
    void CommandProcessor::cmdComponents() {
        auto result = Algorithm::GetConnectedComponents(graph);
        std::cout << "COMPONENTS " << result.count << " SIZES";
        for (int size : result.sizes) {
            std::cout << " " << size;
        }
        std::cout << std::endl;
    }

    // ==================== SHORTEST ====================
    // 基本最短路径查询（Dijkstra 算法）
    // mode 可以是 DIST（最短距离）或 TIME（最短时间）
    // 不可达时输出 NO_PATH
    // 输出格式：PATH <DIST|TIME> <total_cost> NODES <id1> <id2> ...
    void CommandProcessor::cmdShortest(std::istringstream &args) {
        std::string from_id, to_id, mode_str;
        if (!(args >> from_id >> to_id >> mode_str)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }
        // 校验起止地点是否存在
        if (!graph.exist_vertex(from_id) || !graph.exist_vertex(to_id)) {
            std::cout << "ERROR place_not_found" << std::endl;
            return;
        }
        // 解析路径模式：DIST 按距离，TIME 按时间
        Algorithm::PathMode mode = (mode_str == "DIST") ? Algorithm::PathMode::DIST : Algorithm::PathMode::TIME;
        Algorithm::PathResult result = Algorithm::GetShortestPath(graph, from_id, to_id, mode);     //调用
        if (!result.reachable) { std::cout << "NO_PATH" << std::endl; return; }
        // 输出路径结果
        std::cout << "PATH " << mode_str << " " << result.total_cost << " NODES";
        for (const auto &id : result.path) std::cout << " " << id;
        std::cout << std::endl;
    }

    // ==================== TIMED_SHORTEST ====================
    // 带时刻约束的最短路径：在指定时刻 time_str 下，只走当时开放的地点
    // 即只有 open_time <= time_str <= close_time 的地点才能被经过
    // 输出格式同 SHORTEST
    void CommandProcessor::cmdTimedShortest(std::istringstream &args) {
        std::string from_id, to_id, time_str, mode_str;
        if (!(args >> from_id >> to_id >> time_str >> mode_str)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }
        if (!graph.exist_vertex(from_id) || !graph.exist_vertex(to_id)) {
            std::cout << "ERROR place_not_found" << std::endl;
            return;
        }
        Algorithm::PathMode mode = (mode_str == "DIST") ? Algorithm::PathMode::DIST : Algorithm::PathMode::TIME;
        // 调用带时间约束的最短路径算法
        Algorithm::PathResult result = Algorithm::GetTimedShortestPath(graph, from_id, to_id, time_str, mode);
        if (!result.reachable) { std::cout << "NO_PATH" << std::endl; return; }
        std::cout << "PATH " << mode_str << " " << result.total_cost << " NODES";
        for (const auto &id : result.path) std::cout << " " << id;
        std::cout << std::endl;
    }

    // ==================== MUST_PASS ====================
    // 必经点路径：从起点出发，必须按序经过 k 个指定地点，最终到达终点
    // 实现方式是将路径拆分为 k+1 段，每段用 Dijkstra 拼接
    // 输出格式同 SHORTEST，不可达输出 NO_PATH
    void CommandProcessor::cmdMustPass(std::istringstream &args) {
        std::string from_id, to_id, mode_str;
        int k = 0;
        if (!(args >> from_id >> to_id >> mode_str >> k)) {
            std::cout << "ERROR invalid_arguments" << std::endl;
            return;
        }
        if (!graph.exist_vertex(from_id) || !graph.exist_vertex(to_id)) {
            std::cout << "ERROR place_not_found" << std::endl;
            return;
        }
        Algorithm::PathMode mode = (mode_str == "DIST") ? Algorithm::PathMode::DIST : Algorithm::PathMode::TIME;

        // 读取 k 个必经点 ID，逐个校验存在性
        std::vector<std::string> waypoints;
        for (int i = 0; i < k; ++i) {
            std::string wp;
            if (!(args >> wp)) { std::cout << "ERROR invalid_arguments" << std::endl; return; }
            if (!graph.exist_vertex(wp)) { std::cout << "ERROR place_not_found" << std::endl; return; }
            waypoints.push_back(wp);
        }

        // 调用必经点路径算法
        Algorithm::PathResult result = Algorithm::GetMustPassPath(graph, from_id, to_id, mode, waypoints);
        if (!result.reachable) { std::cout << "NO_PATH" << std::endl; return; }
        std::cout << "PATH " << mode_str << " " << result.total_cost << " NODES";
        for (const auto &id : result.path) std::cout << " " << id;
        std::cout << std::endl;
    }

    // ==================== MST ====================
    // 最小生成树（Minimum Spanning Tree，Prim/Kruskal）
    // 图不连通时输出 DISCONNECTED
    // 输出格式：MST <total_distance> EDGES <u1>-<v1>:<w1> <u2>-<v2>:<w2> ...
    // 边的输出顺序：先按 (min(u,v), max(u,v)) 的字典序排序
    void CommandProcessor::cmdMst() {
        std::vector<EdgeNode> mst = Algorithm::MinimumSpanningTree(graph);
        // 图为森林（多个连通分量）时无法生成单棵生成树
        if (mst.empty()) { std::cout << "DISCONNECTED" << std::endl; return; }

        // 计算 MST 总距离
        int total = 0;
        for (const auto &e : mst) total += e.distance;

        // 按 (min(u,v), max(u,v)) 字典序排序
        std::sort(mst.begin(), mst.end(), [](const EdgeNode &a, const EdgeNode &b) {
            std::string ua = std::min(a.from_id, a.to_id), va = std::max(a.from_id, a.to_id);
            std::string ub = std::min(b.from_id, b.to_id), vb = std::max(b.from_id, b.to_id);
            if (ua != ub) return ua < ub;
            return va < vb;
        });

        // 输出 MST 结果
        std::cout << "MST " << total << " EDGES";
        for (const auto &e : mst) {
            std::string u = std::min(e.from_id, e.to_id), v = std::max(e.from_id, e.to_id);
            std::cout << " " << u << "-" << v << ":" << e.distance;
        }
        std::cout << std::endl;
    }

    // ==================== CRITICAL ====================
    // 关节点与关键边分析（割点/桥，基于 Tarjan 算法）
    // 关节点：删除后图不再连通的顶点
    // 关键边（桥）：删除后图不再连通的边
    // 输出格式：CRITICAL NODES <count> <id1> ... EDGES <count> <u1>-<v1> ...
    // 节点按字典序，边按 (min(u,v), max(u,v)) 字典序
    void CommandProcessor::cmdCritical() {
        Algorithm::CriticalResult cr = Algorithm::FindCriticalNodesAndEdges(graph);

        // 关节点按字典序排序
        std::sort(cr.critical_nodes.begin(), cr.critical_nodes.end());
        std::cout << "CRITICAL NODES " << cr.critical_nodes.size();
        for (const auto &n : cr.critical_nodes) std::cout << " " << n;

        // 关键边统一规范化为 (min, max) 并按字典序排序
        std::vector<std::pair<std::string, std::string>> edges_sorted = cr.critical_edges;
        for (auto &p : edges_sorted)
            if (p.first > p.second) std::swap(p.first, p.second);
        std::sort(edges_sorted.begin(), edges_sorted.end());

        std::cout << " EDGES " << edges_sorted.size();
        for (const auto &p : edges_sorted) std::cout << " " << p.first << "-" << p.second;
        std::cout << std::endl;
    }

} // namespace Graph