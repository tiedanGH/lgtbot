// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).

#include <array>
#include <map>
#include <functional>
#include <memory>
#include <vector>
#include <random>
#include <algorithm>

#include "game_framework/stage.h"
#include "game_framework/util.h"
#include "utility/html.h"

#include "opencomb.h"

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

class MainStage;
template <typename... SubStages> using SubGameStage = StageFsm<MainStage, SubStages...>;
template <typename... SubStages> using MainGameStage = StageFsm<void, SubStages...>;
const GameProperties k_properties { 
    .name_ = "开放蜂巢",
    .developer_ = "铁蛋",
    .description_ = "使用特殊道具改变经典地图，体验不一样的数字蜂巢玩法",
};
uint64_t MaxPlayerNum(const MyGameOptions& options) { return 10; }
uint32_t Multiple(const MyGameOptions& options) { return GET_OPTION_VALUE(options, 种子).empty() ? 2 : 0; }
const MutableGenericOptions k_default_generic_options;
const std::vector<RuleCommand> k_rule_commands = {};

bool AdaptOptions(MsgSenderBase& reply, MyGameOptions& game_options, const GenericOptions& generic_options_readonly, MutableGenericOptions& generic_options) { return true; }

const std::vector<InitOptionsCommand> k_init_options_commands = {
    InitOptionsCommand("独自一人开始游戏",
            [] (MyGameOptions& game_options, MutableGenericOptions& generic_options)
            {
                generic_options.bench_computers_to_player_num_ = 1;
                return NewGameMode::SINGLE_USER;
            },
            VoidChecker("单机")),
    InitOptionsCommand("设置多人游戏使用的卡池和地图",
            [] (MyGameOptions& game_options, MutableGenericOptions& generic_options, const int32_t& mode, const std::string map)
            {
                GET_OPTION_VALUE(game_options, 卡池) = mode;
                for (size_t i = 0; i < map_string.size(); ++i) {
                    if (map_string[i].first == map) {
                        GET_OPTION_VALUE(game_options, 地图) = i;
                        break;
                    }
                }
                return NewGameMode::MULTIPLE_USERS;
            },
            AlterChecker<uint32_t>({{"经典", 0}, {"癞子", 1}, {"空气", 2}, {"混乱", 3}}), OptionalDefaultChecker<BasicChecker<std::string>>("经典", "地图", "经典")),
};

// ========== GAME STAGES ==========

static const std::array<std::vector<int32_t>, k_direct_max> all_points{
    std::vector<int32_t>{3, 4, 8, 10, 0},
    std::vector<int32_t>{1, 5, 9, 10, 0},
    std::vector<int32_t>{2, 6, 7, 10, 0}
};

struct Player
{
    Player(std::string resource_path, const uint32_t map) : comb_(new OpenComb(std::move(resource_path), map)) {}
    Player(Player&&) = default;
    std::unique_ptr<OpenComb> comb_;
};

class RoundStage;

