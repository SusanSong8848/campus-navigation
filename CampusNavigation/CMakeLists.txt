cmake_minimum_required(VERSION 3.16)
project(CampusNavigation)

set(CMAKE_CXX_STANDARD 17)

# MSVC 默认用 GBK 编码，中文注释会引发 C4819 警告和后续语法错误
# 加上 /utf-8 告诉编译器源文件是 UTF-8 编码
if(MSVC)
    add_compile_options("/utf-8")
endif()

add_executable(
        CampusNavigation
        main.cpp
        LGraph.cpp
        LocationInfo.cpp
        Algorithm.cpp
        CsvIO.cpp
        CommandProcessor.cpp
)
