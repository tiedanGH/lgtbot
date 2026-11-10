// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).

#include "game_framework/unittest_base.h"

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

// 5 人局：0 号为渔夫，1 ~ 4 号为鱼
#define FIVE_PLAYER_GAME()                          \
    do {                                            \
        ASSERT_PUB_MSG(OK, 0, "渔夫列表 1");        \
        START_GAME();                               \
    } while (0)

// 全员停留原地
#define STAY_ALL()                                  \
    do {                                            \
        ASSERT_PRI_MSG(OK, 0, "停留");              \
        ASSERT_PRI_MSG(OK, 1, "停留");              \
        ASSERT_PRI_MSG(OK, 2, "停留");              \
        ASSERT_PRI_MSG(OK, 3, "停留");              \
        ASSERT_PRI_MSG(CHECKOUT, 4, "停留");        \
    } while (0)

// 渔夫下饵，四条鱼自动咬住唯一的钩并全部投入 0 体力，渔夫猜 0.5 必定落空。
// 场上只有一个鱼钩且无极度饥饿的鱼，择钩阶段会被整个略过
#define SIMPLE_ROUND(bait)                          \
    do {                                            \
        ASSERT_PRI_MSG(CHECKOUT, 0, "钓鱼 " bait);  \
        ASSERT_PRI_MSG(OK, 1, "0");                 \
        ASSERT_PRI_MSG(OK, 2, "0");                 \
        ASSERT_PRI_MSG(OK, 3, "0");                 \
        ASSERT_PRI_MSG(OK, 4, "0");                 \
        ASSERT_PRI_MSG(CHECKOUT, 0, "0.5");         \
    } while (0)


GAME_TEST(4, player_not_enough)
{
    ASSERT_FALSE(StartGame());
}

// 渔夫本池有鱼且自己有饵时必须下钩，且只能在 B 池挖饵
GAME_TEST(5, fisherman_action_check)
{
    FIVE_PLAYER_GAME();

    ASSERT_PRI_MSG(FAILED, 0, "放弃");           // 有鱼有饵，不得放弃
    ASSERT_PRI_MSG(FAILED, 0, "挖饵");           // A 池无沙虫可挖
    ASSERT_PRI_MSG(FAILED, 0, "钓鱼 沙虫");      // 尚未挖到沙虫
    ASSERT_PRI_MSG(FAILED, 1, "钓鱼 初级");      // 鱼无法执行渔夫行动
    ASSERT_PUB_MSG(FAILED, 0, "钓鱼 初级");      // 必须私信
    ASSERT_PRI_MSG(CHECKOUT, 0, "钓鱼 初级");
    ASSERT_PRI_MSG(NOT_FOUND, 0, "钓鱼 初级");   // 阶段已切换，该指令不复存在
}

// 本池只有一个鱼钩且无极度饥饿的鱼时，鱼群自动咬钩，择钩阶段整个略过
GAME_TEST(5, single_hook_skips_hook_stage)
{
    FIVE_PLAYER_GAME();

    ASSERT_PRI_MSG(CHECKOUT, 0, "钓鱼 初级");
    ASSERT_PRI_MSG(NOT_FOUND, 1, "咬钩 1");      // 择钩阶段已被略过
    ASSERT_PRI_MSG(OK, 1, "0");                  // 直接进入角力阶段提交体力
    ASSERT_PRI_MSG(OK, 2, "0");
    ASSERT_PRI_MSG(OK, 3, "0");
    ASSERT_PRI_MSG(OK, 4, "0");
    ASSERT_PRI_MSG(CHECKOUT, 0, "0.5");
    ASSERT_FINISHED(false);
}

