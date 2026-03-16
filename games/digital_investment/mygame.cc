// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).

#include <algorithm>
#include <numeric>
#include <vector>
#include <ctime>
#include <cstdlib>

#include "game_framework/stage.h"
#include "game_framework/util.h"
#include "utility/html.h"

using namespace std;

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

class MainStage;
template <typename... SubStages> using SubGameStage = StageFsm<MainStage, SubStages...>;
template <typename... SubStages> using MainGameStage = StageFsm<void, SubStages...>;

const GameProperties k_properties {
    .name_ = "数字投资",
    .developer_ = "铁蛋",
    .description_ = "按顺序提交唯一数字，争取连续得分的游戏",
};

uint64_t MaxPlayerNum(const CustomOptions& options) { return 10; }
uint32_t Multiple(const CustomOptions& options) { return 1; }
const MutableGenericOptions k_default_generic_options;
const std::vector<RuleCommand> k_rule_commands = {};

bool AdaptOptions(MsgSenderBase& reply, CustomOptions& game_options, const GenericOptions& generic_options_readonly,
        MutableGenericOptions& generic_options)
{
    if (generic_options_readonly.PlayerNum() < 2 || generic_options_readonly.PlayerNum() > 10) {
        reply() << "该游戏支持 2-10 人参加，当前玩家数为 " << generic_options_readonly.PlayerNum();
        return false;
    }
    return true;
}

const std::vector<InitOptionsCommand> k_init_options_commands = {
    InitOptionsCommand("独自一人开始游戏",
            [] (CustomOptions& game_options, MutableGenericOptions& generic_options)
            {
                generic_options.bench_computers_to_player_num_ = 5;
                return NewGameMode::SINGLE_USER;
            },
            VoidChecker("单机")),
};

class RoundStage;

class MainStage : public MainGameStage<RoundStage>
{
  public:
    MainStage(StageUtility&& utility)
        : StageFsm(std::move(utility), MakeStageCommand(*this, "查看当前游戏进展情况", &MainStage::Status_, VoidChecker("赛况"))),
        round_(0),
        group_idx_(0),
        round_limit_(0),
        player_total_scores_(Global().PlayerNum(), 0),
        player_last_total_scores_(Global().PlayerNum(), 0),
        player_last_scored_(Global().PlayerNum(), false),
        player_current_select_(Global().PlayerNum(), 0),
        player_last_select_(Global().PlayerNum(), 0)
    {
        round_limit_ = static_cast<int>(GAME_OPTION(N));
        player_left_nums_.resize(Global().PlayerNum());
        for (auto& left_nums : player_left_nums_) {
            for (int num = 1; num <= round_limit_; ++num) {
                left_nums.push_back(num);
            }
        }
    }

    virtual void FirstStageFsm(SubStageFsmSetter setter) override;
    virtual void NextStageFsm(RoundStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter) override;
    virtual int64_t PlayerScore(const PlayerID pid) const override { return player_total_scores_[pid]; }

    string PendingGroupDesc() const;
    string LeftNumsDesc(const PlayerID pid) const;
    bool IsCurrentGroupPlayer(const PlayerID pid) const;
    bool IsLastGroup() const { return group_idx_ + 1 >= round_groups_.size(); }
    void BuildGroupsForCurrentRound();
    void ResolveCurrentRound();

    int round_;
    size_t group_idx_;
    int round_limit_;

    vector<int64_t> player_total_scores_;
    vector<int64_t> player_last_total_scores_;
    vector<bool> player_last_scored_;
    vector<int64_t> player_current_select_;
    vector<int64_t> player_last_select_;
    vector<vector<int64_t>> player_left_nums_;

    vector<vector<PlayerID>> round_groups_;
    string table_title_;
    string table_body_;

    const string sum_color = "DCDCDC";
    const string bonus_color = "E8FACC";

  private:
    CompReqErrCode Status_(const PlayerID pid, const bool is_public, MsgSenderBase& reply)
    {
        string left_num_info = "你当前剩余数字：" + LeftNumsDesc(pid);
        reply() << "当前第 " << round_ << " / " << round_limit_ << " 轮\n" << PendingGroupDesc();
        reply() << left_num_info;
        if (!table_body_.empty()) {
            reply() << Markdown(table_title_ + table_body_ + "</table>");
        }
        return StageErrCode::OK;
    }
};

class RoundStage : public SubGameStage<>
{
  public:
    RoundStage(MainStage& main_stage, const uint64_t round)
        : StageFsm(main_stage,
                "第 " + std::to_string(round) + " / " + std::to_string(main_stage.round_limit_) + " 轮",
                MakeStageCommand(*this, "提交数字", &RoundStage::Submit_, ArithChecker<int64_t>(1, main_stage.round_limit_, "数字")))
    {
    }

    virtual void OnStageBegin() override
    {
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            if (!Main().IsCurrentGroupPlayer(pid)) {
                Global().SetReady(pid);
            }
        }

