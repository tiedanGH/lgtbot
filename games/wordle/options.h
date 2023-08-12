EXTEND_OPTION("回合数", 回合数, (ArithChecker<uint32_t>(1, 40, "回合数")), 20)
EXTEND_OPTION("每回合时间限制", 时限, (ArithChecker<uint32_t>(10, 3600, "超时时间（秒）")), 180)
EXTEND_OPTION("设置随机配置", 随机, (AlterChecker<int>({{"5-8", 1}, {"2-11", 2}})), 1)
EXTEND_OPTION("长度", 长度, (ArithChecker<uint32_t>(2, 11, "长度")), 0)
EXTEND_OPTION("高难", 高难, (ArithChecker<uint32_t>(0, 2, "高难")), 0)