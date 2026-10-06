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

} // namespace GAME_MODULE_NAME

} // namespace game

} // namespace lgtbot

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    return RUN_ALL_TESTS();
}
