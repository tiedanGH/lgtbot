// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).
//
// This file was generated with the assistance of Claude Code (claude.ai/code).

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "game_framework/stage.h"
#include "game_framework/util.h"

using namespace std;

#include "constants.h"
#include "player.h"
#include "board.h"

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

class MainStage;
template <typename... SubStages> using SubGameStage = StageFsm<MainStage, SubStages...>;
template <typename... SubStages> using MainGameStage = StageFsm<void, SubStages...>;

const GameProperties k_properties {
    .name_ = "钓鱼",
    .developer_ = "铁蛋",
    .description_ = "鱼为不饿死而来，渔夫为不空军而来，两池之间的多人博弈游戏",
    .shuffled_player_id_ = true,
};

uint64_t MaxPlayerNum(const CustomOptions& options) { return MAX_PLAYER; }
// TODO: 正式版本上线前需启用实际倍率
uint32_t Multiple(const CustomOptions& options) { return 0; }

const MutableGenericOptions k_default_generic_options{
    .is_formal_ = false,
};

const std::vector<RuleCommand> k_rule_commands = {};

bool AdaptOptions(ChildMsgSenderBase& reply, CustomOptions& game_options, const GenericOptions& generic_options_readonly, MutableGenericOptions& generic_options)
{
    if (generic_options_readonly.PlayerNum() < MIN_PLAYER) {
        reply() << "该游戏至少 " << MIN_PLAYER << " 人参加，当前玩家数为 " << generic_options_readonly.PlayerNum();
        return false;
    }
    return true;
}

const std::vector<InitOptionsCommand> k_init_options_commands = {
    InitOptionsCommand("开启任命模式，由随机一名玩家任命全部渔夫",
            [] (CustomOptions& game_options, MutableGenericOptions& generic_options)
            {
                GET_OPTION_VALUE(game_options, 任命模式) = true;
                return NewGameMode::MULTIPLE_USERS;
            },
            VoidChecker("任命")),
    InitOptionsCommand("独自一人开始游戏",
            [] (CustomOptions& game_options, MutableGenericOptions& generic_options)
            {
                generic_options.bench_computers_to_player_num_ = 7;
                return NewGameMode::SINGLE_USER;
            },
            VoidChecker("单机")),
};

/* ========== 指令取值表 ========== */

// 体力：0 ~ 5 之间 0.5 的倍数
static const std::map<std::string, int> k_stamina_map = {
    {"0", 0},   {"0.5", 1}, {"1", 2},   {"1.5", 3}, {"2", 4}, {"2.5", 5},
    {"3", 6},   {"3.5", 7}, {"4", 8},   {"4.5", 9}, {"5", 10},
};

static const std::map<std::string, Bait> k_bait_map = {
    {"初级", Bait::PRIMARY},  {"初级鱼饵", Bait::PRIMARY},  {"1", Bait::PRIMARY},
    {"中等", Bait::MEDIUM},   {"中等鱼饵", Bait::MEDIUM},   {"2", Bait::MEDIUM},
    {"高等", Bait::ADVANCED}, {"高等鱼饵", Bait::ADVANCED}, {"3", Bait::ADVANCED},
    {"沙虫", Bait::SANDWORM}, {"4", Bait::SANDWORM},
};

static const std::map<std::string, int> k_pool_map = {
    {"a", POOL_A}, {"A", POOL_A}, {"b", POOL_B}, {"B", POOL_B},
};

// ========== GAME STAGES ==========

class AppointStage;
class MoveStage;
class BaitStage;
class HookStage;
class StaminaStage;

class MainStage : public MainGameStage<AppointStage, MoveStage, BaitStage, HookStage, StaminaStage>
{
  public:
    MainStage(StageUtility&& utility)
        : StageFsm(std::move(utility),
            MakeStageCommand(*this, "查看当前两池的局势与全部玩家数据", &MainStage::Status_, VoidChecker("赛况")),
            MakeStageCommand(*this, "私信查看自己本回合可以执行的行动", &MainStage::MyStatus_, VoidChecker("状态")))
        , round_(0)
        , player_scores_(Global().PlayerNum(), 0)
        , hints_(Global().PlayerNum())
    {}

    virtual void FirstStageFsm(SubStageFsmSetter setter) override;
    virtual void NextStageFsm(AppointStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter) override;
    virtual void NextStageFsm(MoveStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter) override;
    virtual void NextStageFsm(BaitStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter) override;
    virtual void NextStageFsm(HookStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter) override;
    virtual void NextStageFsm(StaminaStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter) override;

    virtual void OnPlayerLeave(const PlayerID pid) override;

    virtual int64_t PlayerScore(const PlayerID pid) const override { return player_scores_[pid]; }

    // 回合数，任命阶段为 0
    int round_;
    // 两口水池与全部玩家
    Board board;
    // 玩家最终得分
    std::vector<int64_t> player_scores_;
    // 各玩家在当前阶段可执行的行动说明
    std::vector<std::string> hints_;
    // 本回合的公屏战报
    std::string report_;
    // 本回合离场、待向框架发出淘汰通知的玩家
    std::vector<int> pending_eliminate_;
    // 获得任命权的玩家
    int appointer_ = 0;
    // 身份是否已经确定
    bool roles_assigned_ = false;
    // 因人数不足而中止
    bool aborted_ = false;

    int PlayerNum() const { return static_cast<int>(Global().PlayerNum()); }
    Player& P(const int pid) { return board.players[pid]; }
    const Player& P(const int pid) const { return board.players[pid]; }

    // 尚未退出的玩家数
    int AliveCount() const
    {
        int count = 0;
        for (const Player& p : board.players) {
            if (p.Alive()) {
                count++;
            }
        }
        return count;
    }
    // 当前人数下本局需要的渔夫数量
    int RequiredFishermen() const { return FishermanNum(AliveCount()); }

    /* ===== 身份 ===== */
    void AssignRoles(const std::vector<int>& fishermen);
    void AssignRolesRandomly();
    void AnnounceRoles();

    /* ===== 结算 ===== */
    void KillPlayer(const int pid, const OutReason reason);
    void SettleMove();
    void SettleHunt();
    void SettleFishing();
    void SettleConsume();
    bool AutoBiteIfNoChoice(const int pid);
    void AnnounceBites();
    void AfterHooksDecided(SubStageFsmSetter& setter);
    void FinishRound(SubStageFsmSetter& setter);
    bool CheckGameEnd(const bool check_round_limit);
    void FinishGame();

    /* ===== 界面 ===== */
    void SetPhase(const std::string& phase) { cur_phase_ = phase; }
    void BoardcastBoard(const std::string& phase, const bool reveal)
    {
        SetPhase(phase);
        Global().Boardcast() << Markdown(board.BoardHtml(phase, DisplayRound(), reveal), Board::ImageWidth());
    }
    int DisplayRound() const { return round_ < 1 ? 1 : round_; }

  private:
    CompReqErrCode Status_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply)
    {
        if (!roles_assigned_) {
            reply() << Markdown(board.RosterHtml(appointer_, RequiredFishermen()), Board::ImageWidth());
        } else {
            reply() << Markdown(board.BoardHtml(cur_phase_, DisplayRound(), game_over_), Board::ImageWidth());
        }
        return StageErrCode::OK;
    }

    CompReqErrCode MyStatus_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply)
    {
        if (is_public) {
            reply() << "[错误] 请私信裁判查看自己的行动选项";
            return StageErrCode::FAILED;
        }
        const Player& p = P(pid);
        auto sender = reply();
        sender << "【" << PlayerNo(pid) << " 号 · " << p.RoleName() << "】所在水池：" << pool_cn[p.pool] << "\n";
        if (p.IsFish()) {
            sender << "当前肥度：" << NumStr(p.fat) << (p.Starving() ? "（极度饥饿）" : "") << "\n";
        } else {
            sender << "剩余鱼饵：";
            for (int i = 0; i < BAIT_KIND_NUM; i++) {
                sender << bait_cn[i] << " " << p.baits[i] << "　";
            }
            sender << "\n已挖鱼饵 " << p.dig_used << " / " << DIG_LIMIT << " 次，累计 " << NumStr(p.weight) << " 公斤\n";
        }
        sender << "\n" << (hints_[pid].empty() ? "本阶段您无需行动" : hints_[pid]);
        return StageErrCode::OK;
    }

    std::string cur_phase_ = "开局";

  public:
    // 游戏是否已经结束，终局赛况会公开全部隐藏信息
    bool game_over_ = false;
};


