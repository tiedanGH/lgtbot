// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).
//
// This file was generated with the assistance of Claude Code (claude.ai/code).

#include <filesystem>
#include <set>

#include "game_framework/unittest_base.h"

namespace lgtbot {
namespace game {
namespace GAME_MODULE_NAME {

// 注：MainStage::PlayerScore 由「计分」选项决定返回值——排名模式返回淘汰名次分，
// 分数模式返回玩家最终总分。本文件所有用例都在开局前设置「计分 分数」，
// 因此 ASSERT_SCORE 断言的是玩家的盘面总分（含「有1吗」加成）。
// 「皮肤」默认为自选，除专门测试皮肤的用例外都设置「皮肤 默认」跳过该阶段。

// ============================================================================
// 1. 开局校验
// ============================================================================

// 少于 2 人时 AdaptOptions 应拒绝开局
GAME_TEST(1, one_player_rejected) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "皮肤 默认");
    ASSERT_FALSE(StartGame());
}

GAME_TEST(2, two_player_start_ok) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "皮肤 默认");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_TRUE(StartGame());
    ASSERT_FINISHED(false);
}

// ============================================================================
// 2. 放置阶段指令校验
// ============================================================================

// 重复设置应失败
GAME_TEST(2, set_twice_failed) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "皮肤 默认");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_TRUE(StartGame());
    ASSERT_PUB_MSG(OK, 0, "10");
    ASSERT_PUB_MSG(FAILED, 0, "11");
}

// 位置超出 0-19 的范围时参数校验不通过，匹配不到任何指令 → NOT_FOUND
GAME_TEST(2, set_out_of_range_failed) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "皮肤 默认");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_TRUE(StartGame());
    ASSERT_PUB_MSG(NOT_FOUND, 0, "20");
}

// 弃牌（位置 0）与放置都能让回合正常结束；第一回合放 1 张牌不可能成线，双方均为 0 分
GAME_TEST(2, discard_and_place_first_round) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "皮肤 默认");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_TRUE(StartGame());
    ASSERT_PUB_MSG(OK, 0, "1");        // P0 放置
    ASSERT_PUB_MSG(CHECKOUT, 1, "0");  // P1 弃牌，本回合结束
    ASSERT_SCORE(0, 0);
}

// 覆盖放置：同一位置连续放两回合，盘面仍只有 1 张牌 → 不可能成线
GAME_TEST(2, overwrite_same_position) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "皮肤 默认");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_TRUE(StartGame());
    ASSERT_PUB_MSG(OK, 0, "10");  ASSERT_PUB_MSG(CHECKOUT, 1, "0");
    ASSERT_PUB_MSG(OK, 0, "10");  ASSERT_PUB_MSG(CHECKOUT, 1, "0");
    ASSERT_SCORE(0, 0);
}

// ============================================================================
// 3. 完整流程冒烟测试（含选牌轮、对战、淘汰、终局结算）
//    仅验证不同人数 / 血量 / 事件下游戏都能正常跑完，不断言具体分数。
// ============================================================================

#define AUTO_PLAY_FINISH_TEST(player_num, test_name, hp_option, event_option, max_steps) \
GAME_TEST(player_num, test_name) { \
    ASSERT_PUB_MSG(OK, 0, "计分 分数"); \
    ASSERT_PUB_MSG(OK, 0, "皮肤 默认"); \
    ASSERT_PUB_MSG(OK, 0, "种子 test"); \
    ASSERT_PUB_MSG(OK, 0, "事件 " event_option); \
    ASSERT_PUB_MSG(OK, 0, "血量 " hp_option); \
    ASSERT_TRUE(StartGame()); \
    for (int i = 0; i < (max_steps) && !this->main_stage_->IsOver(); ++i) this->TimeoutRequest_(); \
    ASSERT_FINISHED(true); \
}

// 低血量：正常走完淘汰流程（剩 1 人结束）
AUTO_PLAY_FINISH_TEST(2, two_player_low_hp,   "50",  "无", 300)
AUTO_PLAY_FINISH_TEST(4, four_player_low_hp,  "50",  "无", 400)
AUTO_PLAY_FINISH_TEST(8, eight_player_low_hp, "50",  "无", 600)

// 高血量：走"卡池耗尽"结束分支（多人并列存活）
AUTO_PLAY_FINISH_TEST(2, two_player_pool_exhaust,   "1000", "无", 600)
AUTO_PLAY_FINISH_TEST(3, three_player_pool_exhaust, "1000", "无", 600)

// 奇数人数：每轮都会产生一场镜像对战
AUTO_PLAY_FINISH_TEST(3, three_player_mirror_battle, "50", "无", 400)
AUTO_PLAY_FINISH_TEST(5, five_player_mirror_battle,  "50", "无", 500)

