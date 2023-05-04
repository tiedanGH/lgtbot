#ifdef INIT_OPTION_DEPEND

#include "game_util/poker.h"

#else

EXTEND_OPTION("设置每件商品投标最大轮数（包括首轮投标），若达到该投标轮数仍有复数玩家投标额最高，则流标", 投标轮数, (ArithChecker<uint32_t>(1, 9, "轮数")), 2)
EXTEND_OPTION("每件商品投标时间（秒）", 投标时间, (ArithChecker<uint32_t>(10, 300, "投标时间（秒）")), 60)
EXTEND_OPTION("弃牌时间（秒）", 弃牌时间, (ArithChecker<uint32_t>(10, 3600, "弃牌时间（秒）")), 200)
EXTEND_OPTION("玩家初始金币数", 初始金币数, (ArithChecker<uint32_t>(100, 100000, "金币数")), 300)
EXTEND_OPTION("回合数", 回合数, (ArithChecker<uint32_t>(1, 10, "回合数")), 3)
EXTEND_OPTION("随机种子", 种子, (AnyArg("种子", "我是随便输入的一个字符串")), "")
EXTEND_OPTION("末尾玩家扣除金币比例（%）", 惩罚比例, (ArithChecker<uint32_t>(0, 1, "比例")), 25)
EXTEND_OPTION("使用的卡牌类型", 卡牌, (AlterChecker<game_util::poker::CardType>(std::map<std::string, game_util::poker::CardType>{
                { "波卡", game_util::poker::CardType::BOKAA },
                { "扑克", game_util::poker::CardType::POKER },
            })), game_util::poker::CardType::BOKAA)

#endif
