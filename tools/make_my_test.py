#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
make_my_test.py —— 命令行驱动的测试数据生成器

【一句话用法】
    python3 make_my_test.py <类型> [参数...] [seed]

【常用例子】
    python3 make_my_test.py grid 5 5         # 5x5 网格，每次跑数据不同
    python3 make_my_test.py chain 20         # 20 节点链，每次不同
    python3 make_my_test.py cycle 10         # 10 节点环
    python3 make_my_test.py star 8           # 1 中心 + 8 叶子
    python3 make_my_test.py random 50 80     # 50 节点 80 边随机图

【高级例子】
    python3 make_my_test.py example basic    # 预设的微型校园（数据固定、有手算注释）
    python3 make_my_test.py example timed    # 预设的时刻约束测试
    python3 make_my_test.py example modify   # 预设的增删改测试

所有数据保存到 my_test/。每次跑会覆盖——想保留就改名。
"""

import os
import sys
import math
import random


# ============================================================
# 随机化的属性池
# ============================================================

CATEGORIES = ['Teaching', 'Dining', 'Dormitory', 'Sports', 'Medical', 'Other']

# 营业时间窗：含权重——大多数地点全天开放，少数有时段限制
TIME_WINDOWS = [
    ('00:00', '23:59'),     # 全天（高频）
    ('00:00', '23:59'),     # 全天（再来一份提升权重）
    ('00:00', '23:59'),     # 全天
    ('06:00', '20:00'),     # 白天
    ('08:00', '22:00'),     # 主要营业
    ('18:00', '23:59'),     # 夜间
    ('06:00', '12:00'),     # 上午
]


def _rand_category():
    return random.choice(CATEGORIES)


def _rand_stay():
    return random.randint(0, 60)


def _rand_hours():
    return random.choice(TIME_WINDOWS)


def _rand_distance(dist_range=(50, 300)):
    return random.randint(*dist_range)


# ============================================================
# 核心：TestBuilder
# ============================================================

class TestBuilder:
    """造测试数据的链式构造器。

    构造时可以打开 randomize 开关——之后所有未显式给出属性的 place/edge
    都会自动随机化。例如：

        TestBuilder(randomize=True).grid(5, 5).cmd("...").save("my_test/")

    显式传的参数始终优先：

        TestBuilder(randomize=True).place("Dorm", "Dormitory")
        # category 用了显式的 "Dormitory"，但 stay、hours 仍随机
    """

    def __init__(self, randomize=False):
        self._places = []
        self._roads = []
        self._commands = []
        self._name_to_id = {}
        self._edge_set = set()
        self._next_id = 1
        self.randomize = randomize

    # ---------- 加点 ----------

    def place(self, name, category=None, stay=None, hours=None):
        """加一个地点。

        参数不传时：
          - randomize=True 走随机池
          - randomize=False 用保守默认值（Other / 0 / 全天）
        """
        if name in self._name_to_id:
            raise ValueError(f'地点名重复：{name}')
        if category is None:
            category = _rand_category() if self.randomize else "Other"
        if stay is None:
            stay = _rand_stay() if self.randomize else 0
        if hours is None:
            hours = _rand_hours() if self.randomize else ("00:00", "23:59")

        pid = f'P{self._next_id:04d}'
        self._next_id += 1
        self._name_to_id[name] = pid
        self._places.append((pid, name, category, stay, hours[0], hours[1]))
        return self

    def places(self, names, category=None):
        for n in names:
            self.place(n, category)
        return self

    # ---------- 加边 ----------

    def edge(self, a, b, distance=None, walk_time=None, status="open"):
        if distance is None:
            distance = _rand_distance() if self.randomize else 100
        if walk_time is None:
            walk_time = max(1, math.ceil(distance / 80))
        id_a = self._resolve(a)
        id_b = self._resolve(b)
        key = tuple(sorted([id_a, id_b]))
        if key in self._edge_set:
            return self
        self._edge_set.add(key)
        self._roads.append((id_a, id_b, distance, walk_time, status))
        return self

    def close(self, a, b, distance=None, walk_time=None):
        return self.edge(a, b, distance, walk_time, status="closed")

    # ---------- 拓扑工厂（distance=None 时按 randomize 开关决定） ----------

    def chain(self, names, category=None, distance=None):
        self.places(names, category)
        for i in range(len(names) - 1):
            self.edge(names[i], names[i + 1], distance)
        return self

    def cycle(self, names, category=None, distance=None):
        self.chain(names, category, distance)
        if len(names) >= 3:
            self.edge(names[-1], names[0], distance)
        return self

    def star(self, center, leaves, category=None, distance=None):
        self.place(center, category)
        self.places(leaves, category)
        for leaf in leaves:
            self.edge(center, leaf, distance)
        return self

    def grid(self, rows, cols, name_prefix="G", category=None, distance=None):
        for r in range(rows):
            for c in range(cols):
                self.place(f'{name_prefix}_{r}_{c}', category)
        for r in range(rows):
            for c in range(cols):
                if c + 1 < cols:
                    self.edge(f'{name_prefix}_{r}_{c}',
                              f'{name_prefix}_{r}_{c + 1}', distance)
                if r + 1 < rows:
                    self.edge(f'{name_prefix}_{r}_{c}',
                              f'{name_prefix}_{r + 1}_{c}', distance)
        return self

    def random_graph(self, n, m, closed_ratio=0.0,
                     name_prefix="N", dist_range=(50, 300)):
        """随机连通图：先生成树（链状）再加额外边。"""
        names = [f'{name_prefix}{i:04d}' for i in range(1, n + 1)]
        self.places(names)

        perm = list(names)
        random.shuffle(perm)
        for i in range(len(perm) - 1):
            d = random.randint(*dist_range)
            s = 'closed' if random.random() < closed_ratio else 'open'
            self.edge(perm[i], perm[i + 1], d, status=s)

        attempts = 0
        target = max(m, n - 1)
        while len(self._edge_set) < target and attempts < m * 20:
            attempts += 1
            a, b = random.sample(names, 2)
            d = random.randint(*dist_range)
            s = 'closed' if random.random() < closed_ratio else 'open'
            self.edge(a, b, d, status=s)
        return self

    # ---------- 命令 ----------

    def cmd(self, command):
        self._commands.append(command)
        return self

    def cmds(self, *commands):
        for c in commands:
            self.cmd(c)
        return self

    # ---------- 内部 ----------

    def _resolve(self, name_or_id):
        return self._name_to_id.get(name_or_id, name_or_id)

    def _resolve_command(self, cmd_line):
        return ' '.join(self._name_to_id.get(t, t) for t in cmd_line.split())

    # ---------- 保存 ----------

    def save(self, out_dir):
        os.makedirs(out_dir, exist_ok=True)

        with open(os.path.join(out_dir, 'places.csv'), 'w', encoding='utf-8') as f:
            f.write('place_id,display_name,category,stay_time,open_time,close_time\n')
            for p in self._places:
                f.write(','.join(str(x) for x in p) + '\n')

        with open(os.path.join(out_dir, 'roads.csv'), 'w', encoding='utf-8') as f:
            f.write('from_id,to_id,distance,walk_time,status\n')
            for r in self._roads:
                f.write(','.join(str(x) for x in r) + '\n')

        with open(os.path.join(out_dir, 'command.txt'), 'w', encoding='utf-8') as f:
            for c in self._commands:
                f.write(self._resolve_command(c) + '\n')

        print(f'  -> {out_dir}/places.csv   ({len(self._places)} places)')
        print(f'  -> {out_dir}/roads.csv    ({len(self._roads)} roads)')
        print(f'  -> {out_dir}/command.txt  ({len(self._commands)} commands)')
        return self


# ============================================================
# 默认命令模板
# ============================================================

def default_commands(start_name, end_name):
    return [
        'LOAD places.csv roads.csv',
        'COMPONENTS',
        f'SHORTEST {start_name} {end_name} DIST',
        f'SHORTEST {start_name} {end_name} TIME',
        'MST',
        'CRITICAL',
        'QUIT',
    ]


# ============================================================
# 预设示例（固定数据，不随 seed 变化——供 example 模式调用）
# ============================================================

def example_basic():
    """5 个地点的微型校园，数据完全固定，有手算练习。"""
    return (
        TestBuilder()
            .place("Dorm",       "Dormitory")
            .place("TeachBldg",  "Teaching",  stay=60, hours=("08:00", "22:00"))
            .place("Cafeteria",  "Dining",    stay=20, hours=("06:30", "20:30"))
            .place("Library",    "Other",     stay=90, hours=("08:00", "22:00"))
            .place("Sportsfield","Sports",    stay=45, hours=("06:00", "21:00"))
            .edge("Dorm",      "TeachBldg",   300)
            .edge("Dorm",      "Cafeteria",   200)
            .edge("TeachBldg", "Library",     150)
            .edge("Cafeteria", "Library",     250)
            .edge("Library",   "Sportsfield", 400)
            .close("Dorm",     "Sportsfield", 600)
            .cmds(
                "LOAD places.csv roads.csv",
                "QUERY_PLACE Dorm",
                "QUERY_PLACE Cafeteria",
                "SHORTEST Dorm Sportsfield DIST",
                "SHORTEST Dorm Sportsfield TIME",
                "QUIT",
            )
    )


def example_timed():
    return (
        TestBuilder()
            .place("Home",       "Dormitory")
            .place("Cafeteria",  "Dining", stay=20, hours=("06:00", "20:00"))
            .place("NightSnack", "Other",  stay=30, hours=("18:00", "23:59"))
            .place("TeachBldg",  "Teaching")
            .edge("Home",       "Cafeteria",  100)
            .edge("Cafeteria",  "TeachBldg",  100)
            .edge("Home",       "NightSnack", 100)
            .edge("NightSnack", "TeachBldg",  100)
            .cmds(
                "LOAD places.csv roads.csv",
                "TIMED_SHORTEST Home TeachBldg 10:00 DIST",
                "TIMED_SHORTEST Home TeachBldg 22:00 DIST",
                "TIMED_SHORTEST Home TeachBldg 02:00 DIST",
                "QUIT",
            )
    )


def example_modify():
    return (
        TestBuilder()
            .cycle(["A", "B", "C"], distance=100)
            .cmds(
                "LOAD places.csv roads.csv",
                "COMPONENTS",
                "ADD_PLACE P0004 D Other 0 00:00 23:59",
                "COMPONENTS",
                "ADD_ROAD C P0004 50 1 open",
                "COMPONENTS",
                "CLOSE_ROAD C P0004",
                "COMPONENTS",
                "QUIT",
            )
    )


EXAMPLES = {
    'basic':  example_basic,
    'timed':  example_timed,
    'modify': example_modify,
}


# ============================================================
# 命令行入口
# ============================================================

USAGE = """用法：
    python3 make_my_test.py <类型> [参数...] [seed]