// 逐个特殊事件：验证卡池裁剪 / 癞子 / 加分 / 预知都不会破坏流程
AUTO_PLAY_FINISH_TEST(2, event_big_coming,   "500", "大的要来了",   600)
AUTO_PLAY_FINISH_TEST(2, event_polarization, "500", "两极分化",     600)
AUTO_PLAY_FINISH_TEST(2, event_big_gone,     "500", "大的没了",     600)
AUTO_PLAY_FINISH_TEST(2, event_windfall,     "500", "天降恩泽",     600)
AUTO_PLAY_FINISH_TEST(2, event_palette,      "500", "调色盘",       600)
AUTO_PLAY_FINISH_TEST(2, event_valuable_one, "500", "有1吗",        600)
AUTO_PLAY_FINISH_TEST(2, event_foresee,      "500", "小透不算挂",   600)
AUTO_PLAY_FINISH_TEST(2, event_april_fool,   "500", "？？？",       600)

// 4 人 + 随机事件：不指定事件时走随机分支
GAME_TEST(4, four_player_random_event) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "皮肤 默认");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_TRUE(StartGame());
    for (int i = 0; i < 400 && !this->main_stage_->IsOver(); ++i) this->TimeoutRequest_();
    ASSERT_FINISHED(true);
}

// ============================================================================
// 4. 选牌顺序
// ============================================================================

// 顺位模式：第 1~7 回合双方全部弃牌 → 分数恒为 0、对战平局不扣血，
// 进入第 8 回合选牌轮时双方血量与分数完全相同，此时先后完全由「选牌顺序」决定。
// 顺位模式下平局按开局玩家编号升序，应当由 0 号玩家先选、1 号玩家此时无法选牌。
GAME_TEST(2, select_order_sequential_tie_breaks_by_player_id) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "皮肤 默认");
    ASSERT_PUB_MSG(OK, 0, "选牌顺序 顺位");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_PUB_MSG(OK, 0, "血量 500");
    ASSERT_TRUE(StartGame());
    for (int i = 0; i < 7; ++i) {
        ASSERT_PUB_MSG(OK, 0, "0");
        ASSERT_PUB_MSG(CHECKOUT, 1, "0");
    }
    ASSERT_SCORE(0, 0);
    ASSERT_PUB_MSG(FAILED, 1, "1 1");       // 尚未轮到 1 号玩家
    // 0 号玩家先选；选完后阶段交棒给 1 号玩家（StageOver_ 返回 CONTINUE，阶段不结束）
    ASSERT_PUB_MSG(CONTINUE, 0, "1 1");
    for (int i = 0; i < 600 && !this->main_stage_->IsOver(); ++i) this->TimeoutRequest_();
    ASSERT_FINISHED(true);
}

// 顺位模式的多人局：验证选项在人数更多、血量更低（会发生淘汰）时也能跑完整局
GAME_TEST(4, four_player_select_order_sequential) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "皮肤 默认");
    ASSERT_PUB_MSG(OK, 0, "选牌顺序 顺位");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_PUB_MSG(OK, 0, "血量 50");
    ASSERT_TRUE(StartGame());
    for (int i = 0; i < 400 && !this->main_stage_->IsOver(); ++i) this->TimeoutRequest_();
    ASSERT_FINISHED(true);
}

// ============================================================================
// 5. 分数基线（回归基线，改动放置 / 计分 / 卡池逻辑时若此值变化需确认是否预期）
//    auto-play 用「计分 分数、种子 test、事件 无、血量 500」，2 人局，双方每轮都超时
//    按 SeqFill 顺序填入，结果确定。两位玩家的盘面只差首轮那张砖块，因此首轮发牌顺序
//    直接决定两人分数的归属——两种「选牌顺序」各锁一条基线，且都显式指定该选项，
//    避免以后改动默认值时基线被静默改写。
// ============================================================================

GAME_TEST(2, baseline_auto_play_sequential_deal) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "皮肤 默认");
    ASSERT_PUB_MSG(OK, 0, "选牌顺序 顺位");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_PUB_MSG(OK, 0, "血量 500");
    ASSERT_TRUE(StartGame());
    for (int i = 0; i < 600 && !this->main_stage_->IsOver(); ++i) this->TimeoutRequest_();
    ASSERT_FINISHED(true);
    ASSERT_SCORE(15, 6);
}

GAME_TEST(2, baseline_auto_play_random_deal) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "皮肤 默认");
    ASSERT_PUB_MSG(OK, 0, "选牌顺序 随机");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_PUB_MSG(OK, 0, "血量 500");
    ASSERT_TRUE(StartGame());
    for (int i = 0; i < 600 && !this->main_stage_->IsOver(); ++i) this->TimeoutRequest_();
    ASSERT_FINISHED(true);
    ASSERT_SCORE(6, 15);
}

// ============================================================================
// 6. 皮肤
// ============================================================================