// ========== 任命阶段 ==========

class AppointStage : public SubGameStage<>
{
  public:
    AppointStage(MainStage& main_stage)
        : StageFsm(main_stage, "任命渔夫",
            MakeStageCommand(*this, "一次性任命本局全部渔夫", &AppointStage::Appoint_,
                RepeatableChecker<ArithChecker<int64_t>>(1, MAX_PLAYER, "玩家编号")),
            MakeStageCommand(*this, "放弃任命，随机指派身份", &AppointStage::Pass_, VoidChecker("pass")))
    {}

    virtual void OnStageBegin() override
    {
        const int need = Main().RequiredFishermen();
        Global().Boardcast() << Markdown(Main().board.RosterHtml(Main().appointer_, need), Board::ImageWidth());
        Global().Boardcast() << "两口池塘并列排开，水面之下将有人化鱼，岸上将有人执竿。\n"
                             << At(PlayerID(Main().appointer_)) << " 获得了本局任命权！\n\n"
                             << "请发送「<编号> <编号> ...」，一次性指定 " << need  << " 名渔夫（可以任命自己），或发送「pass」随机指派。\n"
                             << "时限 " << GAME_OPTION(时限) << " 秒，超时则随机指派";
        for (int i = 0; i < Main().PlayerNum(); i++) {
            if (i != Main().appointer_) {
                Global().SetReady(i);
                Main().hints_[i] = "本阶段由 " + PlayerNo(Main().appointer_) + " 号任命渔夫，请稍候";
            }
        }
        Main().hints_[Main().appointer_] = "「<编号> <编号> ...」一次性指定 " + std::to_string(need) + " 名渔夫，或「pass」随机指派";
        Global().StartTimer(GAME_OPTION(时限));
    }

    virtual CheckoutErrCode OnStageOver() override { return StageErrCode::CHECKOUT; }

    virtual CheckoutErrCode OnStageTimeout() override
    {
        Global().Boardcast() << At(PlayerID(Main().appointer_)) << " 迟迟没有指定，已随机指派";
        return StageErrCode::CHECKOUT;
    }

    virtual CheckoutErrCode OnPlayerLeave(const PlayerID pid) override { return StageErrCode::CONTINUE; }

    virtual AtomReqErrCode OnComputerAct(const PlayerID pid, ChildMsgSenderBase& reply) override
    {
        if (Global().IsReady(pid)) {
            return StageErrCode::OK;
        }
        std::vector<int> candidates;
        for (const Player& p : Main().board.players) {
            if (p.Alive()) {
                candidates.push_back(p.pid);
            }
        }
        std::shuffle(candidates.begin(), candidates.end(), Main().board.g);
        const int need = std::min<int>(Main().RequiredFishermen(), static_cast<int>(candidates.size()));
        Main().AssignRoles(std::vector<int>(candidates.begin(), candidates.begin() + need));
        return StageErrCode::READY;
    }

  private:
    AtomReqErrCode Appoint_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply,
            const std::vector<int64_t>& nos)
    {
        if (pid != Main().appointer_) {
            reply() << "[错误] 本次任命权不在您手中";
            return StageErrCode::FAILED;
        }
        if (Global().IsReady(pid)) {
            reply() << "[错误] 您已经完成了任命";
            return StageErrCode::FAILED;
        }
        const int need = Main().RequiredFishermen();
        if (static_cast<int>(nos.size()) != need) {
            reply() << "[错误] 任命失败：本局需要 " << need << " 名渔夫，您指定了 " << nos.size() << " 名";
            return StageErrCode::FAILED;
        }
        std::vector<int> fishermen;
        for (const int64_t no : nos) {
            if (no < 1 || no > Main().PlayerNum()) {
                reply() << "[错误] 任命失败：场上没有 " << no << " 号玩家";
                return StageErrCode::FAILED;
            }
            const int target = static_cast<int>(no) - 1;
            if (!Main().P(target).Alive()) {
                reply() << "[错误] 任命失败：" << no << " 号玩家已经离开了游戏";
                return StageErrCode::FAILED;
            }
            if (std::find(fishermen.begin(), fishermen.end(), target) != fishermen.end()) {
                reply() << "[错误] 任命失败：" << no << " 号玩家被重复指定";
                return StageErrCode::FAILED;
            }
            fishermen.push_back(target);
        }
        Main().AssignRoles(fishermen);
        reply() << "任命完成，身份即将公之于众";
        return StageErrCode::READY;
    }

    AtomReqErrCode Pass_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply)
    {
        if (pid != Main().appointer_) {
            reply() << "[错误] 本次任命权不在您手中";
            return StageErrCode::FAILED;
        }
        reply() << "您放弃了任命，身份将随机指派";
        return StageErrCode::READY;
    }
};


// ========== 移动阶段 ==========

class MoveStage : public SubGameStage<>
{
  public:
    MoveStage(MainStage& main_stage, const int round)
        : StageFsm(main_stage, "第 " + std::to_string(round) + " 回合 · 洄游",
            MakeStageCommand(*this, "前往指定的水池", &MoveStage::Goto_, VoidChecker("前往"), AlterChecker<int>(k_pool_map)),
            MakeStageCommand(*this, "游向另一口水池", &MoveStage::Move_, VoidChecker("移动")),
            MakeStageCommand(*this, "停留在当前水池", &MoveStage::Stay_, VoidChecker("停留", "不动")))
    {}

    virtual void OnStageBegin() override
    {
        Main().BoardcastBoard("洄游", false);
        Global().Boardcast() << "请所有玩家私信裁判选择去向：「前往 A」「前往 B」「停留」\n"
                             << "鱼每次更换水池消耗 " << NumStr(MOVE_COST) << " 点肥度，渔夫的移动不需要任何代价。\n"
                             << "时限 " << GAME_OPTION(时限) << " 秒，超时默认停留原地";
        for (int i = 0; i < Main().PlayerNum(); i++) {
            Player& p = Main().P(i);
            if (!p.Alive()) {
                Global().SetReady(i);
                Main().hints_[i] = "";
                continue;
            }
            Main().hints_[i] = std::string("「前往 A」「前往 B」「停留」选择本回合的去向，当前位于 ") + std::string(pool_cn[p.pool]) + " 池" +
                               (p.IsFish() ? "，更换水池将消耗 " + NumStr(MOVE_COST) + " 点肥度" : "");
            Global().Tell(i) << Main().hints_[i];
        }
        Global().StartTimer(GAME_OPTION(时限));
    }

    virtual CheckoutErrCode OnStageOver() override
    {
        FillUnready_(false);
        return StageErrCode::CHECKOUT;
    }

    virtual CheckoutErrCode OnStageTimeout() override
    {
        FillUnready_(true);
        return StageErrCode::CHECKOUT;
    }

    virtual CheckoutErrCode OnPlayerLeave(const PlayerID pid) override { return StageErrCode::CONTINUE; }

    virtual AtomReqErrCode OnComputerAct(const PlayerID pid, ChildMsgSenderBase& reply) override
    {
        if (Global().IsReady(pid)) {
            return StageErrCode::OK;
        }
        Player& p = Main().P(pid);
        p.move_to = Main().board.RandomInt(3) == 0 ? OtherPool(p.pool) : p.pool;
        return StageErrCode::READY;
    }

  private:
    void FillUnready_(const bool hook)
    {
        for (int i = 0; i < Main().PlayerNum(); i++) {
            if (Global().IsReady(i) || !Main().P(i).Alive()) {
                continue;
            }
            Main().P(i).move_to = Main().P(i).pool;
            Global().Tell(i) << "您未在时限内做出选择，默认停留原地";
            if (hook) {
                Global().Hook(i);
            }
        }
    }

