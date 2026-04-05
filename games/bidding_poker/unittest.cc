// Copyright (c) 2018-present, Chang Liu <github.com/slontia>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).

#include "game_framework/unittest_base.h"

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

GAME_TEST(1, player_not_enough)
{
    ASSERT_FALSE(StartGame());
}

GAME_TEST(5, bid_1)
{
    ASSERT_PUB_MSG(OK, 0, "初始金币数 100");
    START_GAME();
    ASSERT_PUB_MSG(OK, 0, "赛况");

    // item 1
    ASSERT_PUB_MSG(FAILED, 0, "10");
    ASSERT_PRI_MSG(OK, 0, "10");
    ASSERT_PRI_MSG(OK, 1, "12");
    ASSERT_PRI_MSG(OK, 2, "11");
    ASSERT_PRI_MSG(OK, 3, "10");
    ASSERT_PRI_MSG(FAILED, 3, "101");
    ASSERT_PRI_MSG(FAILED, 4, "0");
    ASSERT_PRI_MSG(CHECKOUT, 4, "11"); // player 1 wins

    // item 2
    ASSERT_PRI_MSG(OK, 2, "100"); // coins not decrease in item 1
    ASSERT_TIMEOUT(CHECKOUT); // player 2 wins

    // item 3
    ASSERT_PRI_MSG(FAILED, 1, "89");
    ASSERT_PRI_MSG(FAILED, 2, "1");
    ASSERT_PRI_MSG(OK, 1, "88");
    ASSERT_TIMEOUT(CHECKOUT); // player 1 wins
}

GAME_TEST(5, bid_2)
{
    ASSERT_PUB_MSG(OK, 0, "初始金币数 100");
    ASSERT_PUB_MSG(OK, 0, "投标轮数 3");
    START_GAME();

    // item 5
    ASSERT_PRI_MSG(OK, 0, "9");
    ASSERT_PRI_MSG(OK, 1, "10");
    ASSERT_PUB_MSG(FAILED, 2, "pass"); // must be private
    ASSERT_PRI_MSG(OK, 2, "pass"); // cancel
    ASSERT_PRI_MSG(OK, 3, "10"); // modify
    ASSERT_PRI_MSG(CONTINUE, 4, "10"); // player 1 3 4 win
    ASSERT_PRI_MSG(FAILED, 0, "30"); // lose in term 1
    ASSERT_PRI_MSG(FAILED, 2, "30"); // lose in term 1
    ASSERT_PRI_MSG(FAILED, 1, "9"); // should greater than 10
    ASSERT_PRI_MSG(OK, 1, "100");
    ASSERT_PRI_MSG(FAILED, 1, "pass"); // cannot cancel in term 2
    ASSERT_PRI_MSG(FAILED, 2, "11"); // lose in term 1
    ASSERT_PRI_MSG(OK, 3, "10");
    ASSERT_PRI_MSG(CHECKOUT, 4, "11");
}

GAME_TEST(5, do_nothing)
{
    ASSERT_PUB_MSG(OK, 0, "投标轮数 2");
    ASSERT_PUB_MSG(OK, 0, "初始金币数 100");
    ASSERT_PUB_MSG(OK, 0, "回合数 5");
    START_GAME();

    for (int j = 0; j < 8; ++j) {
        ASSERT_TIMEOUT(CHECKOUT); // no bid will checkout directly
    }
    for (int i = 0; i < 5; ++i) {
        //ASSERT_TIMEOUT(CHECKOUT); // discard stage
        for (int j = 0; j < 8; ++j) {
            ASSERT_TIMEOUT(CHECKOUT);
        }
    }

    ASSERT_SCORE(100, 100, 100, 100, 100);
}

GAME_TEST(5, do_nothing_no_items)
{
    ASSERT_PUB_MSG(OK, 0, "投标轮数 2");
    ASSERT_PUB_MSG(OK, 0, "初始金币数 100");
    ASSERT_PUB_MSG(OK, 0, "回合数 5");
    START_GAME();

    for (int j = 0; j < 8; ++j) {
        ASSERT_PRI_MSG(OK, 0, "1");
        ASSERT_TIMEOUT(CHECKOUT);
    }
    for (int i = 0; i < 5; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }

    ASSERT_SCORE(200, 75, 75, 75, 75);
}

