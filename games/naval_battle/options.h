EXTEND_OPTION("放置阶段时间限制（总时间限制）", 放置时限, (ArithChecker<uint32_t>(60, 3600, "超时时间（秒）")), 300)
EXTEND_OPTION("进攻阶段时间限制（进攻操作每一步的时间限制）", 进攻时限, (ArithChecker<uint32_t>(30, 3600, "超时时间（秒）")), 120)
EXTEND_OPTION("设置地图大小", 边长, (ArithChecker<uint32_t>(8, 15, "边长")), 10)
EXTEND_OPTION("设置飞机数量", 飞机, (ArithChecker<uint32_t>(1, 8, "数量")), 3)
EXTEND_OPTION("是否允许飞机互相重叠", 重叠, (BoolChecker("允许", "不允许")), false)
EXTEND_OPTION("设置命中要害是否有提示", 要害, AlterChecker<int>({{"有", 0}, {"无", 1}, {"首次", 2}}), 0)
EXTEND_OPTION("连发次数", 连发, (ArithChecker<uint32_t>(1, 10, "次数")), 3)
EXTEND_OPTION("初始随机侦察区域大小（默认为随机）", 侦察, (ArithChecker<uint32_t>(0, 30, "面积")), 100)
EXTEND_OPTION("BOSS挑战类型：【快捷配置已不再此处支持】<br/>"
              "如需使用快捷配置请使用「#规则 大海战」查看帮助", BOSS挑战, (ArithChecker<uint32_t>(0, 3, "类型")), 100)
EXTEND_OPTION("自定义飞机形状：需包含5*5图形内的所有格子信息，参数分为5段（视为5行），一行5个格子。其中2为机头（需被全部打击），1为机身，0为空地", 形状,
            (RepeatableChecker<BasicChecker<std::string>>("形状参数", "00000 00200 11111 00100 01110")), (std::vector<std::string>{"默认"}))
