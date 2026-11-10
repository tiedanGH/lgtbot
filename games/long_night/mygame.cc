// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).

#include <memory>
#include <random>
#include <queue>
#include <unordered_set>

#include "game_framework/stage.h"
#include "game_framework/util.h"
#include "utility/html.h"

using namespace std;

#include "common/constants.h"
#include "actor/player.h"
#include "map/grid.h"
#include "map/map.h"
#include "actor/boss.h"
#include "map/board.h"

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

class MainStage;
template <typename... SubStages> using SubGameStage = StageFsm<MainStage, SubStages...>;
template <typename... SubStages> using MainGameStage = StageFsm<void, SubStages...>;
const GameProperties k_properties { 
    .name_ = "漫漫长夜",
    .developer_ = "铁蛋",
    .description_ = "在漆黑的迷宫中探索，根据有限的线索展开追击与逃生",
    .shuffled_player_id_ = true,
};
uint64_t MaxPlayerNum(const CustomOptions& options) { return 8; }
uint32_t Multiple(const CustomOptions& options) {
    const auto& blocks = GET_OPTION_VALUE(options, 区块);
    if (blocks.empty() || blocks[0] != "默认" || GET_OPTION_VALUE(options, 教程))
        return 0;
    return 2;
}
const MutableGenericOptions k_default_generic_options{};

// 规则文本：机制、模式、事件与更新见 common/constants.h 的 rule_details，计分取自 actor/player.h 的 Score，BOSS 规则见 actor/boss.h 各 BOSS 类的 k_rule
const std::vector<RuleCommand> k_rule_commands = {
    RuleCommand("游戏规则细节和机制：「" META_COMMAND_SIGN "规则 漫漫长夜 机制」查看可用列表",
            [](const int type) -> const char* const
            {
                static const string score_rule = Score::ScoreRuleText();
                return type == 8 ? score_rule.c_str() : rule_details[type];
            },
            AlterChecker<int>({{"机制", 0}, {"地形", 1}, {"传送", 2}, {"成就", 3}, {"区块", 4}, {"模式", 5}, {"事件", 6}, {"更新", 7}, {"计分", 8}})),
    RuleCommand("查看 BOSS 列表与简介，附带名称时查看规则和技能详情",
            [](const string& name) -> const char* const
            {
                static string rule_text;
                rule_text = GetBossRuleText(name);
                return rule_text.c_str();
            },
            VoidChecker("BOSS", "boss"), OptionalDefaultChecker<AnyArg>("", "BOSS名称", "暴君")),
};

// [新手教程] 开局校验人数并改为教程固定配置（定义见 stage/tutorial_stage.h）
bool AdaptTutorialOptions(lgtbot::ChildMsgSenderBase& reply, CustomOptions& game_options, const GenericOptions& generic_options_readonly);

bool AdaptOptions(lgtbot::ChildMsgSenderBase& reply, CustomOptions& game_options, const GenericOptions& generic_options_readonly, MutableGenericOptions& generic_options)
{
    if (GET_OPTION_VALUE(game_options, 教程)) return AdaptTutorialOptions(reply, game_options, generic_options_readonly);

    auto& custom_blocks = GET_OPTION_VALUE(game_options, 区块);
    if (custom_blocks.empty()) {
        reply() << "[错误] 区块参数为空：必须包含至少 9 个有效区块代号。形如：1 1 6 16 34 38 E1 e7 S4 s1";
        return false;
    }
    if (custom_blocks[0] != "默认") {
        UnitMaps unitMaps;
        vector<string> vaild_blocks;
        bool has_invalid = false;
        for (auto& block : custom_blocks) {
            bool is_valid;
            std::transform(block.begin(), block.end(), block.begin(), [](unsigned char c) { return std::toupper(c); });
            if (!block.empty() && block[0] == 'E') {
                is_valid = unitMaps.IsBlockExist(block.substr(1), true);
            } else {
                is_valid = unitMaps.IsBlockExist(block, false);
            }
            if (is_valid) vaild_blocks.push_back(block); else has_invalid = true;
        }
        if (generic_options_readonly.PlayerNum() > 6 && vaild_blocks.size() < 16) {
            reply() << "[错误] 区块参数不足：当前玩家数为 " << generic_options_readonly.PlayerNum() << "，必须包含至少 16 个有效区块代号，当前数量为：" << vaild_blocks.size();
            return false;
        }
        if (vaild_blocks.size() < 9) {
            reply() << "[错误] 区块参数不足：必须包含至少 9 个有效区块代号，当前数量为：" << vaild_blocks.size();
            return false;
        }
        GET_OPTION_VALUE(game_options, 模式) = BlockMode::CUSTOM;
        std::sort(vaild_blocks.begin(), vaild_blocks.end(), CompareMapId);
        custom_blocks = vaild_blocks;
        if (GET_OPTION_VALUE(game_options, 拓展)) {
            GET_OPTION_VALUE(game_options, 拓展) = false;
            reply() << "[警告] 自定义区块时使用全部区块作为随机池，拓展模式已自动关闭";
        }
    }

    if (GET_OPTION_VALUE(game_options, 特殊事件) == SpecialEvent::RANDOM) {
        GET_OPTION_VALUE(game_options, 特殊事件) = UnitMaps::GetRandomSpecialEvent();
    }
    // BOSS 列表归一化：仅剔除[无]，允许重复配置（会各自独立生成）
    {
        auto& bosses = GET_OPTION_VALUE(game_options, BOSS);
        std::erase(bosses, BossType::NONE);
        // 超出数量上限时丢弃靠后的多余BOSS
        if (bosses.size() > MAX_BOSS_COUNT) {
            reply() << "[警告] BOSS数量 " << bosses.size() << " 超出上限 " << MAX_BOSS_COUNT << " 个，多余配置已丢弃";
            bosses.resize(MAX_BOSS_COUNT);
        }
    }
    if (generic_options_readonly.PlayerNum() > 6 && GET_OPTION_VALUE(game_options, 边长) < 12) {
        GET_OPTION_VALUE(game_options, 边长) = 12;
        reply() << "[警告] 玩家数 " << generic_options_readonly.PlayerNum() << " 超出普通地图限制，自动将地图边长调整为 12x12";
    }
    if (GET_OPTION_VALUE(game_options, 摸黑) && GET_OPTION_VALUE(game_options, 停止信息) != StopInfo::NONE) {
        GET_OPTION_VALUE(game_options, 停止信息) = StopInfo::NONE;
        reply() << "[警告] 摸黑模式下无法获得墙壁信息，停止信息已调整为「无」";
    }
    return true;
}

