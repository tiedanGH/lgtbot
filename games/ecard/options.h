EXTEND_OPTION("每回合时间限制", 时限, (ArithChecker<uint32_t>(30, 3600, "超时时间（秒）")), 90)
EXTEND_OPTION("游戏模式（可选：经典模式、点球模式、人生模式）", 模式, (AlterChecker<uint32_t>({{"经典", 0}, {"点球", 1}, {"人生", 2}})), 2)
EXTEND_OPTION("经典模式获胜需要的奴隶胜利次数（获胜目标）[仅影响经典模式]", 目标, (ArithChecker<uint32_t>(1, 5, "分数")), 2)
EXTEND_OPTION("回合数 [仅能设置偶数，点球默认50，人生默认12且不支持修改回合数]", 回合数, (ArithChecker<uint32_t>(2, 50, "回合数")), 12)
EXTEND_OPTION("皇帝胜倍率 [仅影响人生模式，原作为10]", 皇帝, (ArithChecker<uint32_t>(1, 100, "倍率")), 5)
EXTEND_OPTION("奴隶胜倍率 [仅影响人生模式]", 奴隶, (ArithChecker<uint32_t>(1, 100, "倍率")), 50)
EXTEND_OPTION("市民牌数量", 市民数, (ArithChecker<uint32_t>(1, 9, "数量")), 4)