class MainStage : public MainGameStage<RoundStage>
{
  public:
    MainStage(StageUtility&& utility)
        : StageFsm(std::move(utility))
        , round_(0)
    {
        srand((unsigned int)time(NULL));
        const uint32_t map = GAME_OPTION(地图) == 0 ? rand() % k_map_size : GAME_OPTION(地图) - 1;
        for (uint64_t i = 0; i < Global().PlayerNum(); ++i) {
            players_.emplace_back(Global().ResourceDir(), map);
        }

        // seed
        seed_str = GAME_OPTION(种子);
        if (seed_str.empty()) {
            std::random_device rd;
            std::uniform_int_distribution<unsigned long long> dis;
            seed_str = std::to_string(dis(rd));
        }
        std::seed_seq seed(seed_str.begin(), seed_str.end());
        std::mt19937 g(seed);

        // 卡池模板生成
        std::vector<AreaCard> classical_cards_;
        std::vector<AreaCard> wild_cards_;
        std::vector<AreaCard> air_cards_;
        std::vector<AreaCard> chaos_cards_;
        for (const int32_t point_0 : all_points[0]) {
            for (const int32_t point_1 : all_points[1]) {
                for (const int32_t point_2 : all_points[2]) {
                    int32_t count0 = (point_0 == 0) + (point_1 == 0) + (point_2 == 0);
                    int32_t count10 = (point_0 == 10) + (point_1 == 10) + (point_2 == 10);
                    if (count0 == 0 && count10 == 0) {
                        classical_cards_.emplace_back(point_0, point_1, point_2);
                    }
                    if (count0 == 0 && count10 == 1) {
                        wild_cards_.emplace_back(point_0, point_1, point_2);
                    }
                    if (count0 == 1 && count10 == 0) {
                        air_cards_.emplace_back(point_0, point_1, point_2);
                    }
                    if (count0 > 0 && count10 > 0) {
                        chaos_cards_.emplace_back(point_0, point_1, point_2);
                    }
                }
            }
        }
        
        if (GAME_OPTION(卡池) == 0) {
            cards_.insert(cards_.end(), classical_cards_.begin(), classical_cards_.end());
            cards_.insert(cards_.end(), classical_cards_.begin(), classical_cards_.end());
        } else if (GAME_OPTION(卡池) == 1) {
            cards_.insert(cards_.end(), classical_cards_.begin(), classical_cards_.end());
            cards_.insert(cards_.end(), wild_cards_.begin(), wild_cards_.end());
        } else if (GAME_OPTION(卡池) == 2) {
            cards_.insert(cards_.end(), classical_cards_.begin(), classical_cards_.end());
            cards_.insert(cards_.end(), air_cards_.begin(), air_cards_.end());
        } else if (GAME_OPTION(卡池) == 3) {
            cards_.insert(cards_.end(), chaos_cards_.begin(), chaos_cards_.end());
            std::shuffle(classical_cards_.begin(), classical_cards_.end(), g);
            std::shuffle(wild_cards_.begin(), wild_cards_.end(), g);
            std::shuffle(air_cards_.begin(), air_cards_.end(), g);
            cards_.insert(cards_.end(), classical_cards_.begin(), classical_cards_.begin() + 10);
            cards_.insert(cards_.end(), wild_cards_.begin(), wild_cards_.begin() + 10);
            cards_.insert(cards_.end(), air_cards_.begin(), air_cards_.begin() + 10);
            cards_.emplace_back(0, 0, 0);
            cards_.emplace_back(0, 0, 0);
        }

        for (uint32_t i = 0; i < 2; ++i) cards_.emplace_back(10, 10, 10);
        for (uint32_t i = 0; i < 2; ++i) cards_.emplace_back("wall");
        for (uint32_t i = 0; i < 2; ++i) cards_.emplace_back("wall_broken");
        for (uint32_t i = 0; i < 4; ++i) cards_.emplace_back("erase");
        for (uint32_t i = 0; i < 3; ++i) cards_.emplace_back("move");
        for (uint32_t i = 0; i < 2; ++i) cards_.emplace_back("reshape");

        std::shuffle(cards_.begin(), cards_.end(), g);

        // first round
        const std::string special_cards[5] = {"wall", "wall_broken", "erase", "move", "reshape"};
        std::uniform_int_distribution<int32_t> dist(0, 4);
        int32_t start = dist(g);
        cards_.insert(cards_.begin(), special_cards[start]);

        it_ = cards_.begin();
    }

    virtual void FirstStageFsm(SubStageFsmSetter setter) override;

    virtual void NextStageFsm(RoundStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter) override;

    int64_t PlayerScore(const PlayerID pid) const
    {
        if (GAME_OPTION(连线奖励)) {
            return players_[pid].comb_->Score() + players_[pid].comb_->ExtraScore();
        } else {
            return players_[pid].comb_->Score();
        }
    }

    std::string CombHtml(const std::string& str)
    {
        html::Table table(players_.size() / 2 + 1, 2);
        table.SetTableStyle("align=\"center\" cellpadding=\"20\" cellspacing=\"0\"");
        for (PlayerID pid = 0; pid.Get() < players_.size(); ++pid) {
            table.Get(pid / 2, pid % 2).SetContent("### " + Global().PlayerAvatar(pid, 40) + "&nbsp;&nbsp; " + Global().PlayerName(pid) +
                    "\n\n### " HTML_COLOR_FONT_HEADER(green) "当前积分：" + std::to_string(PlayerScore(pid)) + HTML_FONT_TAIL +
                    (GAME_OPTION(连线奖励) ? (HTML_COLOR_FONT_HEADER(blue) "（" + std::to_string(players_[pid].comb_->Score()) + "+" + std::to_string(players_[pid].comb_->ExtraScore()) + "）" + HTML_FONT_TAIL) : "") +
                    "\n\n" +
                    players_[pid].comb_->ToHtml());
        }
        if (players_.size() % 2) {
            table.MergeRight(table.Row() - 1, 0, 2);
        }
        return str + GetStyle(Global().ResourceDir()) + table.ToString();
    }