类型：
    grid <rows> <cols> [seed]   rows x cols 网格
    chain <n> [seed]            n 节点链
    cycle <n> [seed]            n 节点环
    star <leaves> [seed]        1 中心 + leaves 叶子
    random <n> <m> [seed]       n 节点 m 边随机连通图
    example <名字>              预设示例：basic / timed / modify（数据固定）

不带 seed：每次跑数据都不一样（distance / category / stay / 时间窗都随机）
带 seed：每次跑完全相同——同样的 seed 永远生成同一份数据

例子：
    python3 make_my_test.py grid 5 5       # 每次跑都不同
    python3 make_my_test.py grid 5 5 42    # 跑两次完全一样
    python3 make_my_test.py random 50 80
    python3 make_my_test.py example basic

输出目录：my_test/
"""


def cli():
    args = sys.argv[1:]
    if not args or args[0] in ('-h', '--help', 'help'):
        print(USAGE)
        return

    cmd = args[0]
    out_dir = 'my_test'

    try:
        if cmd == 'grid':
            rows, cols = int(args[1]), int(args[2])
            seed = int(args[3]) if len(args) > 3 else None
            if seed is not None:
                random.seed(seed)
            b = TestBuilder(randomize=True).grid(rows, cols)
            start, end = 'G_0_0', f'G_{rows - 1}_{cols - 1}'
            b.cmds(*default_commands(start, end))

        elif cmd == 'chain':
            n = int(args[1])
            seed = int(args[2]) if len(args) > 2 else None
            if seed is not None:
                random.seed(seed)
            names = [f'N{i:02d}' for i in range(n)]
            b = TestBuilder(randomize=True).chain(names)
            b.cmds(*default_commands(names[0], names[-1]))

        elif cmd == 'cycle':
            n = int(args[1])
            seed = int(args[2]) if len(args) > 2 else None
            if seed is not None:
                random.seed(seed)
            names = [f'N{i:02d}' for i in range(n)]
            b = TestBuilder(randomize=True).cycle(names)
            b.cmds(*default_commands(names[0], names[n // 2]))

        elif cmd == 'star':
            leaves_n = int(args[1])
            seed = int(args[2]) if len(args) > 2 else None
            if seed is not None:
                random.seed(seed)
            center = 'Center'
            leaves = [f'L{i:02d}' for i in range(leaves_n)]
            b = TestBuilder(randomize=True).star(center, leaves)
            b.cmds(*default_commands(leaves[0], leaves[-1]))

        elif cmd == 'random':
            n = int(args[1])
            m = int(args[2])
            seed = int(args[3]) if len(args) > 3 else None
            if seed is not None:
                random.seed(seed)
            b = TestBuilder(randomize=True).random_graph(n, m)
            start, end = f'N{1:04d}', f'N{n:04d}'
            b.cmds(*default_commands(start, end))

        elif cmd == 'example':
            if len(args) < 2 or args[1] not in EXAMPLES:
                print(f'可用 example：{", ".join(EXAMPLES.keys())}')
                return
            b = EXAMPLES[args[1]]()

        else:
            print(f'未知类型：{cmd}\n')
            print(USAGE)
            return

    except (IndexError, ValueError) as e:
        print(f'参数错误：{e}\n')
        print(USAGE)
        return

    print(f'生成测试数据 [{cmd}] -> {out_dir}/ ...')
    b.save(out_dir)
    print()
    print('接下来：')
    print('  cd CampusNavigation')
    print('  cp ../my_test/*.csv .')
    print('  cp ../my_test/command.txt .')
    print('  ./CampusNavigation < command.txt')


if __name__ == '__main__':
    cli()