    AtomReqErrCode Select_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply, const int pool)
    {
        if (is_public) {
            reply() << "[错误] 请私信裁判选择去向";
            return StageErrCode::FAILED;
        }
        if (Global().IsReady(pid)) {
            reply() << "[错误] 您本回合已经选定了去向";
            return StageErrCode::FAILED;
        }
        Player& p = Main().P(pid);
        p.move_to = pool;
        if (pool == p.pool) {
            reply() << "您决定停留在 " << pool_cn[pool] << " 池";
        } else {
            reply() << "您决定游向 " << pool_cn[pool] << " 池"
                    << (p.IsFish() ? "，这将消耗 " + NumStr(MOVE_COST) + " 点肥度" : "");
        }
        return StageErrCode::READY;
    }

    AtomReqErrCode Goto_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply, const int pool)
    {
        return Select_(pid, is_public, reply, pool);
    }

    AtomReqErrCode Move_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply)
    {
        return Select_(pid, is_public, reply, OtherPool(Main().P(pid).pool));
    }

    AtomReqErrCode Stay_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply)
    {
        return Select_(pid, is_public, reply, Main().P(pid).pool);
    }
};


// ========== 渔夫下饵阶段 ==========

class BaitStage : public SubGameStage<>
{
  public:
    BaitStage(MainStage& main_stage, const int round)
        : StageFsm(main_stage, "第 " + std::to_string(round) + " 回合 · 下饵",
            MakeStageCommand(*this, "[渔夫] 系上指定的鱼饵下钩", &BaitStage::Fish_, VoidChecker("钓鱼", "下钩"),
                AlterChecker<Bait>(k_bait_map)),
            MakeStageCommand(*this, "[渔夫] 在 B 池挖取鱼饵，本回合无法钓鱼", &BaitStage::Dig_,
                VoidChecker("挖饵", "挖鱼饵")),
            MakeStageCommand(*this, "[渔夫] 放弃钓鱼，仅在本池无鱼或自己无饵时可用", &BaitStage::GiveUp_,
                VoidChecker("放弃", "放弃钓鱼")))
    {}

    virtual void OnStageBegin() override
    {
        Main().SetPhase("下饵");
        Global().Boardcast() << "请各位渔夫私信裁判选择本回合的行动，鱼群将在行动公示后再作抉择。时限 " << GAME_OPTION(时限) << " 秒";
        for (int i = 0; i < Main().PlayerNum(); i++) {
            Player& p = Main().P(i);
            if (!p.Alive() || p.IsFish()) {
                Global().SetReady(i);
                Main().hints_[i] = p.IsFish() && p.Alive() ? "本阶段由渔夫行动，请等待下钩情况公示" : "";
                continue;
            }
            Main().hints_[i] = Options_(p);
            Global().Tell(i) << Main().hints_[i];
        }
        Global().StartTimer(GAME_OPTION(时限));
    }

    virtual CheckoutErrCode OnStageOver() override
    {
        FillUnready_(false);
        return StageErrCode::CHECKOUT;
    }

    virtual CheckoutErrCode OnStageTimeout() override
    {
        FillUnready_(true);
        return StageErrCode::CHECKOUT;
    }

    virtual CheckoutErrCode OnPlayerLeave(const PlayerID pid) override { return StageErrCode::CONTINUE; }

    virtual AtomReqErrCode OnComputerAct(const PlayerID pid, ChildMsgSenderBase& reply) override
    {
        if (Global().IsReady(pid)) {
            return StageErrCode::OK;
        }
        Player& p = Main().P(pid);
        std::vector<int> avail;
        for (int i = 0; i < BAIT_KIND_NUM; i++) {
            if (p.baits[i] > 0) {
                avail.push_back(i);
            }
        }
        const bool has_fish = !Main().board.AliveFishInPool(p.pool).empty();
        if (p.CanDig() && (avail.empty() || !has_fish || Main().board.RandomInt(4) == 0)) {
            p.fisher_act = FisherAct::DIG;
        } else if (!avail.empty() && has_fish) {
            p.fisher_act = FisherAct::FISHING;
            p.bait = static_cast<Bait>(Main().board.RandomPick(avail));
        } else {
            p.fisher_act = FisherAct::GIVE_UP;
        }
        return StageErrCode::READY;
    }

  private:
    // 渔夫本回合可以执行的行动说明
    std::string Options_(const Player& p) const
    {
        const bool has_fish = !Main().board.AliveFishInPool(p.pool).empty();
        std::string text = "您正在 " + std::string(pool_cn[p.pool]) + " 池，池中有 " +
                           std::to_string(Main().board.AliveFishInPool(p.pool).size()) + " 条鱼。可选行动：\n";
        if (p.HasBait()) {
            text += "·「钓鱼 <鱼饵>」";
            for (int i = 0; i < BAIT_KIND_NUM; i++) {
                if (p.baits[i] > 0) {
                    text += "　" + std::string(bait_cn[i]) + "×" + std::to_string(p.baits[i]) +
                            "（" + NumStr(bait_nutrition[i]) + " 肥力）";
                }
            }
            text += "\n";
        }
        if (p.CanDig()) {
            text += "·「挖饵」消耗整个回合，挖得一个沙虫（" + NumStr(bait_nutrition[BaitIndex(Bait::SANDWORM)]) +
                    " 肥力），剩余 " + std::to_string(DIG_LIMIT - p.dig_used) + " 次\n";
        }
        if (!p.HasBait() || !has_fish) {
            text += "·「放弃」本回合不下钩\n";
        } else {
            text += "（本池有鱼且您尚有鱼饵，必须下钩，不得放弃）\n";
        }
        return text;
    }

    void FillUnready_(const bool hook)
    {
        for (int i = 0; i < Main().PlayerNum(); i++) {
            Player& p = Main().P(i);
            if (Global().IsReady(i) || !p.Alive()) {
                continue;
            }
            std::vector<int> avail;
            for (int k = 0; k < BAIT_KIND_NUM; k++) {
                if (p.baits[k] > 0) {
                    avail.push_back(k);
                }
            }
            // 本池有鱼且自己有饵时必须下钩，此时随机挑一个鱼饵；否则放弃钓鱼
            if (!avail.empty() && !Main().board.AliveFishInPool(p.pool).empty()) {
                p.fisher_act = FisherAct::FISHING;
                p.bait = static_cast<Bait>(Main().board.RandomPick(avail));
                Global().Tell(i) << "您未在时限内行动，已随机为您系上「" << BaitName(p.bait) << "」下钩";
            } else {
                p.fisher_act = FisherAct::GIVE_UP;
                Global().Tell(i) << "您未在时限内行动，本回合默认放弃钓鱼";
            }
            if (hook) {
                Global().Hook(i);
            }
        }
    }

    bool CheckCommon_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply)
    {
        if (Main().P(pid).IsFish()) {
            reply() << "[错误] 您本局的身份是「鱼」，无法执行渔夫的行动";
            return false;
        }
        if (is_public) {
            reply() << "[错误] 请私信裁判进行行动";
            return false;
        }
        if (Global().IsReady(pid)) {
            reply() << "[错误] 您本回合已经确定了行动";
            return false;
        }
        return true;
    }

    AtomReqErrCode Fish_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply, const Bait bait)
    {
        if (!CheckCommon_(pid, is_public, reply)) {
            return StageErrCode::FAILED;
        }
        Player& p = Main().P(pid);
        if (p.baits[BaitIndex(bait)] <= 0) {
            reply() << "[错误] 下钩失败：您已经没有「" << BaitName(bait) << "」了";
            return StageErrCode::FAILED;
        }
        p.fisher_act = FisherAct::FISHING;
        p.bait = bait;
        reply() << "您将「" << BaitName(bait) << "」系上鱼钩，缓缓沉入 " << pool_cn[p.pool] << " 池";
        return StageErrCode::READY;
    }

    AtomReqErrCode Dig_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply)
    {
        if (!CheckCommon_(pid, is_public, reply)) {
            return StageErrCode::FAILED;
        }
        Player& p = Main().P(pid);
        if (p.pool != POOL_B) {
            reply() << "[错误] 挖饵失败：只有 B 池的岸边才有沙虫可挖";
            return StageErrCode::FAILED;
        }
        if (p.dig_used >= DIG_LIMIT) {
            reply() << "[错误] 挖饵失败：您已经挖过 " << DIG_LIMIT << " 次，这片滩涂再无沙虫";
            return StageErrCode::FAILED;
        }
        p.fisher_act = FisherAct::DIG;
        reply() << "您放下渔竿，蹲在滩涂上翻找沙虫，本回合将不会下钩";
        return StageErrCode::READY;
    }

    AtomReqErrCode GiveUp_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply)
    {
        if (!CheckCommon_(pid, is_public, reply)) {
            return StageErrCode::FAILED;
        }
        Player& p = Main().P(pid);
        const bool has_fish = !Main().board.AliveFishInPool(p.pool).empty();
        if (p.HasBait() && has_fish) {
            reply() << "[错误] 放弃失败：本池尚有游鱼，而您手中还有鱼饵，没有理由空手而归";
            return StageErrCode::FAILED;
        }
        p.fisher_act = FisherAct::GIVE_UP;
        reply() << "您本回合放弃钓鱼";
        return StageErrCode::READY;
    }
};


