EXTEND_OPTION("每局游戏先根据模式从区块池抽取12+4组成随机池，再抽取地图区块<br>"
    "【标准】仅从**经典区块**中抽取区块　　　【幻变】将从**轮换区块**中随机抽取<br>"
    "【狂野】将从**所有区块**中随机抽取　　　【疯狂】随机池包含2个**特殊区块**", 模式,
    (AlterChecker<int32_t>({{"标准", 0}, {"狂野", 1}, {"幻变", 2}, {"疯狂", 3}})), 2)
EXTEND_OPTION("捉捕目标：设置游戏中玩家的捉捕顺序", 捉捕目标, (BoolChecker("下家", "上家")), true)
EXTEND_OPTION("设置游戏地图边长", 边长, (AlterChecker<int32_t>({{"9", 9}, {"10", 10}, {"12", 12}})), 9)
EXTEND_OPTION("点杀模式：捕捉仅在回合结束时触发，路过不会触发捕捉", 点杀, (BoolChecker("开启", "关闭")), false)
EXTEND_OPTION("隐匿模式：隐匿后私聊行动，可选回合和单步模式", 隐匿, (AlterChecker<int32_t>({{"关闭", 0}, {"回合", 1}, {"单步", 2}})), 0)
EXTEND_OPTION("大乱斗模式：逃生舱改为随机传送", 大乱斗, (BoolChecker("开启", "关闭")), false)
EXTEND_OPTION("在游戏中生成BOSS牛头怪米诺陶斯，在回合结束时追击玩家", BOSS, (BoolChecker("开启", "关闭")), false)
EXTEND_OPTION("设置游戏特殊事件", 特殊事件, (AlterChecker<int32_t>({
    {"随机", -1}, {"无", 0}, {"怠惰的园丁", 1}, {"营养过剩", 2}, {"雨天小故事", 3}
})), 0)
EXTEND_OPTION("行动前思考的时间限制", 思考时限, (ArithChecker<uint32_t>(30, 3600, "超时时间（秒）")), 180)
EXTEND_OPTION("开始行动后的总时间限制", 行动时限, (ArithChecker<uint32_t>(60, 3600, "超时时间（秒）")), 360)