// ========== GAME STAGES ==========

class RoundStage;
class TutorialStage;

class MainStage : public MainGameStage<RoundStage, TutorialStage>
{
  public:
    MainStage(StageUtility&& utility)
        : StageFsm(std::move(utility),
            MakeStageCommand(*this, "查看所有区块信息和图例", &MainStage::BlockInfo_, VoidChecker("地图")),
            MakeStageCommand(*this, "指定区块生成地图预览，逃生舱区块需要加 'E' 前缀，未知区块用 0 代替", &MainStage::MapPreview_,
                VoidChecker("预览"), RepeatableChecker<BasicChecker<string>>("序号", "2 3 0 11 E1 0 E2 7 9")))
        , round_(0)
        , player_scores_(Global().PlayerNum(), 0)
        , board(Global().ResourceDir(), GAME_OPTION(纹理), GAME_OPTION(模式), GAME_OPTION(区块), GAME_OPTION(拓展))
        , framework_eliminated_(Global().PlayerNum(), false)
        , is_computer_(Global().PlayerNum(), false)
    {}

    virtual int64_t PlayerScore(const PlayerID pid) const override { return player_scores_[pid]; }

    std::vector<int64_t> player_scores_;

    // 回合数 
	int round_;
    // 地图
    Board board;

    // 多人全员未移动且主动停止/超时结束游戏判定
    bool all_active_stop = true;
    // 无逃生舱最后生还胜利判定
    bool withoutE_win_ = false;
    // 无逃生舱最后生还判定
    bool withE_win_ = false;

    // 框架淘汰状态：所有真人玩家都被淘汰时，主框架进入自动推演
    vector<bool> framework_eliminated_;     // 已调用框架淘汰
    vector<bool> is_computer_;              // 电脑玩家

    // 剩余玩家数
    int Alive_() const { return std::count_if(board.players.begin(), board.players.end(), [](const auto& player){ return player.out == 0; }); }

    // 多人游戏须在公屏行动；[新手教程] 只有 1 名真人，私信也可行动
    bool PublicOnly() const { return Global().PlayerNum() > 1 && !GAME_OPTION(教程); }

    // 淘汰玩家：始终保留至少一名未被框架淘汰的真人玩家，否则主框架进入自动推演吞终局
    void EliminatePlayer(const PlayerID pid)
    {
        if (Global().PlayerNum() <= 1 || framework_eliminated_[pid]) return;     // 单机模式不淘汰
        for (PlayerID other = 0; other < Global().PlayerNum(); ++other) {
            if (other != pid && !framework_eliminated_[other] && !is_computer_[other]) {
                framework_eliminated_[pid] = true;
                Global().Eliminate(pid);
                return;
            }
        }
    }

    // 通用模式提示：特殊事件、静音、摸黑、大地图
    void AppendModeHints(lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
    {
        if (GAME_OPTION(特殊事件) != SpecialEvent::NONE) {
            sender << UnitMaps::ShowSpecialEvent(GAME_OPTION(特殊事件)) << "\n";
        }
        if (GAME_OPTION(静音)) {
            sender << "【静音】一切归于死寂...\n";
        }
        if (GAME_OPTION(摸黑)) {
            sender << "【摸黑】伸手不见五指...\n";
        }
        if (GAME_OPTION(传送紊乱)) {
            sender << "【传送紊乱】斗转星移...\n";
        }
        if (GAME_OPTION(边长) > 9) {
            sender << "本局游戏地图为 " << GAME_OPTION(边长) << "x" << GAME_OPTION(边长) << "\n";
        }
    }

    // [BOSS] 查看赛况、地图时显示在上方的提示
    void AppendBossHints(lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
    {
        for (const auto& boss : board.bosses) {
            if (const string hint = boss->GetStatusHint(); !hint.empty()) sender << hint << "\n";
        }
    }

  private:
    CompReqErrCode BlockInfo_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply)
    {
        auto sender = reply();
        AppendBossHints(sender);
        AppendModeHints(sender);
        sender << Markdown(board.GetAllBlocksInfo(GAME_OPTION(特殊事件), GAME_OPTION(炸弹) > 0), (GRID_SIZE + WALL_SIZE) * 16 + 40);
        return StageErrCode::OK;
    }

    // 记录电脑玩家
    virtual CompReqErrCode OnComputerAct(const PlayerID pid, lgtbot::ChildMsgSenderBase& reply) override
    {
        is_computer_[pid] = true;
        return StageErrCode::OK;
    }

    CompReqErrCode MapPreview_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply, const vector<string>& map_string)
    {
        vector<string> map_str = map_string;
        map_str.resize(board.unitMaps.pos.size(), "0");
        reply() << Markdown(board.MapPreview(map_str), (GRID_SIZE + WALL_SIZE) * (GAME_OPTION(边长) + 1));
        return StageErrCode::OK;
    }
    