// ========== 鱼群择钩阶段 ==========

class HookStage : public SubGameStage<>
{
  public:
    HookStage(MainStage& main_stage, const int round)
        : StageFsm(main_stage, "第 " + std::to_string(round) + " 回合 · 择钩",
            MakeStageCommand(*this, "[鱼] 咬住指定渔夫的钩，争抢其鱼饵", &HookStage::Bite_,
                VoidChecker("咬钩", "咬"), ArithChecker<int64_t>(1, MAX_PLAYER, "渔夫编号")),
            MakeStageCommand(*this, "[鱼] 放弃咬钩，转而捕猎一条极度饥饿的同胞", &HookStage::Hunt_,
                VoidChecker("捕猎", "猎"), ArithChecker<int64_t>(1, MAX_PLAYER, "同胞编号")))
    {}

    virtual void OnStageBegin() override
    {
        Main().BoardcastBoard("择钩", false);
        Global().Boardcast() << "鱼钩已入水，请鱼群私信裁判选择咬住哪一个钩。\n"
                             << "各钩的争抢情况将在全员选定后公示，届时再提交投入的体力。\n"
                             << "时限 " << GAME_OPTION(时限) << " 秒，超时随机咬钩";
        for (int i = 0; i < Main().PlayerNum(); i++) {
            Player& p = Main().P(i);
            if (!p.Alive() || p.IsFisherman()) {
                Global().SetReady(i);
                Main().hints_[i] = p.IsFisherman() && p.Alive() ? "本阶段由鱼群择钩，请稍候" : "";
                continue;
            }
            // 本池只有唯一一个钩的鱼已在下饵结算时自动咬钩，本阶段无需再行动
            if (p.fish_act != FishAct::NONE) {
                Global().SetReady(i);
                Main().hints_[i] = "";
                continue;
            }
            const std::vector<int> hooks = Main().board.HooksInPool(p.pool);
            const std::vector<int> targets = Main().board.HuntTargets(i);
            if (hooks.empty() && targets.empty()) {
                Global().SetReady(i);
                Main().hints_[i] = "";
                Global().Tell(i) << "本池水面之下空无一钩，您本回合无从觅食";
                continue;
            }
            Main().hints_[i] = Options_(p, hooks, targets);
            Global().Tell(i) << Main().hints_[i];
        }
        Global().StartTimer(GAME_OPTION(时限));
    }

    virtual CheckoutErrCode OnStageOver() override
    {
        FillUnready_(false);
        return StageErrCode::CHECKOUT;
    }

    virtual CheckoutErrCode OnStageTimeout() override
    {
        FillUnready_(true);
        return StageErrCode::CHECKOUT;
    }

    virtual CheckoutErrCode OnPlayerLeave(const PlayerID pid) override { return StageErrCode::CONTINUE; }

    virtual AtomReqErrCode OnComputerAct(const PlayerID pid, ChildMsgSenderBase& reply) override
    {
        if (Global().IsReady(pid)) {
            return StageErrCode::OK;
        }
        Player& p = Main().P(pid);
        const std::vector<int> hooks = Main().board.HooksInPool(p.pool);
        const std::vector<int> targets = Main().board.HuntTargets(pid);
        if (!targets.empty() && (hooks.empty() || Main().board.RandomInt(4) == 0)) {
            p.fish_act = FishAct::HUNT;
            p.hunt_target = Main().board.RandomPick(targets);
        } else if (!hooks.empty()) {
            p.fish_act = FishAct::BITE;
            p.hook = Main().board.RandomPick(hooks);
        }
        return StageErrCode::READY;
    }

  private:
    std::string Options_(const Player& p, const std::vector<int>& hooks, const std::vector<int>& targets) const
    {
        std::string text = "您正在 " + std::string(pool_cn[p.pool]) + " 池，当前肥度 " + NumStr(p.fat) +
                           (p.Starving() ? "（极度饥饿）" : "") + "。可选行动：\n";
        if (hooks.empty()) {
            text += "·本池没有任何鱼钩\n";
        }
        for (const int f : hooks) {
            text += "·「咬钩 " + PlayerNo(f) + "」" + Main().P(f).name + " 的" + BaitName(Main().P(f).bait) +
                    "（" + NumStr(bait_nutrition[BaitIndex(Main().P(f).bait)]) + " 肥力）\n";
        }
        for (const int t : targets) {
            text += "·「捕猎 " + PlayerNo(t) + "」" + Main().P(t).name + "，肥度 " + NumStr(Main().P(t).fat) +
                    "\n";
        }
        if (!targets.empty()) {
            text += "（捕猎与咬钩只能二选一。捕猎需在下一阶段猜中对方投入的体力，猜中则将其吞噬并与其他猜中者平分 " +
                    NumStr(HUNT_REWARD) + " 点肥度；本池若无任何鱼钩，则直接吞噬）\n";
        }
        return text;
    }

    void FillUnready_(const bool hook)
    {
        for (int i = 0; i < Main().PlayerNum(); i++) {
            Player& p = Main().P(i);
            if (Global().IsReady(i) || !p.Alive()) {
                continue;
            }
            const std::vector<int> hooks = Main().board.HooksInPool(p.pool);
            // 超时不会发起捕猎，本池若有鱼钩则随机咬住其中一个
            if (!hooks.empty()) {
                p.fish_act = FishAct::BITE;
                p.hook = Main().board.RandomPick(hooks);
                Global().Tell(i) << "您未在时限内行动，已随机为您咬住 " << PlayerNo(p.hook) << " 号的钩";
            } else {
                Global().Tell(i) << "您未在时限内行动，本回合不发起捕猎";
            }
            if (hook) {
                Global().Hook(i);
            }
        }
    }

    bool CheckCommon_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply)
    {
        if (Main().P(pid).IsFisherman()) {
            reply() << "[错误] 您本局的身份是「渔夫」，无法执行鱼的行动";
            return false;
        }
        if (is_public) {
            reply() << "[错误] 请私信裁判进行行动";
            return false;
        }
        if (Global().IsReady(pid)) {
            reply() << "[错误] 您本回合已经做出了选择";
            return false;
        }
        return true;
    }

    AtomReqErrCode Bite_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply, const int64_t no)
    {
        if (!CheckCommon_(pid, is_public, reply)) {
            return StageErrCode::FAILED;
        }
        Player& p = Main().P(pid);
        const int target = static_cast<int>(no) - 1;
        if (target < 0 || target >= Main().PlayerNum()) {
            reply() << "[错误] 咬钩失败：场上没有 " << no << " 号玩家";
            return StageErrCode::FAILED;
        }
        const Player& f = Main().P(target);
        if (!f.HasHook() || f.pool != p.pool) {
            reply() << "[错误] 咬钩失败：" << no << " 号并未在本池投下鱼钩";
            return StageErrCode::FAILED;
        }
        p.fish_act = FishAct::BITE;
        p.hook = target;
        p.hunt_target = -1;
        reply() << "您悄悄靠近了 " << no << " 号的钩，上面挂着「" << BaitName(f.bait) << "」";
        return StageErrCode::READY;
    }

    AtomReqErrCode Hunt_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply, const int64_t no)
    {
        if (!CheckCommon_(pid, is_public, reply)) {
            return StageErrCode::FAILED;
        }
        Player& p = Main().P(pid);
        if (p.Starving()) {
            reply() << "[错误] 捕猎失败：您自己也已陷入极度饥饿，无力捕猎同胞";
            return StageErrCode::FAILED;
        }
        const int target = static_cast<int>(no) - 1;
        const std::vector<int> targets = Main().board.HuntTargets(pid);
        if (std::find(targets.begin(), targets.end(), target) == targets.end()) {
            reply() << "[错误] 捕猎失败：" << no << " 号不是本池中陷入极度饥饿的鱼";
            return StageErrCode::FAILED;
        }
        p.fish_act = FishAct::HUNT;
        p.hunt_target = target;
        p.hook = -1;
        reply() << "您盯上了 " << no << " 号，放弃了本回合的鱼饵。请在下一阶段猜测它投入的体力";
        return StageErrCode::READY;
    }
};