// 种子 ABC
// 1号商品：○1 △3 ○6 ☆6 □8
// 2号商品：△1 ☆1 △2 ○4 △9
// 3号商品：□1 ○2 △4 ☆5 △7
// 4号商品：□2 ☆2 □3 ○5 ☆7
// 5号商品：○3 ☆3 ☆4 ○9 ☆X
// 6号商品：□4 ○7 ○8 △8 □9
// 7号商品：△5 □5 □6 □7 △X
// 8号商品：△6 ☆8 ☆9 ○X □X

GAME_TEST(5, discarder_auto_ready)
{
    ASSERT_PUB_MSG(OK, 0, "种子 ABC");
    ASSERT_PUB_MSG(OK, 0, "投标轮数 1");
    ASSERT_PUB_MSG(OK, 0, "初始金币数 100");
    ASSERT_PUB_MSG(OK, 0, "回合数 1");
    START_GAME();

    // 玩家0 中标 5 号商品（△2 ☆6 □7 ○8 ☆X），其余各商品 timeout
    for (int i = 0; i < 4; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }
    ASSERT_PRI_MSG(OK, 0, "50");
    for (int i = 0; i < 4; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }

    ASSERT_PRI_MSG(CHECKOUT, 0, "圆3"); // others need not discard (○3 in item 5)

    for (int i = 1; i < 4; ++i) {
        ASSERT_PRI_MSG(OK, i, "pass");
    }
    ASSERT_PRI_MSG(CHECKOUT, 4, "pass"); // player 0 need not bid
}

GAME_TEST(5, no_coins_auto_ready)
{
    ASSERT_PUB_MSG(OK, 0, "种子 ABC");
    ASSERT_PUB_MSG(OK, 0, "投标轮数 1");
    ASSERT_PUB_MSG(OK, 0, "初始金币数 100");
    ASSERT_PUB_MSG(OK, 0, "回合数 1");
    START_GAME();

    ASSERT_PRI_MSG(OK, 0, "50");
    ASSERT_TIMEOUT(CHECKOUT);

    ASSERT_PRI_MSG(OK, 1, "100");
    for (int i = 1; i < 8; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }

    ASSERT_PRI_MSG(OK, 0, "圆1");
    ASSERT_TIMEOUT(CHECKOUT);

    for (int i = 2; i < 4; ++i) {
        ASSERT_PRI_MSG(OK, i, "pass");
    }
    ASSERT_PRI_MSG(CHECKOUT, 4, "pass"); // player 0 and 1 need not bid
}

GAME_TEST(5, discard_should_ready)
{
    ASSERT_PUB_MSG(OK, 0, "种子 ABC");
    ASSERT_PUB_MSG(OK, 0, "投标轮数 1");
    ASSERT_PUB_MSG(OK, 0, "初始金币数 100");
    ASSERT_PUB_MSG(OK, 0, "回合数 2");
    START_GAME();

    ASSERT_PRI_MSG(OK, 0, "50");
    for (int i = 0; i < 8; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }

    ASSERT_PRI_MSG(CHECKOUT, 0, "圆1");

    ASSERT_PRI_MSG(OK, 1, "50");
    for (int i = 0; i < 8; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }

    ASSERT_PRI_MSG(OK, 0, "三角3");
    ASSERT_PRI_MSG(CHECKOUT, 1, "圆1");
}

GAME_TEST(5, discard_1)
{
    ASSERT_PUB_MSG(OK, 0, "种子 ABC");
    ASSERT_PUB_MSG(OK, 0, "投标轮数 1");
    ASSERT_PUB_MSG(OK, 0, "初始金币数 100");
    ASSERT_PUB_MSG(OK, 0, "回合数 1");
    START_GAME();

    for (int i = 0; i < 5; ++i) {
        ASSERT_PRI_MSG(OK, i, "50");
        ASSERT_TIMEOUT(CHECKOUT);
    }
    for (int i = 0; i < 3; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }

    ASSERT_TIMEOUT(CHECKOUT);
    for (int i = 0; i < 3; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }

    ASSERT_SCORE(182, 66, 53, 83, 116);
}

