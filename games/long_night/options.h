EXTEND_OPTION("[区块] 先根据模式从区块池抽取 12+4 组成随机池，再抽取地图区块<br>"
    "【标准】仅从**经典区块**中抽取区块　　　【幻变】将从**轮换区块**中随机抽取<br>"
    "【狂野】将从**所有区块**中随机抽取　　　【疯狂】随机池包含2个**特殊区块**", 模式,
    (AlterChecker<int32_t>({{"标准", 0}, {"狂野", 1}, {"幻变", 2}, {"疯狂", 3}})), 2)
EXTEND_OPTION("[区块] 自定义游戏区块随机池：自定义区块时「模式」配置不生效", 区块,
    (RepeatableChecker<BasicChecker<std::string>>("区块", "1 5 6 16 34 38 E1 e7 S4 s1")), (std::vector<std::string>{"默认"}))

EXTEND_OPTION("[常规] 设置游戏地图边长", 边长, (AlterChecker<int32_t>({{"9", 9}, {"10", 10}, {"12", 12}})), 9)
EXTEND_OPTION("[常规] 游戏最大回合数限制", 回合数, (ArithChecker<uint32_t>(3, 40, "回合数")), 20)
EXTEND_OPTION("[常规] 捉捕目标：设置游戏中玩家的捉捕顺序", 捉捕目标, (BoolChecker("下家", "上家")), true)
EXTEND_OPTION("[常规] 停止私信：玩家主动停止或超时，得知私信墙壁信息", 停止私信, (BoolChecker("开启", "关闭")), false)

EXTEND_OPTION("[事件] 设置游戏特殊事件", 特殊事件, (AlterChecker<int32_t>({
    {"随机", -1}, {"无", 0}, {"怠惰的园丁", 1}, {"营养过剩", 2}, {"雨天小故事", 3}
})), 0)

EXTEND_OPTION("[模式] BOSS：具体规则详见「#规则 漫漫长夜 BOSS」", BOSS,
    (AlterChecker<int32_t>({{"无", 0}, {"米诺陶斯", 1}, {"邦邦", 2}})), 0)
EXTEND_OPTION("[模式] 点杀：捕捉改为仅在回合结束时触发，路过不会触发捕捉", 点杀, (BoolChecker("开启", "关闭")), true)
EXTEND_OPTION("[模式] 隐匿：隐匿后私聊行动，可选回合和单步模式", 隐匿, (AlterChecker<int32_t>({{"关闭", 0}, {"回合", 1}, {"单步", 2}})), 0)
EXTEND_OPTION("[模式] 大乱斗：逃生舱改为随机传送", 大乱斗, (BoolChecker("开启", "关闭")), false)
EXTEND_OPTION("[模式] 谋定后动：每回合仅能执行一次移动，可使用多步行动指令", 谋定后动, (BoolChecker("开启", "关闭")), false)
EXTEND_OPTION("[模式] 炸弹人：公开安置炸弹，任何人经过炸弹并离开会引爆炸弹并出局", 炸弹, (ArithChecker<uint32_t>(0, 3, "数量")), 0)

EXTEND_OPTION("[时限] 行动前思考的时间限制", 思考时限, (ArithChecker<uint32_t>(30, 3600, "超时时间（秒）")), 120)
EXTEND_OPTION("[时限] 开始行动后的总时间限制", 行动时限, (ArithChecker<uint32_t>(60, 3600, "超时时间（秒）")), 300)

EXTEND_OPTION("[图片] 设置游戏使用的图片素材&纹理", 纹理, (AlterChecker<int32_t>({{"经典", 0}, {"复古", 1}})), 0)