// ========== 体力提交阶段 ==========

class StaminaStage : public SubGameStage<>
{
  public:
    StaminaStage(MainStage& main_stage, const int round)
        : StageFsm(main_stage, "第 " + std::to_string(round) + " 回合 · 角力",
            MakeStageCommand(*this, "提交本回合的体力数值", &StaminaStage::Submit_,
                AlterChecker<int>(k_stamina_map)),
            MakeStageCommand(*this, "提交本回合的体力数值（带前缀）", &StaminaStage::SubmitWithPrefix_,
                VoidChecker("体力", "猜测", "垂钓"), AlterChecker<int>(k_stamina_map)))
    {}

    virtual void OnStageBegin() override
    {
        Main().SetPhase("角力");
        Global().Boardcast() << "各钩的争抢情况已公示，请相关玩家私信裁判提交数值。\n"
                             << "鱼提交投入的体力（0 ~ 5，0.5 的倍数），渔夫猜测钩上鱼的体力（0.5 ~ 5）。\n"
                             << "双方同时提交，时限 " << GAME_OPTION(时限) << " 秒";
        for (int i = 0; i < Main().PlayerNum(); i++) {
            Player& p = Main().P(i);
            Main().hints_[i] = "";
            if (!p.Alive()) {
                Global().SetReady(i);
                continue;
            }
            if (p.IsFish()) {
                if (p.fish_act == FishAct::BITE) {
                    Main().hints_[i] = "直接发送 0 ~ 5 之间 0.5 的倍数，提交争抢 " + PlayerNo(p.hook) +
                                       " 号鱼饵投入的体力，会在回合结束时从肥度中扣除，"
                                       "若被渔夫猜中则会被钓上，若未猜中则由投入最高者平分鱼饵";
                } else if (p.fish_act == FishAct::HUNT && !Main().board.HooksInPool(p.pool).empty()) {
                    Main().hints_[i] = "直接发送 0 ~ 5 之间 0.5 的倍数，猜测 " + PlayerNo(p.hunt_target) +
                                       " 号为争抢鱼饵投入的体力。猜中即可将其吞噬";
                }
            } else if (p.HasHook() && !Main().board.BitersOf(i).empty()) {
                Main().hints_[i] = "直接发送 0.5 ~ 5 之间 0.5 的倍数，猜测咬住您鱼钩的鱼投入了多少体力。"
                                   "本钩共有 " + std::to_string(Main().board.BitersOf(i).size()) +
                                   " 条鱼争抢，猜中者全部上钩；猜 5 时，投入 0 体力的鱼也会一并上钩";
            }
            if (Main().hints_[i].empty()) {
                Global().SetReady(i);
            } else {
                Global().Tell(i) << Main().hints_[i];
            }
        }
        Global().StartTimer(GAME_OPTION(时限));
    }

    virtual CheckoutErrCode OnStageOver() override
    {
        FillUnready_(false);
        return StageErrCode::CHECKOUT;
    }

    virtual CheckoutErrCode OnStageTimeout() override
    {
        FillUnready_(true);
        return StageErrCode::CHECKOUT;
    }

    virtual CheckoutErrCode OnPlayerLeave(const PlayerID pid) override { return StageErrCode::CONTINUE; }

    virtual AtomReqErrCode OnComputerAct(const PlayerID pid, ChildMsgSenderBase& reply) override
    {
        if (Global().IsReady(pid)) {
            return StageErrCode::OK;
        }
        Player& p = Main().P(pid);
        if (p.IsFish()) {
            if (p.fish_act == FishAct::BITE) {
                p.stamina = Main().board.RandomInt(STAMINA_MAX + 1);
            } else {
                p.hunt_guess = Main().board.RandomInt(STAMINA_MAX + 1);
            }
        } else {
            p.guess = GUESS_MIN + Main().board.RandomInt(STAMINA_MAX - GUESS_MIN + 1);
        }
        return StageErrCode::READY;
    }

  private:
    void FillUnready_(const bool hook)
    {
        for (int i = 0; i < Main().PlayerNum(); i++) {
            Player& p = Main().P(i);
            if (Global().IsReady(i) || !p.Alive()) {
                continue;
            }
            if (p.IsFish()) {
                if (p.fish_act == FishAct::BITE) {
                    p.stamina = STAMINA_MIN;
                    Global().Tell(i) << "您未在时限内提交，默认投入 " << StaminaStr(STAMINA_MIN) << " 点体力";
                } else {
                    p.hunt_guess = STAMINA_MIN;
                    Global().Tell(i) << "您未在时限内提交，默认猜测 " << StaminaStr(STAMINA_MIN) << " 点体力";
                }
            } else {
                p.guess = GUESS_FULL;
                Global().Tell(i) << "您未在时限内提交，默认猜测 " << StaminaStr(GUESS_FULL) << " 点体力";
            }
            if (hook) {
                Global().Hook(i);
            }
        }
    }

    AtomReqErrCode Submit_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply, const int value)
    {
        if (is_public) {
            reply() << "[错误] 请私信裁判提交数值";
            return StageErrCode::FAILED;
        }
        if (Global().IsReady(pid)) {
            reply() << "[错误] 您本回合已经提交过数值，或您本阶段无需行动";
            return StageErrCode::FAILED;
        }
        Player& p = Main().P(pid);
        if (p.IsFisherman()) {
            if (value < GUESS_MIN) {
                reply() << "[错误] 垂钓失败：您无法猜测 " << StaminaStr(STAMINA_MIN) << " 点体力，最低只能猜 " << StaminaStr(GUESS_MIN);
                return StageErrCode::FAILED;
            }
            p.guess = value;
            reply() << "您屏息凝神，认定钩上的鱼投入了 " << StaminaStr(value) << " 点体力"
                    << (value == GUESS_FULL ? "（投入 0 点体力的鱼也将一并上钩）" : "");
        } else if (p.fish_act == FishAct::BITE) {
            p.stamina = value;
            reply() << "您为这口鱼饵投入了 " << StaminaStr(value) << " 点体力";
        } else {
            p.hunt_guess = value;
            reply() << "您猜测 " << PlayerNo(p.hunt_target) << " 号投入了 " << StaminaStr(value) << " 点体力";
        }
        return StageErrCode::READY;
    }

    AtomReqErrCode SubmitWithPrefix_(const PlayerID pid, const bool is_public, ChildMsgSenderBase& reply, const int value)
    {
        return Submit_(pid, is_public, reply, value);
    }
};


// ========== 主阶段流程 ==========