        Global().Boardcast() << Main().PendingGroupDesc();
        Global().Boardcast() << "请当前分组玩家私信裁判提交数字（1-N），每个数字只能使用一次。";
        Global().StartTimer(GAME_OPTION(时限));
    }

    virtual CheckoutErrCode OnStageTimeout() override
    {
        Global().HookUnreadyPlayers();
        for (const auto pid : Main().round_groups_[Main().group_idx_]) {
            if (!Global().IsReady(pid)) {
                const auto num = Main().player_left_nums_[pid].front();
                CommitSelect_(pid, num);
                Global().SetReady(pid);
                Global().Boardcast() << "玩家 " << Global().PlayerName(pid) << " 超时，自动提交最小剩余数字 " << num;
            }
        }

        if (Main().IsLastGroup()) {
            Main().ResolveCurrentRound();
        }
        return StageErrCode::CHECKOUT;
    }

    virtual CheckoutErrCode OnPlayerLeave(const PlayerID pid) override
    {
        if (Main().IsCurrentGroupPlayer(pid) && !Global().IsReady(pid)) {
            const auto num = Main().player_left_nums_[pid].front();
            CommitSelect_(pid, num);
            Global().SetReady(pid);
        }
        return StageErrCode::CONTINUE;
    }

    virtual AtomReqErrCode OnComputerAct(const PlayerID pid, MsgSenderBase& reply) override
    {
        if (!Main().IsCurrentGroupPlayer(pid) || Global().IsReady(pid)) {
            return StageErrCode::OK;
        }
        const int idx = rand() % Main().player_left_nums_[pid].size();
        CommitSelect_(pid, Main().player_left_nums_[pid][idx]);
        return StageErrCode::READY;
    }

    virtual CheckoutErrCode OnStageOver() override
    {
        for (const auto pid : Main().round_groups_[Main().group_idx_]) {
            if (!Global().IsReady(pid)) {
                const auto num = Main().player_left_nums_[pid].front();
                CommitSelect_(pid, num);
            }
        }
        if (Main().IsLastGroup()) {
            Main().ResolveCurrentRound();
        }
        return StageErrCode::CHECKOUT;
    }

  private:
    void CommitSelect_(const PlayerID pid, const int64_t num)
    {
        Main().player_current_select_[pid] = num;
        auto& left_nums = Main().player_left_nums_[pid];
        left_nums.erase(remove(left_nums.begin(), left_nums.end(), num), left_nums.end());
    }

    AtomReqErrCode Submit_(const PlayerID pid, const bool is_public, MsgSenderBase& reply, const int64_t num)
    {
        if (is_public) {
            reply() << "[错误] 请私信裁判提交数字";
            return StageErrCode::FAILED;
        }
        if (!Main().IsCurrentGroupPlayer(pid)) {
            reply() << "[错误] 当前不是你的提交分组。\n" << Main().PendingGroupDesc();
            return StageErrCode::FAILED;
        }
        if (Global().IsReady(pid)) {
            reply() << "[错误] 你本组已经提交过数字";
            return StageErrCode::FAILED;
        }

        // 规则关键点：每位玩家 1-N 每个数字只能使用一次。
        if (count(Main().player_left_nums_[pid].begin(), Main().player_left_nums_[pid].end(), num) == 0) {
            reply() << "[错误] 该数字已使用，剩余可用数字：" << Main().LeftNumsDesc(pid);
            return StageErrCode::FAILED;
        }

        CommitSelect_(pid, num);
        reply() << "提交成功，剩余可用数字：" << Main().LeftNumsDesc(pid);
        return StageErrCode::READY;
    }
};

string MainStage::LeftNumsDesc(const PlayerID pid) const
{
    string s;
    for (size_t i = 0; i < player_left_nums_[pid].size(); ++i) {
        if (i) {
            s += " ";
        }
        s += to_string(player_left_nums_[pid][i]);
    }
    return s.empty() ? "无" : s;
}

bool MainStage::IsCurrentGroupPlayer(const PlayerID pid) const
{
    const auto& group = round_groups_[group_idx_];
    return count(group.begin(), group.end(), pid) > 0;
}

string MainStage::PendingGroupDesc() const
{
    string msg;
    msg += "当前提交分组：" + to_string(group_idx_ + 1) + " / " + to_string(round_groups_.size()) + "\n";
    msg += "本组玩家：";
    const auto& group = round_groups_[group_idx_];
    for (size_t i = 0; i < group.size(); ++i) {
        msg += Global().PlayerName(group[i]);
        if (i + 1 < group.size()) {
            msg += "、";
        }
    }
    if (group_idx_ + 1 < round_groups_.size()) {
        msg += "\n下一组将在当前组全部提交后开放。";
    }
    return msg;
}

void MainStage::BuildGroupsForCurrentRound()
{
    round_groups_.clear();

    // 规则关键点：第 1 轮全员同时提交。
    if (round_ == 1) {
        vector<PlayerID> all_players;
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            all_players.push_back(pid);
        }
        round_groups_.push_back(std::move(all_players));
        return;
    }

    // 规则关键点：第 2 轮起按“上一轮提交数字”升序分组；同数字玩家同组可并行提交。
    for (int num = 1; num <= round_limit_; ++num) {
        vector<PlayerID> group;
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            if (player_last_select_[pid] == num) {
                group.push_back(pid);
            }
        }
        if (!group.empty()) {
            round_groups_.push_back(std::move(group));
        }
    }
}