GAME_TEST(5, discard_2)
{
    ASSERT_PUB_MSG(OK, 0, "种子 ABC");
    ASSERT_PUB_MSG(OK, 0, "投标轮数 1");
    ASSERT_PUB_MSG(OK, 0, "初始金币数 100");
    ASSERT_PUB_MSG(OK, 0, "回合数 2");
    START_GAME();

    ASSERT_PRI_MSG(OK, 0, "1"); // pool coins 1
    ASSERT_TIMEOUT(CHECKOUT);
    for (int i = 0; i < 7; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }

    // round 1
    ASSERT_PRI_MSG(FAILED, 0, "方1"); // □1 not in item 1 (○1 ☆2 ☆3 △6 ○9)
    ASSERT_PRI_MSG(CHECKOUT, 0, "圆1 星2 星3"); // ○1 ☆2 ☆3 all in item 1

    ASSERT_PRI_MSG(FAILED, 0, "1"); // cannot bid own item
    ASSERT_PRI_MSG(FAILED, 0, "pass"); // cannot cancel own item
    ASSERT_PRI_MSG(OK, 1, "50");
    ASSERT_TIMEOUT(CHECKOUT);
    ASSERT_PRI_MSG(OK, 0, "101"); // pool coins 1 + 101 = 102
    ASSERT_TIMEOUT(CHECKOUT);
    ASSERT_PRI_MSG(OK, 2, "1"); // pool coins 102 + 1 = 103
    ASSERT_TIMEOUT(CHECKOUT);
    for (int i = 0; i < 5; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }

    // round 2
    // player 1 has {○1 ☆2 ☆3}, player 0 has {△6 ○9}, player 2 has item 2 {△1 ○3 ○5 ○6 ☆9}
    ASSERT_PRI_MSG(FAILED, 1, "弃牌"); // empty discard
    ASSERT_PRI_MSG(FAILED, 1, "今天天气真好啊");
    ASSERT_PRI_MSG(FAILED, 1, "方1"); // □1 not in player 1's item {○1 ☆2 ☆3}
    ASSERT_PRI_MSG(OK, 1, "圆1 星2 星3"); // ○1 ☆2 ☆3 all in player 1's item
    ASSERT_PRI_MSG(FAILED, 1, "圆5"); // ○5 not in player 1's item
    ASSERT_PUB_MSG(FAILED, 1, "pass");
    ASSERT_PRI_MSG(OK, 0, "pass");
    ASSERT_PRI_MSG(OK, 1, "pass");
    ASSERT_PRI_MSG(OK, 3, "pass");
    ASSERT_PRI_MSG(OK, 4, "pass");
    ASSERT_PRI_MSG(CHECKOUT, 2, "星1"); // ☆1 in player 2's item {□1 ☆1 □2 ☆8 △X}

    ASSERT_PRI_MSG(OK, 0, "赛况");

    for (int i = 0; i < 6; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }

    ASSERT_SCORE(48 + 83, 37, 99 + 83, 75, 75);
}

GAME_TEST(5, no_bid_return_item)
{
    ASSERT_PUB_MSG(OK, 0, "种子 ABC");
    ASSERT_PUB_MSG(OK, 0, "投标轮数 1");
    ASSERT_PUB_MSG(OK, 0, "初始金币数 100");
    ASSERT_PUB_MSG(OK, 0, "回合数 2");
    START_GAME();

    ASSERT_PRI_MSG(OK, 0, "50");
    for (int i = 0; i < 8; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }

    ASSERT_PRI_MSG(CHECKOUT, 0, "圆1");

    for (int i = 0; i < 8; ++i) {
        ASSERT_TIMEOUT(CHECKOUT);
    }

    ASSERT_PRI_MSG(CHECKOUT, 0, "星2");
}

} // namespace GAME_MODULE_NAME

} // namespace game

} // gamespace lgtbot

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    return RUN_ALL_TESTS();
}