void MainStage::FirstStageFsm(SubStageFsmSetter setter)
{
    board.max_round = GAME_OPTION(回合数);
    board.players.reserve(PlayerNum());
    for (int i = 0; i < PlayerNum(); i++) {
        board.players.emplace_back(i, Global().PlayerName(i), Global().PlayerAvatar(i, 20));
    }
    Global().Boardcast() << "【钓鱼】\n"
                         << "两口池塘并列排开，分为 A、B 两池。\n"
                         << "鱼为食物而来，只要不饿死就是胜利；渔夫为不空军而来，只要带着鱼回家就是胜利。\n"
                         << "本局共 " << PlayerNum() << " 人，将进行 " << board.max_round << " 回合。"
                         << "随时发送「赛况」查看全局，私信「状态」查看自己本回合可执行的行动。";
#ifdef TEST_BOT
    if (!GAME_OPTION(渔夫列表).empty()) {
        std::vector<int> fishermen;
        for (const uint32_t no : GAME_OPTION(渔夫列表)) {
            if (no >= 1 && static_cast<int>(no) <= PlayerNum()) {
                fishermen.push_back(static_cast<int>(no) - 1);
            }
        }
        AssignRoles(fishermen);
        AnnounceRoles();
        round_ = 1;
        setter.Emplace<BaitStage>(*this, round_);
        return;
    }
#endif
    if (GAME_OPTION(任命模式)) {
        std::vector<int> candidates;
        for (const Player& p : board.players) {
            candidates.push_back(p.pid);
        }
#ifdef TEST_BOT
        appointer_ = 0;
#else
        appointer_ = board.RandomPick(candidates);
#endif
        setter.Emplace<AppointStage>(*this);
        return;
    }
    AssignRolesRandomly();
    AnnounceRoles();
    round_ = 1;
    setter.Emplace<BaitStage>(*this, round_);
}

void MainStage::NextStageFsm(AppointStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter)
{
    if (!roles_assigned_) {
        AssignRolesRandomly();
    }
    // 任命阶段有人退出时以剩余人数开局，人数不足则中止本局
    if (AliveCount() < MIN_PLAYER) {
        aborted_ = true;
        game_over_ = true;
        Global().Boardcast() << "在场人数已不足 " << MIN_PLAYER << " 人，本局游戏中止";
        return;
    }
    AnnounceRoles();
    round_ = 1;
    setter.Emplace<BaitStage>(*this, round_);
}

void MainStage::NextStageFsm(MoveStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter)
{
    SettleMove();
    for (const int pid : pending_eliminate_) {
        Global().Eliminate(PlayerID(pid));
    }
    pending_eliminate_.clear();
    if (!report_.empty()) {
        Global().Boardcast() << "【第 " << round_ << " 回合 · 洄游】\n" << report_;
        report_.clear();
    }
    if (CheckGameEnd(false)) {
        FinishGame();
        return;
    }
    setter.Emplace<BaitStage>(*this, round_);
}

void MainStage::NextStageFsm(BaitStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter)
{
    // 结算渔夫的行动：下钩即消耗鱼饵，挖饵即入账一个沙虫
    {
        auto sender = Global().Boardcast();
        sender << "【第 " << round_ << " 回合 · 渔夫行动公示】";
        for (Player& p : board.players) {
            if (!p.IsFisherman() || p.fisher_act == FisherAct::NONE) {
                continue;
            }
            sender << "\n" << PlayerNo(p.pid) << " 号（" << pool_cn[p.pool] << " 池）：";
            if (p.fisher_act == FisherAct::FISHING) {
                p.baits[BaitIndex(p.bait)]--;
                sender << "下钩 · " << BaitName(p.bait) << "（" << NumStr(bait_nutrition[BaitIndex(p.bait)])
                       << " 肥力）";
            } else if (p.fisher_act == FisherAct::DIG) {
                p.dig_used++;
                p.baits[BaitIndex(Bait::SANDWORM)]++;
                sender << "挖得一个沙虫，本回合不下钩（已挖 " << p.dig_used << " / " << DIG_LIMIT << " 次）";
            } else {
                sender << "放弃钓鱼";
            }
        }
    }

    // 没有选择余地的鱼直接替其咬钩，若全场无人需要抉择则整个择钩阶段都可以略过
    bool need_hook_stage = false;
    bool any_auto = false;
    for (int i = 0; i < PlayerNum(); i++) {
        const Player& p = P(i);
        if (!p.Alive() || !p.IsFish()) {
            continue;
        }
        if (AutoBiteIfNoChoice(i)) {
            need_hook_stage = true;
        } else if (p.fish_act == FishAct::BITE) {
            any_auto = true;
        }
    }
    if (need_hook_stage) {
        setter.Emplace<HookStage>(*this, round_);
        return;
    }
    if (any_auto) {
        Global().Boardcast() << "水中只有唯一的选择，鱼群已尽数扑向鱼钩，本回合略过择钩";
    } else {
        Global().Boardcast() << "水面之下风平浪静，本回合无鱼可觅食";
    }
    AfterHooksDecided(setter);
}

// 本池只有一个鱼钩且没有可捕猎的目标时，咬住这个钩是唯一的合法行动，直接替其决定
bool MainStage::AutoBiteIfNoChoice(const int pid)
{
    Player& p = P(pid);
    const std::vector<int> hooks = board.HooksInPool(p.pool);
    if (!board.HuntTargets(pid).empty() || hooks.size() > 1) {
        return true;
    }
    if (hooks.size() == 1) {
        p.fish_act = FishAct::BITE;
        p.hook = hooks[0];
        Global().Tell(pid) << "本池只有 " << PlayerNo(hooks[0]) << " 号的一个鱼钩，也没有可以捕猎的同胞，"
                              "您已自动咬住这个钩，稍后直接提交投入的体力即可";
    }
    return false;
}

void MainStage::NextStageFsm(HookStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter)
{
    AfterHooksDecided(setter);
}

void MainStage::AnnounceBites()
{
    // 公示各钩的争抢情况，鱼与渔夫同时获知
    auto sender = Global().Boardcast();
    sender << "【第 " << round_ << " 回合 · 争抢情况公示】";
    bool any_hook = false;
    for (const Player& p : board.players) {
        if (!p.HasHook()) {
            continue;
        }
        any_hook = true;
        const std::vector<int> biters = board.BitersOf(p.pid);
        sender << "\n" << PlayerNo(p.pid) << " 号的" << BaitName(p.bait) << "（" << pool_cn[p.pool] << " 池）：";
        if (biters.empty()) {
            sender << "无鱼问津";
            continue;
        }
        sender << biters.size() << " 条鱼争抢　";
        for (const int b : biters) {
            sender << PlayerNo(b) << " 号　";
        }
    }
    if (!any_hook) {
        sender << "\n本回合水中没有任何鱼钩";
    }
}

void MainStage::AfterHooksDecided(SubStageFsmSetter& setter)
{
    AnnounceBites();

    // 仅当有人需要提交数值时才进入角力阶段
    bool need_stamina_stage = false;
    for (int i = 0; i < PlayerNum(); i++) {
        const Player& p = P(i);
        if (!p.Alive()) {
            continue;
        }
        if (p.IsFish() && p.fish_act == FishAct::BITE) {
            need_stamina_stage = true;
        } else if (p.IsFish() && p.fish_act == FishAct::HUNT && !board.HooksInPool(p.pool).empty()) {
            need_stamina_stage = true;
        } else if (p.IsFisherman() && p.HasHook() && !board.BitersOf(i).empty()) {
            need_stamina_stage = true;
        }
    }
    if (!need_stamina_stage) {
        FinishRound(setter);
        return;
    }
    setter.Emplace<StaminaStage>(*this, round_);
}

void MainStage::NextStageFsm(StaminaStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter)
{
    FinishRound(setter);
}


// ========== 身份 ==========

void MainStage::AssignRoles(const std::vector<int>& fishermen)
{
    for (Player& p : board.players) {
        p.role = Role::FISH;
    }
    for (const int pid : fishermen) {
        Player& p = P(pid);
        p.role = Role::FISHERMAN;
        for (int i = 0; i < BAIT_KIND_NUM; i++) {
            p.baits[i] = bait_init_count[i];
        }
    }
    roles_assigned_ = true;
}

void MainStage::AssignRolesRandomly()
{
    std::vector<int> candidates;
    for (const Player& p : board.players) {
        if (p.Alive()) {
            candidates.push_back(p.pid);
        }
    }
    std::shuffle(candidates.begin(), candidates.end(), board.g);
    const int need = std::min<int>(RequiredFishermen(), static_cast<int>(candidates.size()));
    AssignRoles(std::vector<int>(candidates.begin(), candidates.begin() + need));
}