void MainStage::ResolveCurrentRound()
{
    const auto round_sum = accumulate(player_current_select_.begin(), player_current_select_.end(), 0LL);

    string board;
    board += "<tr bgcolor=\"#FFE4C4\"><th>回合</th><th>玩家</th><th>提交</th><th>总和</th><th>是否得分</th><th>基础分</th><th>连击+2</th><th>本轮</th><th>总分</th></tr>";

    // 规则关键点：连续得分 +2 仅在“上一轮得分且本轮得分”时触发。
    for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
        const int64_t select = player_current_select_[pid];
        const bool scored = (round_sum % select == 0);
        const int64_t base_score = scored ? select : 0;
        const bool combo = round_ > 1 && scored && player_last_scored_[pid];
        const int64_t bonus = combo ? 2 : 0;
        const int64_t round_score = base_score + bonus;

        player_last_total_scores_[pid] = player_total_scores_[pid];
        player_total_scores_[pid] += round_score;
        player_last_scored_[pid] = scored;
        player_last_select_[pid] = select;

        board += "<tr>";
        board += "<td>" + to_string(round_) + "</td>";
        board += "<td>" + Global().PlayerName(pid) + "</td>";
        board += "<td>" + to_string(select) + "</td>";
        board += "<td bgcolor=\"" + sum_color + "\">" + to_string(round_sum) + "</td>";
        board += "<td>" + string(scored ? "是" : "否") + "</td>";
        board += "<td>" + to_string(base_score) + "</td>";
        board += "<td" + string(combo ? " bgcolor=\"" + bonus_color + "\"" : "") + ">" + (combo ? "是" : "否") + "</td>";
        board += "<td>" + to_string(round_score) + "</td>";
        board += "<td>" + to_string(player_total_scores_[pid]) + "</td>";
        board += "</tr>";
    }

    table_title_ = "<table style=\"text-align:center\">";
    table_body_ += board;

    vector<PlayerID> rank(Global().PlayerNum());
    iota(rank.begin(), rank.end(), 0);
    sort(rank.begin(), rank.end(), [this](const PlayerID a, const PlayerID b) {
        if (player_total_scores_[a] != player_total_scores_[b]) {
            return player_total_scores_[a] > player_total_scores_[b];
        }
        return a < b;
    });

    string rank_msg = "当前排名：\n";
    for (size_t i = 0; i < rank.size(); ++i) {
        rank_msg += to_string(i + 1) + ". " + Global().PlayerName(rank[i]) + " - " + to_string(player_total_scores_[rank[i]]) + "分";
        if (i + 1 < rank.size()) {
            rank_msg += "\n";
        }
    }

    Global().Boardcast() << "第 " << round_ << " 轮结算完成。";
    Global().Boardcast() << Markdown(table_title_ + board + "</table>");
    Global().Boardcast() << rank_msg;
}

void MainStage::FirstStageFsm(SubStageFsmSetter setter)
{
    srand((unsigned int)time(NULL));
    Global().Boardcast() << "游戏开始：数字投资。共 " << round_limit_ << " 轮，数字 1-" << round_limit_ << " 每个仅可使用一次。";

    round_ = 1;
    group_idx_ = 0;
    BuildGroupsForCurrentRound();
    setter.Emplace<RoundStage>(*this, round_);
}

void MainStage::NextStageFsm(RoundStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter)
{
    if (group_idx_ + 1 < round_groups_.size()) {
        ++group_idx_;
        setter.Emplace<RoundStage>(*this, round_);
        return;
    }

    if (round_ >= round_limit_) {
        vector<PlayerID> rank(Global().PlayerNum());
        iota(rank.begin(), rank.end(), 0);
        sort(rank.begin(), rank.end(), [this](const PlayerID a, const PlayerID b) {
            if (player_total_scores_[a] != player_total_scores_[b]) {
                return player_total_scores_[a] > player_total_scores_[b];
            }
            return a < b;
        });

        string final_rank = "最终排名：\n";
        for (size_t i = 0; i < rank.size(); ++i) {
            final_rank += to_string(i + 1) + ". " + Global().PlayerName(rank[i]) + " - " + to_string(player_total_scores_[rank[i]]) + "分";
            if (i + 1 < rank.size()) {
                final_rank += "\n";
            }
        }
        Global().Boardcast() << round_limit_ << " 轮结束，游戏结束。";
        Global().Boardcast() << final_rank;
        return;
    }

    ++round_;
    fill(player_current_select_.begin(), player_current_select_.end(), 0);
    group_idx_ = 0;
    BuildGroupsForCurrentRound();
    setter.Emplace<RoundStage>(*this, round_);
}

auto* MakeMainStage(MainStageFactory factory) { return factory.Create<MainStage>(); }

} // namespace GAME_MODULE_NAME

} // namespace game

} // namespace lgtbot