    bool CheckGameOver_(const std::vector<Player>& players_) const
    {
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            if (players_[pid].comb_->IsAllFilled()) {
                return true;
            }
        }
        return false;
    }

    std::vector<Player> players_;

    std::string seed_str;

  private:
    void NewStage_(SubStageFsmSetter& setter);

    uint32_t round_;
    std::vector<AreaCard> cards_;
    decltype(cards_)::iterator it_;
};

class RoundStage : public SubGameStage<>
{
  public:
    RoundStage(MainStage& main_stage, const uint64_t round, const AreaCard& card)
            : StageFsm(main_stage, "第" + std::to_string(round) + "回合",
                MakeStageCommand(*this, "放置砖块（或使用道具）", &RoundStage::Set_, ArithChecker<uint32_t>(1, 73, "数字")),
                MakeStageCommand(*this, "使用移动道具", &RoundStage::Move_, ArithChecker<uint32_t>(1, 73, "起始"), ArithChecker<uint32_t>(1, 73, "结束")),
                MakeStageCommand(*this, "跳过本回合（仅限特殊道具）", &RoundStage::Pass_, VoidChecker("pass")),
                MakeStageCommand(*this, "查看蜂巢初始状态，用于预览序号", &RoundStage::InitInfo_, AlterChecker<uint32_t>({{"地图", 0}, {"盘面", 0}})),
                MakeStageCommand(*this, "查看本回合开始时蜂巢情况，可用于图片重发", &RoundStage::Info_, VoidChecker("赛况")))
            , card_(card)
            , comb_html_(main_stage.CombHtml("## 第 " + std::to_string(round) + " 回合"))
    {}

    virtual void OnStageBegin() override
    {
        if (card_.Type() == "") {
            Global().Boardcast() << "本回合砖块为 " << card_.CardName() << "，请公屏或私信裁判设置数字：";
        } else {
            Global().Boardcast() << "本回合为特殊道具 [" << card_.CardName() << "]，请公屏或私信裁判使用道具，或「pass」跳过本回合：";
        }
        SendInfo(Global().BoardcastMsgSender());
        Global().StartTimer(GAME_OPTION(局时));
    }