void MainStage::AnnounceRoles()
{
    auto sender = Global().Boardcast();
    sender << "身份已定：\n【渔夫】";
    for (const Player& p : board.players) {
        if (p.IsFisherman()) {
            sender << "\n　" << PlayerNo(p.pid) << " 号 " << At(PlayerID(p.pid));
        }
    }
    sender << "\n【鱼】";
    for (const Player& p : board.players) {
        if (p.IsFish() && p.Alive()) {
            sender << "\n　" << PlayerNo(p.pid) << " 号 " << At(PlayerID(p.pid));
        }
    }
    sender << "\n\n全员降临 A 池，每条鱼的初始肥度为 " << NumStr(INIT_FAT)
           << "，每回合结束自然消耗 " << NumStr(NATURAL_CONSUME) << " 点。";
    for (const Player& p : board.players) {
        if (!p.Alive()) {
            continue;
        }
        if (p.IsFisherman()) {
            Global().Tell(p.pid) << "您是【渔夫】。初始鱼饵："
                                 << bait_cn[0] << "×" << p.baits[0] << "　"
                                 << bait_cn[1] << "×" << p.baits[1] << "　"
                                 << bait_cn[2] << "×" << p.baits[2]
                                 << "\n每次下钩必定消耗一个鱼饵，钓上的鱼有多少肥度便是多少公斤。"
                                 << "身处 B 池时可以挖鱼饵，全局限 " << DIG_LIMIT << " 次。";
        } else {
            Global().Tell(p.pid) << "您是【鱼】。初始肥度 " << NumStr(INIT_FAT) << "，肥度达到 "
                                 << NumStr(STARVING_LINE) << " 会陷入极度饥饿，达到 " << NumStr(DEAD_LINE)
                                 << " 将饿死腐烂并为水池留下永久瘟疫。活到终局即可获胜。";
        }
    }
}


// ========== 结算 ==========

void MainStage::KillPlayer(const int pid, const OutReason reason)
{
    Player& p = P(pid);
    if (p.out != 0) {
        return;
    }
    p.out = 1;
    p.out_reason = reason;
    p.out_round = round_;
    pending_eliminate_.push_back(pid);
}

void MainStage::SettleMove()
{
    for (Player& p : board.players) {
        if (!p.Alive() || p.move_to < 0 || p.move_to == p.pool) {
            continue;
        }
        p.pool = p.move_to;
        p.moved = true;
        if (p.IsFish()) {
            p.fat -= MOVE_COST;
            p.detail += "你游向 " + std::string(pool_cn[p.pool]) + " 池，消耗 " + NumStr(MOVE_COST) + " 点肥度\n";
        }
    }
    // 洄游的代价同样可能当场夺去性命，腐烂留下的瘟疫将在本回合的自然消耗中生效
    for (const int pid : board.AliveFish()) {
        Player& p = P(pid);
        if (!p.Starved()) {
            continue;
        }
        board.pools[p.pool].plague++;
        KillPlayer(pid, OutReason::STARVED);
        report_ += "☠ " + PlayerNo(pid) + " 号在洄游途中力竭，饿死腐烂于 " + std::string(pool_cn[p.pool]) + " 池，此池瘟疫 +1\n";
    }
}

void MainStage::SettleHunt()
{
    std::map<int, std::vector<int>> hunters;
    for (const Player& p : board.players) {
        if (p.Alive() && p.IsFish() && p.fish_act == FishAct::HUNT && p.hunt_target >= 0) {
            hunters[p.hunt_target].push_back(p.pid);
        }
    }
    for (const auto& [target, list] : hunters) {
        Player& t = P(target);
        if (!t.Alive()) {
            continue;
        }
        // 本池若无任何鱼钩，饥饿的鱼无处觅食，猎手无需猜测即可直接吞噬
        const bool no_hook = board.HooksInPool(t.pool).empty();
        std::vector<int> winners;
        for (const int h : list) {
            Player& hp = P(h);
            if (!hp.Alive()) {
                continue;
            }
            if (no_hook || hp.hunt_guess == t.stamina) {
                hp.hunt_hit = true;
                winners.push_back(h);
            }
        }
        if (winners.empty()) {
            for (const int h : list) {
                if (P(h).Alive()) {
                    P(h).detail += "你对 " + PlayerNo(target) + " 号发起捕猎，却猜错了它投入的体力，一无所获\n";
                }
            }
            report_ += "🐟 " + PlayerNo(target) + " 号躲过了同胞的獠牙\n";
            continue;
        }
        // 被猎中的鱼本回合视为完全没有参与争抢
        t.fish_act = FishAct::NONE;
        t.hook = -1;
        t.stamina = STAMINA_MIN;
        const Num share = SplitShare(HUNT_REWARD, static_cast<int>(winners.size()));
        std::string names;
        for (const int w : winners) {
            P(w).fat += share;
            P(w).gain += share;
            P(w).detail += "你成功捕猎了 " + PlayerNo(target) + " 号，分得 " + NumStr(share) + " 点肥度\n";
            names += PlayerNo(w) + " 号 ";
        }
        t.detail += "你被同胞盯上了，本回合的行动作废，随后被吞噬\n";
        KillPlayer(target, OutReason::DEVOURED);
        report_ += "🩸 " + PlayerNo(target) + " 号被 " + names + "吞噬，各分得 " + NumStr(share) + " 点肥度\n";
    }
}

void MainStage::SettleFishing()
{
    for (Player& f : board.players) {
        if (!f.HasHook()) {
            continue;
        }
        const std::vector<int> biters = board.BitersOf(f.pid);
        const Num nutrition = bait_nutrition[BaitIndex(f.bait)];
        if (biters.empty()) {
            f.detail += "本回合无鱼咬钩，" + BaitName(f.bait) + "白白沉入水底\n";
            report_ += "🪝 " + PlayerNo(f.pid) + " 号的" + BaitName(f.bait) + "无鱼问津\n";
            continue;
        }
        // 猜中满体力时，投入 0 体力的鱼也会一并被钓上
        std::vector<int> caught;
        if (f.guess >= GUESS_MIN) {
            for (const int b : biters) {
                const int s = P(b).stamina;
                if (s == f.guess || (f.guess == GUESS_FULL && s == STAMINA_MIN)) {
                    caught.push_back(b);
                }
            }
        }
        if (!caught.empty()) {
            std::string names;
            for (const int c : caught) {
                Player& cp = P(c);
                f.weight += cp.Weight();
                f.caught++;
                f.catches.push_back(c);
                board.any_caught = true;
                cp.detail += "你投入的 " + StaminaStr(cp.stamina) + " 点体力恰好被渔夫料中，被生生拽出了水面\n";
                names += PlayerNo(c) + " 号（" + NumStr(cp.Weight()) + " 公斤）";
                KillPlayer(c, OutReason::CAUGHT);
            }
            for (const int b : biters) {
                if (P(b).Alive()) {
                    P(b).detail += "渔夫成功提竿，此钩下无鱼吃到鱼饵，但你也无需付出体力\n";
                }
            }
            f.detail += "您猜中了 " + StaminaStr(f.guess) + " 点体力，成功钓上 " + names + "\n";
            report_ += "🎣 " + PlayerNo(f.pid) + " 号猜中 " + StaminaStr(f.guess) + " 点体力，钓上 " + names + "\n";
            continue;
        }
        // 渔夫落空，由投入体力最高的鱼平分鱼饵，此钩下所有鱼照常扣除投入的体力
        int top = STAMINA_MIN;
        for (const int b : biters) {
            top = std::max(top, P(b).stamina);
        }
        std::vector<int> winners;
        for (const int b : biters) {
            if (P(b).stamina == top) {
                winners.push_back(b);
            }
        }
        const Num share = SplitShare(nutrition, static_cast<int>(winners.size()));
        std::string names;
        for (const int w : winners) {
            P(w).fat += share;
            P(w).gain += share;
            P(w).detail += "你以 " + StaminaStr(top) + " 点体力抢到了" + BaitName(f.bait) + "，得到 " +
                           NumStr(share) + " 点肥度\n";
            names += PlayerNo(w) + " 号 ";
        }
        for (const int b : biters) {
            Player& bp = P(b);
            bp.paid = StaminaToNum(bp.stamina);
            bp.fat -= bp.paid;
            bp.detail += "本次争抢投入 " + StaminaStr(bp.stamina) + " 点体力，扣除 " + NumStr(bp.paid) + " 点肥度\n";
        }
        f.detail += "您猜测 " + StaminaStr(f.guess) + " 点体力，然而鱼线只是轻轻一颤，" + BaitName(f.bait) + "被抢走了\n";
        report_ += "🪝 " + PlayerNo(f.pid) + " 号的" + BaitName(f.bait) + "被 " + names + "以 " + StaminaStr(top) +
                   " 点体力抢得，各分 " + NumStr(share) + " 点肥度\n";
    }
}

