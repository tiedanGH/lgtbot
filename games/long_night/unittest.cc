// Copyright (c) 2018-present, Chang Liu <github.com/slontia>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).

#include "game_framework/unittest_base.h"

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

GAME_TEST(2, leave_test1)
{
    START_GAME();

    ASSERT_LEAVE(CHECKOUT, 0);

    ASSERT_SCORE(-300, 0);
}

GAME_TEST(2, leave_test2)
{
    START_GAME();

    ASSERT_LEAVE(CHECKOUT, 1);

    ASSERT_SCORE(0, -300);
}

GAME_TEST(3, all_active_stop1)
{
    START_GAME();

    ASSERT_PUB_MSG(CONTINUE, 0, "停止");
    ASSERT_PUB_MSG(CONTINUE, 1, "停止");
    ASSERT_PUB_MSG(CHECKOUT, 2, "停止");

    ASSERT_FINISHED(true);
    ASSERT_SCORE(0, 0, 0);
}

GAME_TEST(3, all_active_stop2)
{
    START_GAME();

    ASSERT_TIMEOUT(CONTINUE);
    ASSERT_TIMEOUT(CONTINUE);
    ASSERT_PUB_MSG(CHECKOUT, 2, "停止");

    ASSERT_FINISHED(true);
    ASSERT_SCORE(0, 0, 0);
}

GAME_TEST(3, all_active_stop3)
{
    START_GAME();

    ASSERT_TIMEOUT(CONTINUE);
    ASSERT_TIMEOUT(CONTINUE);
    ASSERT_TIMEOUT(CHECKOUT);

    ASSERT_FINISHED(true);
    ASSERT_SCORE(0, 0, 0);
}

GAME_TEST(3, all_active_stop_after_move)
{
    START_GAME();

    if (LGTBOT_STAGE_ERR_UINT_(this->PublicRequest(0, "上")) == LGTBOT_STAGE_ERR_UINT_(OK)) {
        ASSERT_PUB_MSG(CONTINUE, 0, "停止");
    }
    ASSERT_PUB_MSG(CONTINUE, 1, "停止");
    ASSERT_PUB_MSG(CHECKOUT, 2, "停止");

    ASSERT_FINISHED(false);
}

// 谋定后动：移动后结束回合，不计入全员主动停止
GAME_TEST(3, deliberate_move_ends_turn)
{
    ASSERT_PRI_MSG(OK, 0, "谋定后动 开启");
    START_GAME();

    ASSERT_PUB_MSG(CONTINUE, 0, "上");
    ASSERT_PUB_MSG(CONTINUE, 1, "停止");
    ASSERT_PUB_MSG(CHECKOUT, 2, "停止");

    ASSERT_FINISHED(false);
}

// 多BOSS：配置项可一次配置多个，同时生效
GAME_TEST(2, multi_boss_option)
{
    ASSERT_PRI_MSG(OK, 0, "BOSS 米诺陶斯 邦邦");
    START_GAME();

    ASSERT_PUB_MSG(CONTINUE, 0, "停止");
    ASSERT_PUB_MSG(CHECKOUT, 1, "停止");
}

// 多BOSS：一键指令分别配置不再互相覆盖
GAME_TEST(2, multi_boss_init_command)
{
    ASSERT_PRI_MSG(OK, 0, "米诺陶斯 邦邦");
    START_GAME();

    ASSERT_PUB_MSG(CONTINUE, 0, "停止");
    ASSERT_PUB_MSG(CHECKOUT, 1, "停止");
}

// 多BOSS：同类型可重复配置生成多只，[无]会被剔除
GAME_TEST(2, multi_boss_duplicate)
{
    ASSERT_PRI_MSG(OK, 0, "BOSS 邦邦 无 邦邦 米诺陶斯");
    START_GAME();

    ASSERT_PUB_MSG(CONTINUE, 0, "停止");
    ASSERT_PUB_MSG(CHECKOUT, 1, "停止");
}

// 多BOSS：一键指令重复配置同一BOSS也会叠加生成
GAME_TEST(2, multi_boss_init_duplicate)
{
    ASSERT_PRI_MSG(OK, 0, "米诺陶斯 米诺陶斯 邦邦");
    START_GAME();

    ASSERT_PUB_MSG(CONTINUE, 0, "停止");
    ASSERT_PUB_MSG(CHECKOUT, 1, "停止");
}

// 多BOSS：超出数量上限时丢弃多余BOSS，仍正常开局
GAME_TEST(2, multi_boss_exceed_limit)
{
    ASSERT_PRI_MSG(OK, 0, "BOSS 邦邦 邦邦 邦邦 米诺陶斯 米诺陶斯 米诺陶斯 邦邦");
    START_GAME();

    ASSERT_PUB_MSG(CONTINUE, 0, "停止");
    ASSERT_PUB_MSG(CHECKOUT, 1, "停止");
}

// 幻变模式正常开局
GAME_TEST(2, twist_mode_start)
{
    ASSERT_PRI_MSG(OK, 0, "模式 幻变");
    START_GAME();

    ASSERT_PUB_MSG(CONTINUE, 0, "停止");
    ASSERT_PUB_MSG(CHECKOUT, 1, "停止");

    ASSERT_SCORE(0, 0);
}

// 巨大的心房区块铺满地图可正常开局
GAME_TEST(2, heart_blocks_full_map)
{
    ASSERT_PRI_MSG(OK, 0, "区块 51 51 51 51 51 51 51 51 51");
    START_GAME();

    ASSERT_PUB_MSG(CONTINUE, 0, "停止");
    ASSERT_PUB_MSG(CHECKOUT, 1, "停止");

    ASSERT_SCORE(0, 0);
}