  private:
    void HandleUnreadyPlayers_()
    {
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            if (!Global().IsReady(pid)) {
                auto& player = Main().players_[pid];
                auto sender = Global().Boardcast();
                if (card_.Type() == "") {
                    const auto [num, result] = player.comb_->SeqFill(card_);
                    sender << At(pid) << "因为超时未做选择，自动填入空位置 " << num;
                    if (result.score_ > 0) {
                        sender << "，意外收获 " << result.score_ << " 点积分";
                    }
                    if (GAME_OPTION(连线奖励) && result.extra_score_ > 0) {
                        sender << "和连线奖励分数 " << result.score_ << " 点积分";
                    }
                } else {
                    sender << At(pid) << "因为超时未做选择，自动跳过特殊道具";
                }
            }
        }
        Global().HookUnreadyPlayers();
    }

    virtual CheckoutErrCode OnStageTimeout() override
    {
        HandleUnreadyPlayers_();
        return StageErrCode::CHECKOUT;
    }

    virtual CheckoutErrCode OnStageOver() override
    {
        HandleUnreadyPlayers_();
        return StageErrCode::CHECKOUT;
    }

    virtual AtomReqErrCode OnComputerAct(const PlayerID pid, MsgSenderBase& reply) override
    {
        if (Global().IsReady(pid)) {
            return StageErrCode::OK;
        }
        auto& player = Main().players_[pid];
        if (card_.Type() == "") {
            player.comb_->SeqFill(card_);
        }
        return StageErrCode::READY;
    }

    AtomReqErrCode Set_(const PlayerID pid, const bool is_public, MsgSenderBase& reply, const uint32_t num)
    {
        if (Global().IsReady(pid)) {
            reply() << "您本回合已经行动完成，无法重复设置";
            return StageErrCode::FAILED;
        }
        if (card_.Type() == "move") {
            reply() << "[错误] 移动道具需要输入两个位置，包含起始和结束位置";
            return StageErrCode::FAILED;
        }

        auto& player = Main().players_[pid];
        if (player.comb_->IsWall(num) && !player.comb_->CanReplace(num) && card_.Type() != "erase" && card_.Type() != "reshape") {
            reply() << "[错误] 该位置为普通墙块，无法放置";
            return StageErrCode::FAILED;
        }
        if (player.comb_->IsFilled(num) && !player.comb_->CanReplace(num) && card_.Type() != "erase" && card_.Type() != "reshape") {
            reply() << "[错误] 该位置已经有砖块了，当前位置不可被覆盖";
            return StageErrCode::FAILED;
        }
        if (card_.Type() == "erase" && !player.comb_->IsWall(num) && !player.comb_->IsFilled(num)) {
            reply() << "[错误] 消除失败，该位置为空，试试其它位置吧";
            return StageErrCode::FAILED;
        }
        const auto [point, extra_point] = player.comb_->Fill(num, card_);
        auto sender = reply();
        if (card_.Type() == "erase") {
            sender << "消除位置 " << num << " 成功";
        } else if (card_.Type() == "reshape") {
            if (player.comb_->IsFilled(num)) {
                sender << "将位置 " << num << " 转化为癞子";
            } else {
                sender << "重塑位置 " << num << " 成功";
            }
        } else {
            sender << "设置数字 " << num << " 成功";
        }
        if (point > 0) sender << "，本次操作收获 " + std::to_string(point) + " 点积分";
        else if (point < 0) sender << "，本次操作损失 " + std::to_string(-point) + " 点积分";
        if (GAME_OPTION(连线奖励)) {
            if (extra_point > 0) sender << "，连线额外奖励 " + std::to_string(extra_point) + " 点积分";
            else if (extra_point < 0) sender << "，损失奖励得分 " + std::to_string(-extra_point) + " 点积分";
        }
        return StageErrCode::READY;
    }

    AtomReqErrCode Move_(const PlayerID pid, const bool is_public, MsgSenderBase& reply, const uint32_t from, const uint32_t to)
    {
        if (Global().IsReady(pid)) {
            reply() << "您本回合已经行动完成，无法重复设置";
            return StageErrCode::FAILED;
        }
        if (card_.Type() != "move") {
            reply() << "[错误] 本回合并非移动道具，不能执行移动操作";
            return StageErrCode::FAILED;
        }

        auto& player = Main().players_[pid];
        if (!player.comb_->IsWall(from) && !player.comb_->IsFilled(from)) {
            reply() << "[错误] 移动的起始位置为空，仅能移动墙块和砖块";
            return StageErrCode::FAILED;
        }
        if (!player.comb_->CanReplace(to)) {
            reply() << "[错误] 移动的结束位置存在墙块或砖块且不可覆盖，仅能移动至空区域";
            return StageErrCode::FAILED;
        }
        const auto [point, extra_point] = player.comb_->Move(from, to);
        auto sender = reply();
        sender << "成功将位置 " << from << " 的砖块移动至 " << to;
        if (point > 0) sender << "，本次操作收获 " + std::to_string(point) + " 点积分";
        else if (point < 0) sender << "，本次操作损失 " + std::to_string(-point) + " 点积分";
        if (GAME_OPTION(连线奖励)) {
            if (extra_point > 0) sender << "，连线额外奖励 " + std::to_string(extra_point) + " 点积分";
            else if (extra_point < 0) sender << "，损失奖励得分 " + std::to_string(-extra_point) + " 点积分";
        }
        return StageErrCode::READY;
    }

    AtomReqErrCode Pass_(const PlayerID pid, const bool is_public, MsgSenderBase& reply)
    {
        if (Global().IsReady(pid)) {
            reply() << "您本回合已经行动完成，无法重复设置";
            return StageErrCode::FAILED;
        }
        if (card_.Type() == "") {
            reply() << "[错误] 本回合为普通砖块，仅特殊道具可以pass";
            return StageErrCode::FAILED;
        }
        reply() << "您选择跳过本回合";
        return StageErrCode::READY;
    }

    AtomReqErrCode InitInfo_(const PlayerID pid, const bool is_public, MsgSenderBase& reply, const uint32_t null)
    {
        reply() << Markdown(Main().players_[pid].comb_->GetInitTable_());
        return StageErrCode::OK;
    }

    AtomReqErrCode Info_(const PlayerID pid, const bool is_public, MsgSenderBase& reply)
    {
        SendInfo(reply);
        return StageErrCode::OK;
    }

    void SendInfo(MsgSenderBase& sender)
    {
        sender() << Markdown{comb_html_};
        const std::string style = "<style>body{margin:0;}</style>" + GetStyle(Global().ResourceDir());
        if (card_.Type() == "") {
            sender() << Markdown(style + card_.ToHtml(Global().ResourceDir()), 64);
        } else {
            sender() << Image((Global().ResourceDir() / std::filesystem::path(card_.Type() + ".png")).string());
        }
    }

    const AreaCard card_;
    const std::string comb_html_;
};

