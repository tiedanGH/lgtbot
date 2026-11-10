#include "constants.h"

EXTEND_OPTION("开局由随机一名玩家任命全部渔夫，关闭则直接随机分配身份", 任命模式, (BoolChecker("开启", "关闭")), false)
EXTEND_OPTION("游戏进行的回合数", 回合数, (ArithChecker<uint32_t>(3, 20, "回合数")), DEFAULT_MAX_ROUND)
EXTEND_OPTION("每个阶段的行动时间限制", 时限, (ArithChecker<uint32_t>(30, 3600, "超时时间（秒）")), 120)
#ifdef TEST_BOT
EXTEND_OPTION("【仅测试用】指定作为渔夫的玩家编号", 渔夫列表, (RepeatableChecker<ArithChecker<uint32_t>>(1, MAX_PLAYER, "玩家编号")), std::vector<uint32_t>{})
#endif