    void FirstStageFsm(SubStageFsmSetter setter) override
    {
        srand((unsigned int)time(NULL));
        if (GAME_OPTION(教程)) {
            setter.Emplace<TutorialStage>(*this);
            return;
        }

        auto sender = Global().Boardcast();
        if (GAME_OPTION(捕捉目标) == Target::NEXT) {
            sender << "【捕捉顺位】本局玩家的捕捉顺位为相反顺序，捕捉目标变更为下家\n\n";
        }
        if (GAME_OPTION(特殊事件) != SpecialEvent::NONE) {      // 特殊事件
            switch (GAME_OPTION(特殊事件)) {
                case SpecialEvent::LAZYGARDENER:    board.unitMaps.SpecialEvent1(); break;
                case SpecialEvent::OVERGROWTH:      board.unitMaps.SpecialEvent2(); break;
                case SpecialEvent::RAINSTORY:       board.unitMaps.SpecialEvent3(); break;
                case SpecialEvent::AMBUSH:          board.unitMaps.SpecialEvent4(); break;
                case SpecialEvent::NONE: case SpecialEvent::RANDOM: break;
            }
            sender << UnitMaps::ShowSpecialEvent(GAME_OPTION(特殊事件)) << "\n\n";
        }
        if (GAME_OPTION(静音)) {      // 静音模式
            board.unitMaps.MarkSilenceMode();
            sender << "【静音】一切归于死寂...游戏中**不存在任何声响**。路过声响地形不会通知任何人（包括自己），巨大的心脏与BOSS同样被静音。\n\n";
        }
        if (GAME_OPTION(摸黑)) {      // 摸黑模式
            sender << "【摸黑】伸手不见五指...游戏中**无法获得任何墙壁信息**。开局、撞墙与停止均不会告知四周墙壁，BOSS公开的墙壁信息不受影响。\n\n";
        }
        if (GAME_OPTION(传送紊乱)) {      // 传送紊乱模式
            board.unitMaps.MarkPortalChaos();
            sender <<"【传送紊乱】斗转星移...地图中**所有传送门打乱传送目标**，一一对应组成一个大环，从任意传送门进入都会传送至环中的下一个传送门。\n\n";
        }
        if (GAME_OPTION(边长) == 10) {      // 边长10
            board.unitMaps.RandomizeBlockPosition(GAME_OPTION(边长));
            sender << "【10x10】本局游戏地图将更改为 " << GAME_OPTION(边长) << "x" << GAME_OPTION(边长) << " 大地图。9个区块在大地图随机排列，区块不会重叠。没有区块覆盖的地图空隙将变成普通道路。\n";
        }
        if (GAME_OPTION(边长) == 11) {      // 边长11
            board.unitMaps.SetMapPosition11();
            sender << "【11x11】本局游戏地图将更改为 " << GAME_OPTION(边长) << "x" << GAME_OPTION(边长) << " 大地图。9 个区块之间隔着横竖各 2 条大走廊，地图边缘两侧的区块仍直接相邻，地图中心为逃生舱\n";
        }
        if (GAME_OPTION(边长) == 12) {      // 边长12
            board.unitMaps.SetMapPosition12();
            board.exit_num = 4;
            sender << "【12x12】本局游戏地图将更改为 " << GAME_OPTION(边长) << "x" << GAME_OPTION(边长) << " 大地图。使用 16 个区块铺满地图，逃生舱固定为 4 个。\n";
        }
        if (GAME_OPTION(拓展)) {    // 区块拓展
            sender << "【区块拓展】本局随机池为 " << board.unitMaps.maps.size() << "+" << board.unitMaps.exits.size() << "\n";
        }
        if (!GAME_OPTION(点杀)) {   // 关闭点杀
            sender << "【关闭点杀】仅需路过即可触发捕捉，回合结束也会触发捕捉\n";
        }
        if (GAME_OPTION(隐匿) == HideMode::TURN) {   // 隐匿
            sender << "【隐匿模式】回合隐匿：隐匿效果持续1回合，**仅可使用1次**，隐匿后当回合的行动转为私聊进行，不会发出声响，不会触发捕捉。\n";
        } else if (GAME_OPTION(隐匿) == HideMode::STEP) {
            sender << "【隐匿模式】单步隐匿：隐匿效果仅作用于下一步，**可使用" << HIDE_LIMIT << "次**，隐匿后在私聊行动一步，不会发出声响，不会触发捕捉。\n";
        }
        if (GAME_OPTION(大乱斗)) {  // 大乱斗
            sender << "【大乱斗模式】所有的逃生舱改为随机传送！但仍会统计逃生分\n";
        }
        if (GAME_OPTION(炸弹) > 0) {    // 炸弹人
            sender << "【炸弹人模式】玩家可在公屏安置炸弹，经过并离开会爆炸立即出局并-100分。在炸弹上结束行动可拆除炸弹\n";
        }

        board.size = GAME_OPTION(边长);
        // 初始化玩家
        board.playerNum = Global().PlayerNum();
        board.players.reserve(board.playerNum);
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            board.players.emplace_back(pid, StripNameBrackets(Global().PlayerName(pid)), Global().PlayerAvatar(pid, 40), board.size, board.unitMaps.pos);
            if (GAME_OPTION(隐匿) == HideMode::TURN) {   // 隐匿
                board.players[pid].hide_remaining = 1;
            } else if (GAME_OPTION(隐匿) == HideMode::STEP) {
                board.players[pid].hide_remaining = HIDE_LIMIT;
            }
            if (GAME_OPTION(炸弹) > 0) board.players[pid].bomb = GAME_OPTION(炸弹); // 炸弹人
        }
        board.UpdatePlayerTarget(GAME_OPTION(捕捉目标));
        // 出口数
        if (GAME_OPTION(边长) != 12) {
            if (Global().PlayerNum() > 1) {
                board.exit_num = Global().PlayerNum() / 2;
            } else {
                board.exit_num = 1;
            }
        }
        // 单机模式
        if (Global().PlayerNum() == 1) board.players[0].target = 100;
        // 初始化地图
        board.Initialize();
        // [传送紊乱] 打乱传送门
        if (GAME_OPTION(传送紊乱)) board.ShufflePortals();
        board.exit_num = board.TypeCount(GridType::EXIT);

        // 创建全部BOSS（允许同类型多只，同类型自动编号）
        board.bosses = CreateBosses(GAME_OPTION(BOSS), board.size, board.players);
        // 初始化并播报
        for (auto& boss : board.bosses) {
            boss->BossInitialize();
            boss->InitBossStartRecord();
            sender << "【BOSS】" + boss->GetBossStartInfo() + "\n";
            if (boss->HasTarget()) {
                sender << "当前 " << boss->GetBossIcon() << boss->GetBossName() << " 锁定的玩家为 " << At(boss->target) << "\n";
            }
        }

        board.SaveGameStartMap();   // 保存初始盘面

        sender << "[提示] 本局游戏人数为 " << board.playerNum << " 人，逃生舱数量为 " << board.exit_num << " 个。"
               << (GAME_OPTION(摸黑) ? "" : "请留意私信发送的开局墙壁信息") << "\n\n";
        sender << "【区块模式】" << mode_str[static_cast<int>(GAME_OPTION(模式))] << "\n";
        sender << "本局可能出现的区块详见下图所示：\n";
        sender << Markdown(board.GetAllBlocksInfo(GAME_OPTION(特殊事件), GAME_OPTION(炸弹) > 0), (GRID_SIZE + WALL_SIZE) * 16 + 40);

