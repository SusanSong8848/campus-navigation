//
// 动态图上的校园路线规划与设施分析系统
// LocationInfo.h - 地点信息结构体
//

#ifndef CAMPUSNAVIGATION_LOCATIONINFO_H
#define CAMPUSNAVIGATION_LOCATIONINFO_H

#include <string>
#include <utility>

namespace Graph {
    struct LocationInfo {                                   //Info就是Information的意思
        std::string place_id;       // 唯一ID，如 P0001
        std::string display_name;   // 展示名称
        std::string category;       // 类别，如 Teaching, Dining, Dormitory, Sports, Medical, Other
        int stay_time{0};           // 建议停留时间（单位：分钟）
        std::string open_time;      // 开放时间，如 08:00
        std::string close_time;     // 关闭时间，如 22:00
        
        LocationInfo() = default;       //这是显式要求编译器生成默认构造函数。        //作用：告诉编译器“用你自动生成的那个无参构造函数就行，别因为我写了其他构造函数就不生成了”。

        LocationInfo(std::string place_id_, std::string display_name_,
                     std::string category_, int stay_time_,
                     std::string open_time_, std::string close_time_)
                : place_id(std::move(place_id_)),
                  display_name(std::move(display_name_)),
                  category(std::move(category_)),
                  stay_time(stay_time_),
                  open_time(std::move(open_time_)),
                  close_time(std::move(close_time_)) {}
    };
} // Graph

#endif //CAMPUSNAVIGATION_LOCATIONINFO_H
