#ifdef INIT_OPTION_DEPEND

#include "game_util/poker.h"

#else

EXTEND_OPTION("游戏轮数", 轮数, (ArithChecker<uint32_t>(20, 1, "轮数")), 6)
EXTEND_OPTION("下注时间（秒）", 下注时间, (ArithChecker<uint32_t>(10, 3600, "弃牌时间（秒）")), 200)
EXTEND_OPTION("首轮下注时，平均每副手牌分得的筹码数量", 首轮筹码, (ArithChecker<uint32_t>(1, 100, "筹码数")), 10)
EXTEND_OPTION("次轮下注时，平均每副手牌分得的筹码数量（不得高于「首轮筹码」）", 次轮筹码, (ArithChecker<uint32_t>(1, 100, "筹码数")), 5)
EXTEND_OPTION("首轮下注时，弃牌可直接获取的积分（不得高于「首轮筹码」）", 首轮弃牌得分, (ArithChecker<uint32_t>(1, 100, "得分数")), 4)
EXTEND_OPTION("次轮下注时，弃牌可直接获取的积分（不得高于「次轮筹码」）", 次轮弃牌得分, (ArithChecker<uint32_t>(1, 100, "得分数")), 2)
EXTEND_OPTION("公共牌数", 公共牌数, (ArithChecker<uint32_t>(3, 5, "牌数")), 5)
EXTEND_OPTION("随机种子", 种子, (AnyArg("种子", "我是随便输入的一个字符串")), "")
EXTEND_OPTION("信息公开模式：0 = 隐藏手牌最大的一张，公开 2-3 张公共牌；1 = 公开全部手牌和 0-1 张公共牌；2 = 隐藏手牌最大的一张，公开 0-1 张公共牌，公布被隐藏了哪些牌",
        模式, (ArithChecker<uint32_t>(0, 2, "模式")), 0)
EXTEND_OPTION("使用的卡牌类型", 卡牌, (AlterChecker<game_util::poker::CardType>(std::map<std::string, game_util::poker::CardType>{
                { "波卡", game_util::poker::CardType::BOKAA },
                { "扑克", game_util::poker::CardType::POKER },
            })), game_util::poker::CardType::BOKAA)

#endif