        setter.Emplace<RoundStage>(*this, ++round_);
    }

    // [新手教程] 教程结束即游戏结束：不计分、不结算成就
    void NextStageFsm(TutorialStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter) override {}

    void NextStageFsm(RoundStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter) override
    {
        bool game_end =
            ((Alive_() <= 1 || all_active_stop) && Global().PlayerNum() > 1) ||
            ((Alive_() <= 0 || board.TypeCount(GridType::EXIT) == 0) && Global().PlayerNum() == 1);
        if (!game_end && round_ < GAME_OPTION(回合数)) {
            setter.Emplace<RoundStage>(*this, ++round_);
            return;
        }

        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            player_scores_[pid] = board.players[pid].score.FinalScore();
            board.players[pid].coins_public = board.players[pid].coins;  // 终局公开全部金币持有量
        }

        if (game_end) {
            if (Global().PlayerNum() > 1) {
                const bool all_eliminated = std::all_of(board.players.begin(), board.players.end(), [](const auto& player){ return player.out == 1; });
                if (all_eliminated) {
                    Global().Boardcast() << "所有玩家均被淘汰，游戏结束！";
                } else if (all_active_stop) {
                    Global().Boardcast() << "所有玩家均主动停止且未进行移动，游戏结束！";
                } else {
                    Global().Boardcast() << "玩家剩余 " + to_string(Alive_()) + " 人，游戏结束！";
                }
            } else if (board.players[0].out == 2) {
                Global().Boardcast() << "经过 " + to_string(round_) + " 回合，您成功抵达了逃生舱！";
            }
        } else {
            Global().Boardcast() << "回合数到达上限，游戏结束";
        }
        Global().Boardcast() << "完整行动轨迹：\n" << Markdown(board.GetAllRecordHtml(-1, false), 500);
        Global().Boardcast() << "玩家分数细则：\n" << Markdown(board.GetAllScore(), 900);
        Global().Boardcast() << Markdown(board.GetFinalBoard(), (GRID_SIZE + WALL_SIZE) * 2 * (GAME_OPTION(边长) + 1));

        // 成就结算
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            const Player& player = board.players[pid];
            const PlayerAchievement& achievement = player.achievement;
            if (achievement.exit_without_sound)         Global().Achieve(pid, Achievement::悄无声息);
            if (achievement.explore_all_map)            Global().Achieve(pid, Achievement::环游世界);
            if (achievement.explore_all_map_silently()) Global().Achieve(pid, Achievement::游荡幽灵);
            if (achievement.exit_first_round)           Global().Achieve(pid, Achievement::我赶时间);
            if (achievement.catch_first_round)          Global().Achieve(pid, Achievement::饥渴难耐);
            if (achievement.visit_five_grid_type())     Global().Achieve(pid, Achievement::乒铃乓啷);
            if (achievement.catch_everyone_4p)          Global().Achieve(pid, Achievement::嗜杀成性);
            if (achievement.catch_without_moving)       Global().Achieve(pid, Achievement::守株待兔);
            if (achievement.boss_chase_four_steps)      Global().Achieve(pid, Achievement::牛头魅魔);
            if (achievement.defuse_four_bombs())        Global().Achieve(pid, Achievement::拆弹专家);
            if (achievement.regicide_alive(player.out)) Global().Achieve(pid, Achievement::弑君者);
        }
    }
};

class RoundStage : public SubGameStage<>
{
  public:
    RoundStage(MainStage& main_stage, const uint64_t round)
            : StageFsm(main_stage, "第 " + std::to_string(round) + " 回合" ,
                MakeStageCommand(*this, "选择方向，在迷宫中探路", &RoundStage::MakeMove_, AlterChecker<Direct>(direction_map)),
                MakeStageCommand(*this, "主动停止行动，并结束回合", &RoundStage::Stop_, VoidChecker("停止")),
                MakeStageCommand(*this, "[隐匿模式] 使用“隐匿”技能", &RoundStage::Hide_, VoidChecker("隐匿")),
                MakeStageCommand(*this, "[炸弹人模式] 在当前位置安放炸弹", &RoundStage::SetBomb_, VoidChecker("下包")),
                MakeStageCommand(*this, "查看当前回合进展情况", &RoundStage::Status_, VoidChecker("赛况"), OptionalDefaultChecker<BoolChecker>(true, "图片", "文字")),
                MakeStageCommand(*this, "查看所有玩家完整行动轨迹", &RoundStage::AllStatus_, VoidChecker("完整赛况"), OptionalDefaultChecker<BoolChecker>(true, "图片", "文字")),
                MakeStageCommand(*this, "多步行动：一次性输入多个方向，自动在迷宫中移动", &RoundStage::MakeMultipleMove_, AnyArg("连续多个方向", "上下左右sxzyUDLR")))
    {
        main_stage.all_active_stop = true;

        currentPlayer = 0;
        if (Main().Alive_() == 0) {
            Global().Boardcast() << "[错误] 发生了意料之外的错误：无可行动玩家但游戏仍未判定结束，请联系管理员或中断游戏（切勿退出强制）！";
            return;
        }
        while (Main().board.players[currentPlayer].out > 0) {
            currentPlayer = currentPlayer + 1;
        }
        ResetTurnState();
        door_modified = 0;
        heart_jammer_notified.assign(Global().PlayerNum(), false);
    }

    // 当前行动玩家
    PlayerID currentPlayer;
    // 玩家行动临时变量
    int step;           // 行动步数
    bool hide;          // 隐匿状态
    bool active_stop;   // 停止行动：主动停止、超时或[谋定后动]结束行动
    bool is_acting;     // 玩家是否开始行动（时限/挂机判定）
    int last_direct;    // 本回合上一次成功移动方向（-1 无，用于树篱冲刺）
    TurnRecord turn_record;     // 本次行动统计（玩家结束行动时交给 BOSS 判定）

    // 记录门的状态发生改变的次数
    int door_modified;
    // [巨大的心脏] 屏蔽器范围玩家本回合是否已收到心跳提示
    vector<bool> heart_jammer_notified;

