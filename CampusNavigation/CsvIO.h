//
// 动态图上的校园路线规划与设施分析系统
// CsvIO.h - CSV 文件读写模块
//

#ifndef CAMPUSNAVIGATION_CSVIO_H
#define CAMPUSNAVIGATION_CSVIO_H

#include <string>
#include <vector>
#include "LocationInfo.h"
#include "LGraph.h"

namespace Graph {

    /*RoadRecord：只是 CSV 读取时的临时载体，读完后立即转换成 EdgeNode 插入图，其他地方完全不需要它，所以放在 CsvIO 内部作为局部实现细节。*/
    // 道路原始数据（从 CSV 读入时使用）
    struct RoadRecord {
        std::string from_id;
        std::string to_id;
        int distance;
        int walk_time;
        std::string status;
    };

//（说明）：CSV = Comma-Separated Values（逗号分隔值）。
//.csv 是一种纯文本文件格式：每行一条记录，字段之间用逗号分隔，类似简易表格，几乎所有数据处理软件都能读写。

    namespace CsvIO {
        // 从 CSV 文件读取地点信息
        // 兼容带表头（首行以 place_id 开头则跳过）和不带表头两种格式
        // 格式：place_id,display_name,category,stay_time,open_time,close_time
        std::vector<LocationInfo> ReadPlaces(const std::string &path);

        // 从 CSV 文件读取道路信息
        // 兼容带表头（首行以 from_id 开头则跳过）和不带表头两种格式
        // 格式：from_id,to_id,distance,walk_time,status
        std::vector<RoadRecord> ReadRoads(const std::string &path);

        // 将当前图的地点信息写回 CSV（带表头）
        void WritePlaces(const std::string &path, const LGraph &graph);

        // 将当前图的道路信息写回 CSV（带表头）
        void WriteRoads(const std::string &path, const LGraph &graph);
    }
}

#endif //CAMPUSNAVIGATION_CSVIO_H