// 择钩与角力阶段的取值校验。两名渔夫同时下钩，鱼有得选，择钩阶段照常进行
GAME_TEST(7, hook_and_stamina_check)
{
    ASSERT_PUB_MSG(OK, 0, "渔夫列表 1 2");
    START_GAME();

    ASSERT_PRI_MSG(OK, 0, "钓鱼 中等");
    ASSERT_PRI_MSG(CHECKOUT, 1, "钓鱼 初级");

    ASSERT_PRI_MSG(FAILED, 2, "咬钩 4");         // 4 号不是渔夫
    ASSERT_PRI_MSG(FAILED, 2, "捕猎 4");         // 场上没有极度饥饿的鱼
    ASSERT_PRI_MSG(FAILED, 0, "咬钩 1");         // 渔夫无法执行鱼的行动
    ASSERT_PUB_MSG(FAILED, 2, "咬钩 1");         // 必须私信
    ASSERT_PRI_MSG(OK, 2, "咬钩 1");
    ASSERT_PRI_MSG(FAILED, 2, "咬钩 1");         // 不得重复提交
    ASSERT_PRI_MSG(OK, 3, "咬钩 1");
    ASSERT_PRI_MSG(OK, 4, "咬钩 2");
    ASSERT_PRI_MSG(OK, 5, "咬钩 2");
    ASSERT_PRI_MSG(CHECKOUT, 6, "咬钩 1");

    ASSERT_PRI_MSG(NOT_FOUND, 2, "6");           // 体力最高只能到 5
    ASSERT_PRI_MSG(NOT_FOUND, 2, "0.3");         // 必须是 0.5 的倍数
    ASSERT_PRI_MSG(FAILED, 0, "0");              // 渔夫无法猜测 0 体力
    ASSERT_PUB_MSG(FAILED, 2, "1");              // 必须私信
    ASSERT_PRI_MSG(OK, 2, "体力 1");
    ASSERT_PRI_MSG(OK, 3, "0");
    ASSERT_PRI_MSG(OK, 4, "0");
    ASSERT_PRI_MSG(OK, 5, "0");
    ASSERT_PRI_MSG(OK, 6, "0");
    ASSERT_PRI_MSG(OK, 0, "垂钓 2.5");
    ASSERT_PRI_MSG(CHECKOUT, 1, "垂钓 2.5");
}

// 全程无人被钓上，鱼群取得群体胜利；渔夫鱼饵耗尽后游戏立即结束
GAME_TEST(5, fish_group_win_when_baits_run_out)
{
    FIVE_PLAYER_GAME();

    SIMPLE_ROUND("初级");                        // R1  肥度 5 → 4.5
    STAY_ALL();
    SIMPLE_ROUND("初级");                        // R2  4.5 → 4
    STAY_ALL();
    SIMPLE_ROUND("初级");                        // R3  4 → 3.5
    STAY_ALL();
    SIMPLE_ROUND("中等");                        // R4  3.5 → 3.5
    STAY_ALL();
    SIMPLE_ROUND("中等");                        // R5  3.5 → 3.5
    STAY_ALL();
    SIMPLE_ROUND("高等");                        // R6  3.5 → 4，鱼饵耗尽，游戏结束

    // 四条鱼肥度相同并列第一，且全程无鱼被钓上
    ASSERT_SCORE(0, 75, 75, 75, 75);
}

// 渔夫猜中体力，鱼被钓上并计入公斤数
GAME_TEST(5, fisherman_catches_fish)
{
    ASSERT_PUB_MSG(OK, 0, "渔夫列表 1");
    ASSERT_PUB_MSG(OK, 0, "回合数 3");
    START_GAME();

    // R1：1 号投入 1 点体力，恰好被渔夫料中
    ASSERT_PRI_MSG(CHECKOUT, 0, "钓鱼 初级");
    ASSERT_PRI_MSG(OK, 1, "1");
    ASSERT_PRI_MSG(OK, 2, "0");
    ASSERT_PRI_MSG(OK, 3, "0");
    ASSERT_PRI_MSG(OK, 4, "0");
    ASSERT_PRI_MSG(CHECKOUT, 0, "1");
    ASSERT_ELIMINATED(1);

    // R2
    ASSERT_PRI_MSG(OK, 0, "停留");
    ASSERT_PRI_MSG(OK, 2, "停留");
    ASSERT_PRI_MSG(OK, 3, "停留");
    ASSERT_PRI_MSG(CHECKOUT, 4, "停留");
    ASSERT_PRI_MSG(CHECKOUT, 0, "钓鱼 初级");
    ASSERT_PRI_MSG(OK, 2, "0");
    ASSERT_PRI_MSG(OK, 3, "0");
    ASSERT_PRI_MSG(OK, 4, "0");
    ASSERT_PRI_MSG(CHECKOUT, 0, "0.5");

    // R3
    ASSERT_PRI_MSG(OK, 0, "停留");
    ASSERT_PRI_MSG(OK, 2, "停留");
    ASSERT_PRI_MSG(OK, 3, "停留");
    ASSERT_PRI_MSG(CHECKOUT, 4, "停留");
    ASSERT_PRI_MSG(CHECKOUT, 0, "钓鱼 初级");
    ASSERT_PRI_MSG(OK, 2, "0");
    ASSERT_PRI_MSG(OK, 3, "0");
    ASSERT_PRI_MSG(OK, 4, "0");
    ASSERT_PRI_MSG(CHECKOUT, 0, "0.5");

    // 渔夫钓上 1 条 5 公斤的鱼，存活的三条鱼肥度相同并列第一但无群体胜利分
    ASSERT_SCORE(55, 0, 55, 55, 55);
}

