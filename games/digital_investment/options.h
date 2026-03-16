EXTEND_OPTION("N（回合数），同时也是玩家可用数字上限", N, (ArithChecker<uint32_t>(5, 30, "回合数")), 10)
EXTEND_OPTION("每组玩家提交时间限制", 时限, (ArithChecker<uint32_t>(10, 3600, "超时时间（秒）")), 120)