void MainStage::SettleConsume()
{
    for (int pool = 0; pool < POOL_NUM; pool++) {
        const Num consume = board.pools[pool].Consume();
        for (const int pid : board.AliveFishInPool(pool)) {
            P(pid).fat -= consume;
            P(pid).detail += "回合结束，自然消耗扣去 " + NumStr(consume) + " 点肥度\n";
        }
    }
    // 饿死腐烂会当场为水池添上瘟疫并追加消耗，如此往复直到不再有鱼饿死
    while (true) {
        std::vector<int> starved;
        for (const int pid : board.AliveFish()) {
            if (P(pid).Starved()) {
                starved.push_back(pid);
            }
        }
        if (starved.empty()) {
            break;
        }
        std::array<int, POOL_NUM> added{};
        for (const int pid : starved) {
            Player& p = P(pid);
            board.pools[p.pool].plague++;
            added[p.pool]++;
            p.detail += "你的肥度终于耗尽，饿死腐烂在 " + std::string(pool_cn[p.pool]) + " 池\n";
            KillPlayer(pid, OutReason::STARVED);
            report_ += "☠ " + PlayerNo(pid) + " 号饿死腐烂于 " + std::string(pool_cn[p.pool]) + " 池，此池瘟疫 +1\n";
        }
        for (int pool = 0; pool < POOL_NUM; pool++) {
            if (added[pool] == 0) {
                continue;
            }
            const Num extra = added[pool] * PLAGUE_CONSUME;
            for (const int pid : board.AliveFishInPool(pool)) {
                P(pid).fat -= extra;
                P(pid).detail += "腐尸带来的瘟疫当场追加扣去 " + NumStr(extra) + " 点肥度\n";
            }
        }
    }
}

void MainStage::FinishRound(SubStageFsmSetter& setter)
{
    SettleHunt();
    SettleFishing();
    SettleConsume();

    for (const int pid : pending_eliminate_) {
        Global().Eliminate(PlayerID(pid));
    }
    pending_eliminate_.clear();

    BoardcastBoard("结算", true);
    if (!report_.empty()) {
        Global().Boardcast() << "【第 " << round_ << " 回合】\n" << report_;
    } else {
        Global().Boardcast() << "【第 " << round_ << " 回合】水面静如明镜，无事发生";
    }
    report_.clear();
    for (Player& p : board.players) {
        if (p.detail.empty()) {
            continue;
        }
        Global().Tell(p.pid) << "【第 " << round_ << " 回合结算】\n" << p.detail
                             << (p.IsFish() ? "当前肥度：" + NumStr(p.fat) : "累计公斤数：" + NumStr(p.weight));
    }

    if (CheckGameEnd(true)) {
        FinishGame();
        return;
    }
    round_++;
    for (Player& p : board.players) {
        p.ClearRoundData();
    }
    setter.Emplace<MoveStage>(*this, round_);
}

bool MainStage::CheckGameEnd(const bool check_round_limit)
{
    if (board.AliveFishermen().empty()) {
        Global().Boardcast() << "岸边再无执竿之人，游戏结束";
        return true;
    }
    if (board.AliveFish().empty()) {
        Global().Boardcast() << "两口池塘之中再无游动的鱼，游戏结束";
        return true;
    }
    bool any_bait = false;
    for (const int pid : board.AliveFishermen()) {
        if (P(pid).HasBait()) {
            any_bait = true;
        }
    }
    if (!any_bait) {
        Global().Boardcast() << "所有渔夫的鱼饵都已耗尽，再无鱼可钓，游戏结束";
        return true;
    }
    if (check_round_limit && round_ >= board.max_round) {
        Global().Boardcast() << "第 " << board.max_round << " 回合结束，夜色渐深，游戏结束";
        return true;
    }
    return false;
}

void MainStage::FinishGame()
{
    game_over_ = true;
    if (aborted_) {
        return;
    }

    // 存活的鱼按肥度排名，并列者同名次，因此并列第一时不再产生第二
    for (const int pid : board.AliveFish()) {
        P(pid).rank = board.FatRank(pid);
    }
    Num top_weight = 0;
    for (const Player& p : board.players) {
        if (p.IsFisherman()) {
            top_weight = std::max(top_weight, p.weight);
        }
    }
    for (Player& p : board.players) {
        if (p.IsFish()) {
            if (!p.Alive()) {
                continue;
            }
            p.score.base = Score::FISH_SURVIVE;
            if (p.rank == 1) {
                p.score.extra = Score::FISH_FIRST;
            } else if (p.rank == 2) {
                p.score.extra = Score::FISH_SECOND;
            }
            if (!board.any_caught) {
                p.score.group = Score::FISH_GROUP;
            }
        } else {
            if (p.caught > 0) {
                p.score.base = Score::FISHER_CATCH;
            }
            if (top_weight > 0 && p.weight == top_weight) {
                p.score.extra = Score::FISHER_TOP;
            }
        }
    }
    for (int i = 0; i < PlayerNum(); i++) {
        player_scores_[i] = P(i).score.FinalScore();
    }

    auto sender = Global().Boardcast();
    const std::vector<int> survivors = board.AliveFish();
    if (survivors.empty()) {
        sender << "两口池塘归于死寂，再没有一条鱼活着见到第二天的日出。";
    } else {
        sender << "天光渐亮，活着游进晨光里的鱼：";
        for (const int pid : survivors) {
            sender << "\n　" << PlayerNo(pid) << " 号 " << At(PlayerID(pid)) << "　肥度 " << NumStr(P(pid).fat)
                   << (P(pid).rank == 1 ? "（最肥）" : (P(pid).rank == 2 ? "（次肥）" : ""));
        }
        if (!board.any_caught) {
            sender << "\n\n自始至终无一条鱼被钓上，鱼群取得群体胜利！";
        }
    }
    if (top_weight > 0) {
        sender << "\n\n满载而归的渔夫：";
        for (const Player& p : board.players) {
            if (p.IsFisherman() && p.weight == top_weight) {
                sender << "\n　" << PlayerNo(p.pid) << " 号 " << At(PlayerID(p.pid)) << "　" << NumStr(p.weight)
                       << " 公斤（" << p.caught << " 条）";
            }
        }
    } else {
        sender << "\n\n所有渔夫全部空军，无人能向对象交待。";
    }
    Global().Boardcast() << Markdown(board.ScoreHtml(), Board::ImageWidth());
}

void MainStage::OnPlayerLeave(const PlayerID pid)
{
    Player& p = P(pid);
    if (p.out != 0) {
        return;
    }
    p.out = 2;
    p.out_reason = OutReason::QUIT;
    p.out_round = round_;
    p.score.quit = Score::QUIT_SCORE;
    if (p.IsFish()) {
        // 退出的鱼当即死去，其尸身同样会腐烂并为水池留下永久的瘟疫
        p.fish_act = FishAct::NONE;
        p.hook = -1;
        p.hunt_target = -1;
        board.pools[p.pool].plague++;
        Global().Boardcast() << At(pid) << " 离开了水池，腐烂在 " << pool_cn[p.pool] << " 池，瘟疫 +1";
    } else {
        // 已经投下的钩本回合仍会收线，视为猜测满体力
        const bool has_hook = p.HasHook();
        if (has_hook) {
            p.guess = GUESS_FULL;
        }
        Global().Boardcast() << At(pid) << " 收起渔具离开了池边" << (has_hook ? "，其已投下的鱼钩本回合仍会收线" : "");
    }
}

auto* MakeMainStage(MainStageFactory factory) { return factory.Create<MainStage>(); }

} // namespace GAME_MODULE_NAME

} // namespace game

} // namespace lgtbot