    virtual void OnStageBegin() override
    {
        Global().SaveMarkdown(Main().board.GetBoard(Main().board.grid_map), (GRID_SIZE + WALL_SIZE) * (GAME_OPTION(边长) + 1));

        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            Main().board.players[pid].ClearMoveRecord(Main().round_);
            if (pid != currentPlayer) {
                Global().SetReady(pid);
            }
        }
        Global().Boardcast() << Markdown(Main().board.GetPlayerTable(Main().round_));
        // [BOSS] 回合开始行动
        for (const auto& boss : Main().board.bosses) {
            boss->HandleRoundBegin(*this);
        }

        StartNextPlayerTurn(Main().board.players[currentPlayer]);

        // 首轮信息播报
        if (Main().round_ == 1) {
            // 开局私信墙壁信息，[摸黑]不发送
            if (!GAME_OPTION(摸黑)) {
                for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
                    auto [info, md] = Main().board.GetSurroundingWalls(pid);
                    Main().board.players[pid].private_record = "【开局】\n您所在位置的四周墙壁信息，按照 上下左右 顺序分别是：\n" + info;
                    Global().Tell(pid) << Main().board.players[pid].private_record << "\n" << Markdown(md, (GRID_SIZE + WALL_SIZE * 2) + 40);
                }
            }
            // [BOSS-米诺陶斯] 开局声音
            if (!GAME_OPTION(静音)) {
                for (const auto& boss : Main().board.bosses) {
                    if (boss->Is(BossType::MINOTAUR)) {
                        SendSoundMessage(boss->x, boss->y, Sound::BOSS, true, false, boss.get());
                    }
                }
            }
            // 开局帮助和模式信息播报
            const char* stop_info_msg;
            switch (GAME_OPTION(停止信息)) {
                case StopInfo::NONE:    stop_info_msg = "【无停止信息】主动停止或超时将无法得知四周墙壁信息"; break;
                case StopInfo::PRIVATE: stop_info_msg = "【私信停止信息】主动停止或超时可以获得私信四周墙壁信息"; break;
                case StopInfo::PUBLIC:  stop_info_msg = "【公开停止信息】主动停止或超时时将在公屏公开四周墙壁信息"; break;
            }
            if (GAME_OPTION(摸黑)) stop_info_msg = "【摸黑】开局、撞墙与停止均无法得知四周墙壁信息";
            Global().Boardcast() << "网页草稿本：" << DRAFT_URL << "\n\n"
                                 << "指令「预览」可生成自定义地图来记录草稿，格式例如：预览 2 3 0 11 E1 0 E2（E前缀表示逃生舱）\n\n"
                                 << "私信「完整赛况」可查询完整的私信信息汇总，包括其他玩家的声响方向历史记录\n\n"
                                 << (GAME_OPTION(谋定后动) ? "【谋定后动】每回合仅能执行一次移动，可使用多步行动指令\n" : "")
                                 << stop_info_msg;
        }
    }

  protected:
    // [新手教程] 跳过本回合无需行动的电脑
    virtual bool SkipTurn(const PlayerID pid) const { return false; }
    // [新手教程] 每步移动后（行动未结束）追加讲解
    virtual void AfterStep(Player& player, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender) {}

    // 重置行动回合的临时状态
    void ResetTurnState()
    {
        step = 0;
        hide = false;
        active_stop = false;
        is_acting = false;
        last_direct = -1;
        turn_record = TurnRecord{};
    }

    // 行动结束四周墙壁信息发送方式：[摸黑]不发送，撞墙、捕捉等非停止结束私信发送，停止行动按「停止信息」
    StopInfo TurnEndWalls() const
    {
        if (GAME_OPTION(摸黑)) return StopInfo::NONE;
        if (!active_stop) return StopInfo::PRIVATE;
        return GAME_OPTION(停止信息);
    }

    // 停止行动时提示墙壁信息的获得方式，公开时直接附上四周墙壁
    void AppendStopWalls(Player& player, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
    {
        switch (TurnEndWalls()) {
            case StopInfo::NONE:
                if (!GAME_OPTION(摸黑)) sender << "停止行动无法获得四周墙壁信息";
                break;
            case StopInfo::PRIVATE:
                sender << "请留意机器人私信发送的四周墙壁信息";
                break;
            case StopInfo::PUBLIC:
                sender << "停止行动将公开四周墙壁信息\n\n";
                AppendSurroundingWalls(player, sender);
                break;
        }
    }

    // [谋定后动] 移动后强制停止行动，记入完整赛况
    void DeliberateStop(Player& player, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
    {
        step++;
        active_stop = true;
        if (!hide) player.NewContentRecord("(谋定后动)");
        sender << "\n\n【谋定后动】移动后结束行动，本回合结束！";
        AppendStopWalls(player, sender);
    }

    // 当前玩家开始行动并启动计时
    void StartNextPlayerTurn(const Player& player)
    {
        if (GAME_OPTION(谋定后动)) {
            uint32_t think_time = player.hook_status ? 30 : GAME_OPTION(行动时限);
            Global().Boardcast() << "请 " << At(currentPlayer) << " 在公屏选择方向移动，仅能移动一次（时限 " << think_time << " 秒，剩余加时卡 " << player.extra_time_card << " 张）";
            Global().StartTimer(think_time);
            return;
        }
        uint32_t think_time = player.hook_status ? 30 : GAME_OPTION(思考时限);
        Global().Boardcast() << "请 " << At(currentPlayer) << " 在公屏选择方向移动（等待时间 " << think_time << " 秒，执行任意指令后获得额外时间 "
                             << GAME_OPTION(行动时限) << " 秒，剩余加时卡 " << player.extra_time_card << " 张）"
                             << (player.bomb > 0 ? "，剩余炸弹 " + to_string(player.bomb) + " 个" : "");
        Global().StartTimer(think_time);
    }

    void ActivatePlayerMovingTimer(const PlayerID pid)
    {
        // 解除挂机状态
        Main().board.players[pid].hook_status = false;
        // 如果是当前玩家，视为开始行动
        if (pid == currentPlayer && !is_acting) {
            is_acting = true;
            Global().StartTimer(TimerLeft() + GAME_OPTION(行动时限));
        }
    }

    bool CheckCommon(const PlayerID pid, lgtbot::ChildMsgSenderBase& reply)
    {
        Player& player = Main().board.players[pid];
        ActivatePlayerMovingTimer(pid);
        if (player.out == 2) {
            reply() << "您已乘坐逃生舱撤离，无需继续行动";
            return false;
        }
        if (pid != currentPlayer) {
            reply() << "[错误] 不是您的回合，当前正在行动玩家是：" << Global().PlayerName(currentPlayer);
            return false;
        }
        return true;
    }

    AtomReqErrCode MakeMove_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply, Direct direction)
    {
        if (!CheckCommon(pid, reply)) return StageErrCode::FAILED;

        Player& player = Main().board.players[pid];
        if (!is_public && Main().PublicOnly()) {
            if (GAME_OPTION(隐匿) == HideMode::NONE) {
                reply() << "[错误] 多人游戏下，请全程在公屏进行行动";
                return StageErrCode::FAILED;
            }
            if (!hide) {
                reply() << "[错误] 您未使用隐匿技能，请在公屏进行行动";
                return StageErrCode::FAILED;
            }
        }
        
        auto sender = reply();
        if (MoveOneStep(player, direction, sender, false)) return StageErrCode::READY;
        // Global().SaveMarkdown(Main().board.GetBoard(Main().board.grid_map), (GRID_SIZE + WALL_SIZE) * (GAME_OPTION(边长) + 1));   // 调试：单步调试用

        if (GAME_OPTION(谋定后动)) {
            DeliberateStop(player, sender);
            return StageErrCode::READY;
        }

        // 继续行动
        sender << "\n\n请继续行动（剩余时间 " << TimerLeft() << " 秒）";
        // 单步隐匿：消除隐匿状态
        if (GAME_OPTION(隐匿) == HideMode::STEP && hide) { hide = false; }
        return StageErrCode::OK;
    }

    AtomReqErrCode MakeMultipleMove_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply, const string& direction_str)
    {
        if (!CheckCommon(pid, reply)) return StageErrCode::FAILED;

        Player& player = Main().board.players[pid];
        if (hide && GAME_OPTION(隐匿) == HideMode::STEP) {
            reply() << "[错误] 您正处于单步隐匿状态，无法使用多步移动指令，请私信裁判使用常规移动";
            return StageErrCode::FAILED;
        }
        if (!is_public && Main().PublicOnly()) {
            reply() << "[错误] 多人游戏下，多步行动指令只能在公屏使用";
            return StageErrCode::FAILED;
        }

        vector<Direct> directions;
        string result = Board::parseDirections(direction_str, directions);

        if (!result.empty()) {
            reply() << "[错误] 解析失败：检测到未知字符 \"" + result + "\"，仅支持包含：\n上/s/U、下/x/D、左/z/L、右/y/R";
            return StageErrCode::FAILED;
        }

        // 多步行动期间暂停计时：期间倒计时被冻结，行动未结束时再按剩余时间恢复计时
        const int time_left = std::max(TimerLeft(), 1);
        Global().StopTimer();

        for (auto it = directions.begin(); it != directions.end(); ++it) {
            auto sender = reply();
            // 中途撞墙或触发强制停止立即结束行动（回合结束）
            if (MoveOneStep(player, *it, sender, true)) return StageErrCode::READY;

            if (std::next(it) == directions.end()) {
                if (GAME_OPTION(谋定后动)) {
                    DeliberateStop(player, sender);
                    return StageErrCode::READY;
                }

                sender << "\n\n请继续行动（剩余时间 " << time_left << " 秒）";
            }
            // 每步随机延迟
            std::uniform_int_distribution<int> dist(0, 1000);
            int delay_ms = 1000 + dist(Main().board.g);
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
        Global().StartTimer(time_left);     // 执行完毕后恢复计时
        return StageErrCode::OK;
    }

    AtomReqErrCode Stop_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply)
    {
        if (!CheckCommon(pid, reply)) return StageErrCode::FAILED;

        Player& player = Main().board.players[pid];
        if (!is_public && Main().PublicOnly()) {
            if (GAME_OPTION(隐匿) == HideMode::NONE) {
                reply() << "[错误] 请全程在公屏进行行动";
            } else {
                reply() << "[错误] 您未使用隐匿技能，请在公屏进行行动";
            }
            return StageErrCode::FAILED;
        }

        step++;
        active_stop = true;
        if (!hide) player.NewContentRecord("(停止)");
        auto sender = reply();
        sender << "[第 " << step << " 步] 您选择主动停止行动，本回合结束！";
        AppendStopWalls(player, sender);
        return StageErrCode::READY;
    }

    AtomReqErrCode Hide_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply)
    {
        if (GAME_OPTION(隐匿) == HideMode::NONE) {
            reply() << "[错误] 本局游戏未开启隐匿技能";
            return StageErrCode::FAILED;
        }

        if (!CheckCommon(pid, reply)) return StageErrCode::FAILED;

        Player& player = Main().board.players[pid];
        if (hide) {
            reply() << "[错误] 您已经处于隐匿状态，请在私信选择行动";
            return StageErrCode::FAILED;
        }
        if (player.hide_remaining == 0) {
            reply() << "[错误] 隐匿技能次数已耗尽";
            return StageErrCode::FAILED;
        }
        if (!is_public && Main().PublicOnly()) {
            reply() << "[错误] 请在公屏使用隐匿技能";
            return StageErrCode::FAILED;
        }
        
        hide = true;
        player.hide_remaining--;
        if (GAME_OPTION(隐匿) == HideMode::TURN) {
            player.NewContentRecord("<隐匿行动>", "hide");
            reply() << "使用隐匿技能，本回合剩余时间转为私聊行动，不会发出声响，不会触发捕捉。剩余次数：" << player.hide_remaining;
        } else if (GAME_OPTION(隐匿) == HideMode::STEP) {
            player.NewContentRecord("�", "hide");
            reply() << "使用隐匿技能，下一步请在私聊行动，不会发出声响，不会触发捕捉。剩余次数：" << player.hide_remaining;
        }
        return StageErrCode::OK;
    }

    AtomReqErrCode SetBomb_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply)
    {
        if (GAME_OPTION(炸弹) == 0) {
            reply() << "[错误] 本局游戏未开启炸弹人模式";
            return StageErrCode::FAILED;
        }

        if (!CheckCommon(pid, reply)) return StageErrCode::FAILED;

        Player& player = Main().board.players[pid];
        if (player.bomb == 0) {
            reply() << "[错误] 炸弹已用尽。但脚下地雷遍布，请谨慎前行";
            return StageErrCode::FAILED;
        }
        if (!is_public && Main().PublicOnly()) {
            reply() << "[错误] 请在公屏使用下包技能";
            return StageErrCode::FAILED;
        }

        Grid& grid = Main().board.grid_map[player.x][player.y];
        if (grid.Attach() == AttachType::EMPTY) grid.SetAttach(AttachType::BOMB);

        player.bomb--;
        player.NewContentRecord("[下包]", "bomb");
        reply() << "在当前所在位置执行下包操作。剩余：" << player.bomb;
        return StageErrCode::OK;
    }

    AtomReqErrCode Status_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply, const bool show_image)
    {
        ActivatePlayerMovingTimer(pid);
        auto sender = reply();
        Main().AppendBossHints(sender);
        if (show_image) {
            sender << Markdown(Main().board.GetPlayerTable(Main().round_));
        } else {
            sender << Main().board.GetPlayerString() << "\n";
        }

        if (is_public) {
            int query_pid_current = currentPlayer == pid ? -1 : pid.Get();
            for (PlayerID pid = 0; pid < currentPlayer.Get(); ++pid) {
                if (Main().board.players[pid].out == 0 && !SkipTurn(pid)) {
                    sender << "\n[" << pid.Get() << "号]本回合行动轨迹：\n" << Main().board.players[pid].move_record.GetMoveRecord(query_pid_current, is_public);
                }
            }
            sender << "\n[" << currentPlayer.Get() << "号]正在行动中：\n" << Main().board.players[currentPlayer].move_record.GetMoveRecord(query_pid_current, is_public);
        } else {
            sender << "\n" << Main().board.players[pid].private_record;
        }
        return StageErrCode::OK;
    }

    AtomReqErrCode AllStatus_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply, const bool show_image)
    {
        ActivatePlayerMovingTimer(pid);
        auto sender = reply();
        Main().AppendBossHints(sender);
        Main().AppendModeHints(sender);

        int query_pid = pid.Get();
        if (show_image) {
            sender << Markdown(Main().board.GetAllRecordHtml(query_pid, is_public), 550);
        } else {
            sender << Main().board.GetAllRecordString(query_pid, is_public);
        }

        int query_pid_current = currentPlayer == pid ? -1 : pid.Get();
        sender << "\n[" << currentPlayer.Get() << "号]正在行动中：\n" << Main().board.players[currentPlayer].move_record.GetMoveRecord(query_pid_current, is_public);
        if (!is_public) {
            sender << "\n\n" << Main().board.players[pid].private_record;
        }
        return StageErrCode::OK;
    }

    virtual CheckoutErrCode OnStageTimeout() override
    {
        Player& player = Main().board.players[currentPlayer];
        // 使用加时卡延长时限，不结束本回合
        if (is_acting && player.extra_time_card > 0) {
            player.extra_time_card--;
            Global().Boardcast() << "行动超时，自动使用加时卡，剩余时间延长 " + to_string(EXTRATIMECRAD_TIME) + " 秒，剩余 " + to_string(player.extra_time_card) + " 张加时卡";
            Global().StartTimer(EXTRATIMECRAD_TIME);
            return StageErrCode::CONTINUE;
        }
        player.NewContentRecord("(超时)");
        active_stop = true;
        // 用块作用域控制 sender 析构时机，保证超时提示先于回合结算播报
        {
            auto sender = Global().Boardcast();
            if (!is_acting) {
                if (!player.hook_status) {
                    Global().Tell(currentPlayer) << "您已进入挂机状态，等待时间将缩减至 30 秒，执行游戏指令可恢复至原状态";
                }
                player.hook_status = true;
                sender << "玩家 " << At(PlayerID(currentPlayer)) << " 超时未行动，已进入挂机状态，再次行动前仅有 30 秒等待时间";
            } else {
                sender << "玩家 " << At(PlayerID(currentPlayer)) << " 行动超时，切换下一个玩家";
            }
            // PUBLIC 模式：与主动停止对齐，把墙壁信息附加到公屏超时提示
            if (TurnEndWalls() == StopInfo::PUBLIC) {
                sender << "\n\n";
                AppendSurroundingWalls(player, sender);
            }
        }
        Global().SetReady(currentPlayer);
        return HandleStageOver();
    }

    virtual CheckoutErrCode OnPlayerLeave(const PlayerID pid) override
    {
        if (Main().board.players[pid].out > 0) {
            return StageErrCode::CONTINUE;
        }
        Main().board.players[pid].out = 1;
        Main().board.players[pid].score.quit_score -= 300;  // 退出分

        auto sender = Global().Boardcast();
        sender << "玩家 " << At(pid) << " 退出游戏";
        if (Main().Alive_() > 1) {
            sender << "，捕捉目标顺位发生变更！\n";
            RefreshCatchOrder(sender);
            return StageErrCode::CONTINUE;
        }
        return StageErrCode::CHECKOUT;
    }

    virtual CheckoutErrCode OnStageOver() override
    {
        return HandleStageOver();
    }

    CheckoutErrCode HandleStageOver()
    {
        Player& player = Main().board.players[currentPlayer];
        // 回合结束检查捕捉（含[暴君]通缉的玩家）；非点杀已捕捉过则跳过
        if (!turn_record.caught_player && FindCatchVictim(player).has_value() && !hide && player.out == 0) {
            auto sender = Global().Boardcast();
            PlayerCatch(player, sender);
        }
        // [BOSS] 玩家结束行动
        turn_record.hidden = hide;
        for (const auto& boss : Main().board.bosses) {
            boss->HandlePlayerTurnEnd(*this, player, turn_record);
        }
        Grid& grid = Main().board.grid_map[player.x][player.y];

        Global().Boardcast() << "[" << currentPlayer.Get() << "号]玩家本回合的完整行动轨迹：\n" << player.move_record.GetMoveRecord(-1, true);
        // 记录历史行动轨迹
        player.all_record.push_back(player.move_record);
        // 更新全员停止状态（全员未移动且主动停止/超时结束游戏）
        if (!active_stop || turn_record.moves > 0) Main().all_active_stop = false;
        // 仅剩1玩家，游戏结束
        if ((Main().Alive_() == 1 && Global().PlayerNum() > 1) || (Main().Alive_() == 0 && Global().PlayerNum() == 1)) {
            AnnounceCoinGains();    // 提前结束也公示本回合金币
            if (Main().withoutE_win_) {     // 无逃生舱最后生还胜利
                Global().Boardcast() << At(currentPlayer) << "\n" << GetRandomHint(withoutE_win_hints);
            }
            if (Main().withE_win_) {        // 有逃生舱但死斗取胜
                Global().Boardcast() << At(currentPlayer) << "\n" << GetRandomHint(withE_win_hints);
            }
            return StageErrCode::CHECKOUT;
        }
        // 私信发送四周墙壁信息
        if (player.out == 0 && TurnEndWalls() == StopInfo::PRIVATE) {
            auto tell = Global().Tell(currentPlayer);
            AppendSurroundingWalls(player, tell);
        }
        // 已触发炸弹才能拆除炸弹
        if (grid.Attach() == AttachType::BOMB && player.bomb_trigger) {
            player.bomb_trigger = false;
            grid.SetAttach(AttachType::EMPTY);
            player.achievement.recordDefuse();  // 成就[拆弹专家]辅助
            Global().Tell(currentPlayer) << "引线熄灭，爆炸未曾发生，你成功拆除了脚下的【炸弹】";
            player.private_record += "\n回合结束时拆除【炸弹】";
        }
        // 重置玩家临时变量
        ResetTurnState();
        // 下一个玩家行动
        do {
            currentPlayer = (currentPlayer + 1) % Global().PlayerNum();
            if (currentPlayer == 0) break;
        } while (Main().board.players[currentPlayer].out > 0 || SkipTurn(currentPlayer));
        if (currentPlayer != 0) {
            StartNextPlayerTurn(Main().board.players[currentPlayer]);
            Global().ClearReady(currentPlayer);
            return StageErrCode::CONTINUE;
        }

        // [回合结束] 所有玩家都行动后结束本回合
        // 金币获得公示
        AnnounceCoinGains();
        // 门变更过进行提示
        if (door_modified > 0) {
            Global().Boardcast() << "【注意】在本回合内，门曾被按钮触发，共发生 " + to_string(door_modified) + " 次变化";
            Main().board.all_extra_record += "<br>【第 " + to_string(Main().round_) + " 回合】门曾被按钮触发，发生 " + to_string(door_modified) + " 次变化";
            door_modified = 0;
        }
        // BOSS相关结算（按配置顺序依次行动）
        for (const auto& boss_ptr : Main().board.bosses) {
            Boss& boss = *boss_ptr;
            string boss_record = "【第 " + to_string(Main().round_) + " 回合】";
            boss.NewRecord("");
            auto sender = Global().Boardcast();
            sender << "【回合结束[" << boss.GetBossIcon() << boss.GetBossName() << " 行动]】";

            boss.HandleRoundAction(*this, boss_record, sender);

            boss.UpdateContentRecord(boss_record);
        }

        return StageErrCode::CHECKOUT;
    }

    virtual AtomReqErrCode OnComputerAct(const PlayerID pid, lgtbot::ChildMsgSenderBase& reply) override
    {
        if (Global().IsReady(pid)) {
            return StageErrCode::OK;
        }
        Main().board.players[pid].out = 1;
        Main().board.players[pid].score.quit_score -= 300;  // 退出分

        auto sender = Global().Boardcast();
        sender << "笨笨的机器人退出了游戏";
        if (Main().Alive_() > 1) {
            sender << "，捕捉目标顺位发生变更！\n";
            RefreshCatchOrder(sender);
        }
        return StageErrCode::READY;
    }

    int TimerLeft() const { return std::chrono::duration_cast<std::chrono::seconds>(*Global().TimerFinishTime() - std::chrono::steady_clock::now()).count(); }

    // ========== 成员函数 ==========
    bool MoveOneStep(Player& player, const Direct direction, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender, const bool multiple_mode);
    bool HandleGridInteraction(Player& player, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender, const bool multiple_mode);
    std::optional<PlayerID> FindCatchVictim(const Player& player);
    bool PlayerCatch(Player& player, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender);
    bool HandleHeartBeat(Player& player, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender);
    void TellBerry(Player& player, const string_view hint, const bool separate);
    void AnnounceCoinGains();
    void AppendSurroundingWalls(Player& player, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender);

  public:
    // 供各 BOSS 的回合结算行动调用（见 actor/boss.h 中的 RoundActionImpl）
    void SendSoundMessage(const int fromX, const int fromY, const Sound sound, const bool to_all,
            const bool is_first_sound = false, Boss* const sound_boss = nullptr);
    void RefreshCatchOrder(lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender);
};

auto* MakeMainStage(MainStageFactory factory) { return factory.Create<MainStage>(); }

} // namespace GAME_MODULE_NAME

} // namespace game

} // namespace lgtbot


#include "stage/tutorial_stage.h"       // 新手教程
#include "stage/init_options.h"         // 开局快捷配置指令
#include "stage/round_stage_impl.h"     // RoundStage 类外成员实现

/* ========== BOSS 回合结算行动 ==========
 * 函数体见 actor/boss.h 的 RoundActionImpl：此处 RoundStage 已定义，转发调用以完成模板实例化 */
void MinotaurBoss::HandleRoundAction(RoundStage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender) { RoundActionImpl(stage, boss_record, sender); }
void BangBangBoss::HandleRoundAction(RoundStage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender) { RoundActionImpl(stage, boss_record, sender); }
void TyrantBoss::HandleRoundAction(RoundStage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender) { RoundActionImpl(stage, boss_record, sender); }
void TyrantBoss::HandleRoundBegin(RoundStage& stage) { RoundBeginImpl(stage); }
void TyrantBoss::HandlePlayerTurnEnd(RoundStage& stage, Player& player, const TurnRecord& turn) { PlayerTurnEndImpl(stage, player, turn); }