// 极度饥饿的鱼被同胞捕猎吞噬
GAME_TEST(5, hunt_starving_fish)
{
    ASSERT_PUB_MSG(OK, 0, "渔夫列表 1");
    ASSERT_PUB_MSG(OK, 0, "回合数 3");
    START_GAME();

    // R1：1 号投入 4.5 点体力却输给了 2 号的 5 点，肥度跌至 -0.5
    ASSERT_PRI_MSG(CHECKOUT, 0, "钓鱼 初级");
    ASSERT_PRI_MSG(OK, 1, "4.5");
    ASSERT_PRI_MSG(OK, 2, "5");
    ASSERT_PRI_MSG(OK, 3, "0");
    ASSERT_PRI_MSG(OK, 4, "0");
    ASSERT_PRI_MSG(CHECKOUT, 0, "0.5");

    // R2：1 号再度落空，肥度跌至 -4，陷入极度饥饿
    STAY_ALL();
    ASSERT_PRI_MSG(CHECKOUT, 0, "钓鱼 初级");
    ASSERT_PRI_MSG(OK, 1, "2.5");
    ASSERT_PRI_MSG(OK, 2, "3");
    ASSERT_PRI_MSG(OK, 3, "0");
    ASSERT_PRI_MSG(OK, 4, "0");
    ASSERT_PRI_MSG(CHECKOUT, 0, "0.5");

    // R3：场上出现极度饥饿的鱼，未饥饿者多了捕猎这一选项，择钩阶段照常进行。
    // 而 2 号玩家自己已极度饥饿、无法捕猎，本池又只有一个钩，故已被自动咬钩
    STAY_ALL();
    ASSERT_PRI_MSG(CHECKOUT, 0, "钓鱼 初级");
    ASSERT_PRI_MSG(FAILED, 1, "捕猎 3");         // 已被自动咬钩，且极度饥饿的鱼无法捕猎
    ASSERT_PRI_MSG(FAILED, 3, "捕猎 4");         // 4 号玩家并未陷入极度饥饿
    ASSERT_PRI_MSG(OK, 2, "咬钩 1");
    ASSERT_PRI_MSG(OK, 3, "捕猎 2");
    ASSERT_PRI_MSG(CHECKOUT, 4, "捕猎 2");
    ASSERT_PRI_MSG(OK, 1, "1");
    ASSERT_PRI_MSG(OK, 2, "0");
    ASSERT_PRI_MSG(OK, 3, "1");                  // 猜中
    ASSERT_PRI_MSG(OK, 4, "2");                  // 猜错
    ASSERT_PRI_MSG(CHECKOUT, 0, "3");
    ASSERT_ELIMINATED(1);

    // 3 号独吞 3 点肥度成为最肥，4 号次之，2 号垫底；全程无鱼被钓上
    ASSERT_SCORE(0, 0, 50, 75, 65);
}

// 渔夫移动到 B 池挖饵，本池无鱼时鱼群无需行动；水池指令同时支持大小写
GAME_TEST(5, dig_bait_in_pool_b)
{
    FIVE_PLAYER_GAME();

    SIMPLE_ROUND("初级");

    // R2：渔夫独自前往 B 池，此处用小写验证指令兼容大小写
    ASSERT_PRI_MSG(OK, 0, "前往 b");
    ASSERT_PRI_MSG(OK, 1, "停留");
    ASSERT_PRI_MSG(OK, 2, "停留");
    ASSERT_PRI_MSG(OK, 3, "停留");
    ASSERT_PRI_MSG(CHECKOUT, 4, "停留");
    // B 池无鱼，可以放弃，也可以挖饵；挖饵后两池皆无鱼钩，本回合直接结算
    ASSERT_PRI_MSG(CHECKOUT, 0, "挖饵");

    // R3：渔夫返回 A 池，此处用大写
    ASSERT_PRI_MSG(OK, 0, "前往 A");
    ASSERT_PRI_MSG(OK, 1, "停留");
    ASSERT_PRI_MSG(OK, 2, "停留");
    ASSERT_PRI_MSG(OK, 3, "停留");
    ASSERT_PRI_MSG(CHECKOUT, 4, "停留");
    ASSERT_PRI_MSG(FAILED, 0, "挖饵");           // A 池无法挖饵
    ASSERT_PRI_MSG(CHECKOUT, 0, "钓鱼 沙虫");
}