// 玩家随机行动直至游戏结束：仅检查流程不会崩溃，并能在请求上限内正常结束
#define PLAY_RANDOMLY_UNTIL_OVER(player_num, max_requests)                              \
    do {                                                                                \
        const char* const k_actions[] = {"上", "下", "左", "右", "停止"};                 \
        for (int i = 0; i < (max_requests) && !this->main_stage_->IsOver(); i++) {      \
            const uint32_t pid = i % (player_num);                                      \
            if (this->match_->IsEliminated(PlayerID{pid})) continue;                    \
            this->PublicRequest(pid, k_actions[rand() % 5]);                            \
        }                                                                               \
        ASSERT_FINISHED(true);                                                          \
    } while (0)

// [暴君] 谋定后动下每次移动即结束行动，多回合随机推进
GAME_TEST(4, tyrant_random_play)
{
    ASSERT_PRI_MSG(OK, 0, "BOSS 暴君");
    ASSERT_PRI_MSG(OK, 0, "谋定后动 开启");
    START_GAME();
    PLAY_RANDOMLY_UNTIL_OVER(4, 5000);
}

// [暴君] 一键指令配置，普通行动模式随机推进
GAME_TEST(3, tyrant_random_play_normal)
{
    ASSERT_PRI_MSG(OK, 0, "暴君");
    START_GAME();
    PLAY_RANDOMLY_UNTIL_OVER(3, 6000);
}

// [暴君] 与其他 BOSS 同时存在（含两只暴君）、静音模式（无[噤声]铁律）随机推进
GAME_TEST(5, tyrant_with_other_bosses_silence)
{
    ASSERT_PRI_MSG(OK, 0, "BOSS 暴君 米诺陶斯 邦邦 暴君");
    ASSERT_PRI_MSG(OK, 0, "静音 开启");
    ASSERT_PRI_MSG(OK, 0, "谋定后动 开启");
    START_GAME();
    PLAY_RANDOMLY_UNTIL_OVER(5, 8000);
}

// [新手教程] 每次请求后让电脑行动，直到电脑都无事可做（模拟框架驱动电脑）
#define TUTORIAL_COMPUTERS_ACT()                                                                                \
    for (int i = 0; i < 20 && !this->main_stage_->IsOver(); i++) {                                              \
        bool idle = true;                                                                                       \
        for (const uint64_t pid : {1, 2}) {                                                                     \
            if (!this->main_stage_->IsOver() &&                                                                 \
                LGTBOT_STAGE_ERR_UINT_(this->ComputerActRequest_(pid)) != LGTBOT_STAGE_ERR_UINT_(OK)) idle = false; \
        }                                                                                                       \
        if (idle) break;                                                                                        \
    }

// [新手教程] 0号按提示走完全部 6 章（多步路线拆成单步，省去逐步演示的等待）：陪练按路线发声、捕捉后传送、最后逃生
GAME_TEST(3, tutorial_full_run)
{
    ASSERT_PRI_MSG(OK, 0, "教程 开启");
    START_GAME();
    TUTORIAL_COMPUTERS_ACT();
    ASSERT_PUB_MSG(CHECKOUT, 0, "开始");
    const char* const k_steps[] = {
        "继续", "继续", "继续", "继续",
        "左", "下", "继续", "继续", "继续", "上", "上", "继续", "右", "上", "继续",
        "下", "左", "左", "右", "继续", "继续", "下", "右", "下", "左", "上", "继续",
        "继续", "停止", "停止", "继续", "继续", "继续", "继续",
        "左", "上", "右", "继续", "继续",
        "右", "下", "上", "左", "左", "上",
    };
    for (const char* const step : k_steps) {
        const auto rc = this->PublicRequest(0, step);
        ASSERT_NE(LGTBOT_STAGE_ERR_UINT_(FAILED), LGTBOT_STAGE_ERR_UINT_(rc)) << step;
        ASSERT_NE(LGTBOT_STAGE_ERR_UINT_(NOT_FOUND), LGTBOT_STAGE_ERR_UINT_(rc)) << step;
        TUTORIAL_COMPUTERS_ACT();
    }
    ASSERT_FINISHED(true);
}

// [新手教程] 讲解页拦截行动；操作页只放行提示的行动，位置不变
GAME_TEST(3, tutorial_guard)
{
    ASSERT_PRI_MSG(OK, 0, "教程 开启");
    START_GAME();
    TUTORIAL_COMPUTERS_ACT();
    ASSERT_PUB_MSG(CHECKOUT, 0, "开始");
    ASSERT_PUB_MSG(FAILED, 0, "左");
    ASSERT_PUB_MSG(OK, 0, "目录");
    for (int i = 0; i < 4; i++) ASSERT_PUB_MSG(OK, 0, "继续");
    ASSERT_PUB_MSG(FAILED, 0, "继续");
    ASSERT_PUB_MSG(FAILED, 0, "右");
    ASSERT_PUB_MSG(FAILED, 0, "停止");
    ASSERT_PUB_MSG(OK, 0, "左");
}

// [新手教程] 仅限 1 名玩家加 2 名电脑，人数不对时拒绝开局
GAME_TEST(2, tutorial_rejects_two_players)
{
    ASSERT_PRI_MSG(OK, 0, "教程 开启");
    ASSERT_FALSE(this->StartGame());
}

GAME_TEST(4, tutorial_rejects_four_players)
{
    ASSERT_PRI_MSG(OK, 0, "教程 开启");
    ASSERT_FALSE(this->StartGame());
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
