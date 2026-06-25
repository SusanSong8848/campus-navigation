# 微型数据集说明：某商场购物中心

## 数据来源

自行构造。模拟一个典型双层购物中心的平面拓扑。

## 为什么这样建模

购物中心是一个高度结构化的室内环境：出入口固定、人流沿固定通道流动、不同店铺有不同的开放时间。这些特征天然适合用图来建模：

- 店铺 = 顶点（含类别、营业时间）
- 人行通道/扶梯 = 边（含步行距离和时间）
- 部分通道在非营业时段关闭 = `status = "closed"`

## 数据概览

| 项目 | 值 |
|------|-----|
| 地点数 | 16 |
| 道路数 | 25（23 open + 2 closed） |
| 类别 | Entrance, Parking, Catering, Retail, Entertainment, Service, Facility |

## 关键特点与检验目标

### 1. 混合时间窗

不同店铺营业时间不同（如 Starbucks 7:00 开、Haidilao 10:00 开、Cinema 23:30 关），可用于验证 `TIMED_SHORTEST` 在不同时段路径的正确性。

### 2. 封闭道路的影响

两条 closed 道路：
- `M004-M001`（地下停车场 → 南门，closed）——模拟停车场维修时南门不可用
- `M007-M005`（美食广场 → Starbucks，closed）——模拟消防通道临时封闭

如 `DELETE_ROAD` 删除这两条边，或 `CLOSE_ROAD` 关闭它们，应影响连通性。关键验证：`COMPONENTS` 在封闭前后是否正确变化。

### 3. 核心节点与冗余路径

- M010（Cinema）是连接一楼和二楼的关键节点，删除后 KFC 和二楼卫生间将孤立
- M007-M005 的封闭创造了一个"绕路"场景：从 Food Court 到 Starbucks 需要绕行 Supermarket

### 4. 可测试的命令

| 命令 | 示例 |
|------|------|
| SHORTEST | `SHORTEST M001 M010 DIST`（应找到南门→Starbucks→Haidilao→Cinema这样的路径） |
| TIMED_SHORTEST | `TIMED_SHORTEST M001 M010 09:00 DIST`（Starbucks已开、Haidilao未开） |
| MUST_PASS | `MUST_PASS M001 M011 DIST 2 M005 M010` |
| COMPONENTS | 加载后应输出连通分量数（封闭边会影响分量数） |
| CRITICAL | M010（Cinema）应是关键节点 |