// 任命模式：任命者必须一次性任命符合配置的渔夫数量
GAME_TEST(6, appoint_mode)
{
    ASSERT_PUB_MSG(OK, 0, "任命模式 开启");
    START_GAME();

    ASSERT_PRI_MSG(FAILED, 1, "3");              // 任命权不在 1 号手中
    ASSERT_PRI_MSG(FAILED, 0, "2 3");            // 6 人局只需 1 名渔夫
    ASSERT_PRI_MSG(FAILED, 0, "9");              // 场上没有 9 号
    ASSERT_PRI_MSG(CHECKOUT, 0, "2");            // 2 号（即 1 号玩家）成为渔夫

    ASSERT_PRI_MSG(FAILED, 0, "钓鱼 初级");      // 0 号是鱼
    ASSERT_PRI_MSG(CHECKOUT, 1, "钓鱼 高等");
}

// 任命者放弃任命时改由随机指派
GAME_TEST(6, appoint_pass)
{
    ASSERT_PUB_MSG(OK, 0, "任命模式 开启");
    START_GAME();

    ASSERT_PRI_MSG(FAILED, 2, "pass");           // 任命权不在 2 号手中
    ASSERT_PRI_MSG(CHECKOUT, 0, "pass");
    ASSERT_FINISHED(false);
}

// 超时的渔夫会被随机系上鱼饵下钩，超时的鱼默认投入 0 体力
GAME_TEST(5, timeout_fills_default_action)
{
    FIVE_PLAYER_GAME();

    ASSERT_TIMEOUT(CHECKOUT);                    // 渔夫超时，随机下钩；随后择钩阶段被略过
    // 角力超时：鱼默认投入 0 体力，渔夫默认猜 5，投入 0 体力的鱼会一并上钩
    ASSERT_TIMEOUT(CHECKOUT);
    ASSERT_FINISHED(true);
    ASSERT_SCORE(55, 0, 0, 0, 0);
}

// 鱼中途退出视同饿死腐烂，渔夫中途退出后其鱼钩仍会收线
GAME_TEST(5, leave_test)
{
    FIVE_PLAYER_GAME();

    ASSERT_PRI_MSG(CHECKOUT, 0, "钓鱼 初级");
    ASSERT_PRI_MSG(OK, 1, "0");
    ASSERT_LEAVE(CONTINUE, 1);                   // 已提交的行动作废
    ASSERT_PRI_MSG(OK, 2, "0");
    ASSERT_PRI_MSG(OK, 3, "0");
    ASSERT_PRI_MSG(OK, 4, "0");
    ASSERT_PRI_MSG(CHECKOUT, 0, "0.5");
    ASSERT_FINISHED(false);
}

// 全部渔夫退出后游戏立即结束，存活的鱼取得群体胜利
GAME_TEST(5, all_fishermen_leave)
{
    FIVE_PLAYER_GAME();

    ASSERT_LEAVE(CHECKOUT, 0);                   // 唯一待行动者离场，下饵阶段随即结束
    ASSERT_FINISHED(true);
    ASSERT_SCORE(-50, 75, 75, 75, 75);
}

// 渔夫在提交垂钓前退出，其已投下的鱼钩仍会按猜测满体力收线
GAME_TEST(6, fisherman_leave_after_casting)
{
    ASSERT_PUB_MSG(OK, 0, "渔夫列表 1");
    START_GAME();

    ASSERT_PRI_MSG(CHECKOUT, 0, "钓鱼 初级");
    ASSERT_PRI_MSG(OK, 1, "0");
    ASSERT_PRI_MSG(OK, 2, "0");
    ASSERT_PRI_MSG(OK, 3, "0");
    ASSERT_PRI_MSG(OK, 4, "0");
    ASSERT_PRI_MSG(OK, 5, "0");
    ASSERT_LEAVE(CHECKOUT, 0);                   // 视为猜 5，投入 0 体力的鱼全部上钩

    ASSERT_FINISHED(true);
    ASSERT_SCORE(5, 0, 0, 0, 0, 0);
}

} // namespace GAME_MODULE_NAME

} // namespace game

} // namespace lgtbot

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    return RUN_ALL_TESTS();
}