// 素材完整性：每个皮肤都要带齐渲染用到的全部图片（导入新皮肤时的第一道检查）
GAME_TEST(1, skin_assets_complete) {
    std::vector<AreaCard> cards = {AreaCard()};
    for (const int a : {3, 4, 8}) for (const int b : {1, 5, 9}) for (const int c : {2, 6, 7}) {
        cards.emplace_back(a, b, c);
    }
    for (const auto& skin : k_skins) {
        std::set<std::string> files;
        for (const auto& card : cards) {
            const auto layers = card.Layers(skin);
            files.insert(layers.begin(), layers.end());
        }
        if (skin.style == SkinStyle::拼装) {
            for (int i = 1; i <= 19; ++i) files.insert("num_" + std::to_string(i));
            for (const char* wall : {"000", "001", "010", "011", "100", "101", "110", "half"}) {
                files.insert(std::string("wall_") + wall);
            }
        } else {
            files.insert("board");
        }
        const std::string dir = SkinImagePath(this->resource_dir_str_, skin);
        for (const auto& file : files) {
            EXPECT_TRUE(std::filesystem::exists(dir + file + ".png")) << "皮肤「" << skin.name << "」缺少 " << dir << file << ".png";
        }
    }
}

// 每个皮肤名都能作为「皮肤」选项的取值（全员使用该皮肤）
GAME_TEST(2, skin_option_accepts_every_name) {
    for (const auto& skin : k_skins) {
        ASSERT_PUB_MSG(OK, 0, "皮肤 " + std::string(skin.name));
    }
}

// 自选（默认）：开局先进入皮肤选择阶段；序号与名称都能选，重复选择被拒绝，全员选完进入第 1 回合
GAME_TEST(2, skin_self_select_by_index_and_name) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_TRUE(StartGame());
    ASSERT_PUB_MSG(OK, 0, "2");
    ASSERT_PUB_MSG(FAILED, 0, "3");          // 已选过
    ASSERT_PUB_MSG(CHECKOUT, 1, "萤火虫");
    ASSERT_PUB_MSG(OK, 0, "1");              // 已进入第 1 回合：放置砖块
    ASSERT_PUB_MSG(CHECKOUT, 1, "0");
    ASSERT_SCORE(0, 0);
}

// 自选超时：未选择的玩家沿用默认皮肤
GAME_TEST(2, skin_self_select_timeout) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_TRUE(StartGame());
    ASSERT_PUB_MSG(OK, 0, "咖波");
    ASSERT_TIMEOUT(CHECKOUT);
    ASSERT_PUB_MSG(OK, 0, "1");
    ASSERT_PUB_MSG(CHECKOUT, 1, "0");
}

// 序号越界、名称不存在：匹配不到任何指令
GAME_TEST(2, skin_self_select_invalid) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_TRUE(StartGame());
    ASSERT_PUB_MSG(NOT_FOUND, 0, "99");
    ASSERT_PUB_MSG(NOT_FOUND, 0, "不存在的皮肤");
}

// 随机：每位玩家随机分配皮肤，8 人局整局正常
GAME_TEST(8, skin_random_eight_players) {
    ASSERT_PUB_MSG(OK, 0, "计分 分数");
    ASSERT_PUB_MSG(OK, 0, "皮肤 随机");
    ASSERT_PUB_MSG(OK, 0, "种子 test");
    ASSERT_PUB_MSG(OK, 0, "事件 无");
    ASSERT_PUB_MSG(OK, 0, "血量 50");
    ASSERT_TRUE(StartGame());
    for (int i = 0; i < 600 && !this->main_stage_->IsOver(); ++i) this->TimeoutRequest_();
    ASSERT_FINISHED(true);
}

// 指定皮肤：每种渲染方案各挑一个皮肤跑完整对局
#define SKIN_FULL_GAME_TEST(test_name, skin_name) \
GAME_TEST(2, test_name) { \
    ASSERT_PUB_MSG(OK, 0, "计分 分数"); \
    ASSERT_PUB_MSG(OK, 0, "皮肤 " skin_name); \
    ASSERT_PUB_MSG(OK, 0, "种子 test"); \
    ASSERT_PUB_MSG(OK, 0, "事件 无"); \
    ASSERT_PUB_MSG(OK, 0, "血量 500"); \
    ASSERT_TRUE(StartGame()); \
    for (int i = 0; i < 600 && !this->main_stage_->IsOver(); ++i) this->TimeoutRequest_(); \
    ASSERT_FINISHED(true); \
}

SKIN_FULL_GAME_TEST(skin_default_full_game,     "默认")
SKIN_FULL_GAME_TEST(skin_green_full_game,       "环保绿")
SKIN_FULL_GAME_TEST(skin_gold_full_game,        "奢华金")
SKIN_FULL_GAME_TEST(skin_firefly_full_game,     "萤火虫")
SKIN_FULL_GAME_TEST(skin_capoo_full_game,       "咖波")
SKIN_FULL_GAME_TEST(skin_hexagon_full_game,     "六边形")
SKIN_FULL_GAME_TEST(skin_mech_full_game,        "未来机甲")
SKIN_FULL_GAME_TEST(skin_rainbow_cat_full_game, "彩虹猫猫教")

} // namespace GAME_MODULE_NAME
} // namespace game
} // namespace lgtbot

int main(int argc, char** argv) { testing::InitGoogleTest(&argc, argv); gflags::ParseCommandLineFlags(&argc, &argv, true); return RUN_ALL_TESTS(); }
