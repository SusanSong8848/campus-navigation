//
// 动态图上的校园路线规划与设施分析系统
// CsvIO.cpp - CSV 文件读写实现
//

#include "CsvIO.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

//说明：助教模版里给的"(void)path; 和 (void)graph..."：这是显式标记"这个参数我暂时不用"，避免编译器警告。
namespace Graph {
    namespace CsvIO {

        // 辅助函数：将字符串中的逗号替换为空格，便于用 istringstream 解析
        static std::string commasToSpaces(const std::string &line) {        //comma  /'o a/：逗号
            std::string result = line;
            for (char &c : result) {
                if (c == ',') c = ' ';
            }
            return result;
        }

    
        // 辅助函数：去掉字符串首尾空白（处理 CSV 可能的空格干扰）
        //这个方法我不知到调用那个函数，就让ai写的，就是那个s.find_first_not_of()函数
        static std::string trim(const std::string &s) {
            size_t start = s.find_first_not_of(" \t\r\n");      //return：Index of first occurrence.找到第一个不是空白符的地方
            if (start == std::string::npos) return "";          /*std::string::npos 是一个特殊常量，意思是"没找到"。
                                                                如果整行全是空格/制表符/回车，就找不到任何非空白字符，返回 npos*/
            size_t end = s.find_last_not_of(" \t\r\n");
            return s.substr(start, end - start + 1);        //剪切
        }


        std::vector<LocationInfo> ReadPlaces(const std::string &path) {     // path是文件名(.csv)
            std::vector<LocationInfo> places;

            // 1. 打开文件
            std::ifstream file(path);
            if (!file.is_open()) {      //这一步不能少，报错处理
                std::cerr << "CsvIO::ReadPlaces: 无法打开文件 " << path << std::endl;       //写明是CsvIO::ReadPlaces会更清楚
                return {};
            }

            std::string line;
            bool first_line = true;      //这个first_line是为了判断是不是表头行（因为表头行肯定是第一行），所以如果不是第一行，那后面的循环也可以不用检查了

            // 2. 逐行读取
            while (std::getline(file, line)) {      //file是ifstream的对象，相当于有很多行string，getline(file, line)就是挨行读取到line
                                                    /*file 本质是操作系统层面的一个文件句柄（读指针 + 缓冲区）
                                                    std::ifstream 的构造函数内部调用了操作系统的 open() 系统调用，把磁盘文件关联到这个对象上*/
                // 跳过空行
                if (line.empty()) continue;
                // 跳过纯空白行
                std::string trimmed = trim(line);       //这个方法我不知到调用那个函数，就让ai写的，就是那个s.find_first_not_of()函数
                if (trimmed.empty()) continue;

                // 将逗号替换为空格，便于用 >> 解析（对 trimmed 操作，保证干净输入）
                std::string parsed = commasToSpaces(trimmed);
                std::istringstream iss(parsed);     //istringstream 是 C++ 标准库里的字符串输入流类。
                                                    //通俗理解：把一坨字符串当成"内存中的文件"，然后像从键盘或文件读取一样逐个提取

                std::string place_id, display_name, category;
                int stay_time = 0;
                std::string open_time, close_time;

                // 尝试解析6个字段
                if (!(iss >> place_id)) continue;

                // 3. 检查是否为表头行（首行以 place_id 开头则跳过）
                if (first_line && place_id == "place_id") {     //这个first_line是为了判断是不是表头行（因为表头行肯定是第一行），所以如果不是第一行，那后面的循环也可以不用检查了
                    first_line = false;
                    continue;
                }
                first_line = false;

                // 继续解析剩余字段
                if (!(iss >> display_name)) continue;       //这个continue是ai帮我加的。这是在处理坏行(就是有残缺的行)，就不构造place了。
                if (!(iss >> category)) continue;
                if (!(iss >> stay_time)) continue;
                if (!(iss >> open_time)) continue;
                if (!(iss >> close_time)) continue;

                // 构建 LocationInfo 并加入结果
                places.emplace_back(place_id, display_name, category,
                                   stay_time, open_time, close_time);       //只有当所有字段解析成功才能places.emplace_back
            }

            file.close();       //别忘了
            return places;
        }


        std::vector<RoadRecord> ReadRoads(const std::string &path) {
            std::vector<RoadRecord> roads;

            // 1. 打开文件
            std::ifstream file(path);
            if (!file.is_open()) {
                std::cerr << "CsvIO::ReadRoads: 无法打开文件 " << path << std::endl;
                return {};
            }

            std::string line;
            bool first_line = true;

            // 2. 逐行读取
            while (std::getline(file, line)) {
                // 跳过空行
                if (line.empty()) continue;
                std::string trimmed = trim(line);   //去掉字符串首尾空白（处理 CSV 可能的空格干扰）
                if (trimmed.empty()) continue;

                // 将逗号替换为空格（对 trimmed 操作，保证干净输入）
                std::string parsed = commasToSpaces(trimmed);
                std::istringstream iss(parsed);

                std::string from_id, to_id, status;
                int distance = 0, walk_time = 0;

            // 尝试解析5个字段
                if (!(iss >> from_id)) continue;

                // 3. 检查是否为表头行（首行以 from_id 开头则跳过）
                if (first_line && from_id == "from_id") {
                    first_line = false;
                    continue;
                }
                first_line = false;

                // 继续解析
                if (!(iss >> to_id)) continue;
                if (!(iss >> distance)) continue;
                if (!(iss >> walk_time)) continue;
                if (!(iss >> status)) continue;

                // 构建 RoadRecord 并加入结果
                roads.push_back({from_id, to_id, distance, walk_time, status});     //只有当所有字段解析成功才能roads.push_back
            }

            file.close();
            return roads;
        }


        void WritePlaces(const std::string &path, const LGraph &graph) {
            // 1. 打开文件输出流
            std::ofstream file(path);
            if (!file.is_open()) {
                std::cerr << "CsvIO::WritePlaces: 无法写入文件 " << path << std::endl;
                return;
            }

            // 2. 写入表头行
            file << "place_id,display_name,category,stay_time,open_time,close_time\n";

            // 3. 遍历所有地点 id（已排序），逐个写出
            std::vector<std::string> all_ids = graph.AllPlaceIds();     //这个函数是有字典序排序的
            for (const auto &id : all_ids) {
                LocationInfo info = graph.GetVertex(id);
                file << info.place_id << ","
                     << info.display_name << ","
                     << info.category << ","
                     << info.stay_time << ","
                     << info.open_time << ","
                     << info.close_time << "\n";
            }

            // 4. 关闭文件（ofstream 析构时自动关闭，那我其实可以不写）
            file.close();
        }


        void WriteRoads(const std::string &path, const LGraph &graph) {
            // 1. 打开文件输出流
            std::ofstream file(path);
            if (!file.is_open()) {
                std::cerr << "CsvIO::WriteRoads: 无法写入文件 " << path << std::endl;
                return;
            }

            // 2. 写入表头行
            file << "from_id,to_id,distance,walk_time,status\n";

            // 3. 获取所有边（包括 closed）
            std::vector<EdgeNode> all_edges = graph.AllEdges(false);

            // 4. 输出每条边
            for (const auto &edge : all_edges) {
                file << edge.from_id << ","
                     << edge.to_id << ","
                     << edge.distance << ","
                     << edge.walk_time << ","
                     << edge.status << "\n";
            }

            // 5. 关闭文件
            file.close();
        }
    }
}