void MainStage::NewStage_(SubStageFsmSetter& setter)
{
    const auto round = round_;
    const auto& card = *(it_++);
    setter.Emplace<RoundStage>(*this, ++round_, card);
    return;
}

void MainStage::FirstStageFsm(SubStageFsmSetter setter)
{
    NewStage_(setter);
}

void MainStage::NextStageFsm(RoundStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter)
{
    if (round_ < GAME_OPTION(回合数) && !CheckGameOver_(players_) && it_ != cards_.end()) {
        NewStage_(setter);
        return;
    }
    if (round_ >= GAME_OPTION(回合数)) Global().Boardcast() << "回合数到达上限，游戏结束";
    if (it_ == cards_.end()) Global().Boardcast() << "卡池耗尽，游戏结束";

    Global().Boardcast() << Markdown(CombHtml("## 终局"));
    if (GAME_OPTION(种子).empty()) {
        const std::string game_mode[4] = {"经典", "癞子", "空气", "混乱"};
        Global().Boardcast() << "【本局卡池】" + game_mode[GAME_OPTION(卡池)] + "\n随机数种子：" + seed_str;
        // achievements
        auto max_player = std::max_element(players_.begin(), players_.end(), [](const Player& a, const Player& b) { return a.comb_->Score() < b.comb_->Score(); });
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            if (GAME_OPTION(卡池) == 0) {
                if (players_[pid].comb_->Score() >= 250) Global().Achieve(pid, Achievement::经典开篇);
                if (players_[pid].comb_->Score() >= 300) Global().Achieve(pid, Achievement::不凡经典);
                if (players_[pid].comb_->Score() >= 350) Global().Achieve(pid, Achievement::传世经典);
            } else if (GAME_OPTION(卡池) == 1) {
                if (players_[pid].comb_->Score() >= 350) Global().Achieve(pid, Achievement::万象归一);
            } else if (GAME_OPTION(卡池) == 2) {
                if (players_[pid].comb_->Score() >= 250) Global().Achieve(pid, Achievement::空灵之境);
            } else if (GAME_OPTION(卡池) == 3) {
                if (players_[pid].comb_->Score() >= 250) Global().Achieve(pid, Achievement::乱象初现);
                if (players_[pid].comb_->Score() >= 300) Global().Achieve(pid, Achievement::破碎旅程);
                if (players_[pid].comb_->Score() >= 350) Global().Achieve(pid, Achievement::混乱主宰);
            }
            if ((GAME_OPTION(卡池) == 2 || GAME_OPTION(卡池) == 3) && players_[pid].comb_->AirAllLine()) {
                Global().Achieve(pid, Achievement::空中楼阁);
            }
            if ((GAME_OPTION(卡池) == 1 || GAME_OPTION(卡池) == 3) && players_[pid].comb_->WildAll10() && players_[pid].comb_->Score() == max_player->comb_->Score()) {
                Global().Achieve(pid, Achievement::十全十美);
            }
            if (players_[pid].comb_->BreakLine() == 2) Global().Achieve(pid, Achievement::蜂巢开拓者);
            if (players_[pid].comb_->BreakLine() == 1) Global().Achieve(pid, Achievement::蜂巢编织者);
            if (players_[pid].comb_->BreakLine() == 0) Global().Achieve(pid, Achievement::蜂巢完美者);
            if (players_[pid].comb_->Length9()) Global().Achieve(pid, Achievement::贯穿星河);
        }
    }
}

auto* MakeMainStage(MainStageFactory factory) { return factory.Create<MainStage>(); }

} // namespace GAME_MODULE_NAME

} // namespace game

} // gamespace lgtbot

