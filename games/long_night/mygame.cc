// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).

#include <random>
#include <queue>
#include <regex>
#include <span>

#include "game_framework/stage.h"
#include "game_framework/util.h"
#include "utility/html.h"

using namespace std;

#include "grid.h"
#include "map.h"
#include "board.h"

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

class MainStage;
template <typename... SubStages> using SubGameStage = StageFsm<MainStage, SubStages...>;
template <typename... SubStages> using MainGameStage = StageFsm<void, SubStages...>;
const GameProperties k_properties { 
    .name_ = "漫漫长夜", // the game name which should be unique among all the games
    .developer_ = "铁蛋",
    .description_ = "在漆黑的迷宫中探索，根据有限的线索展开追击与逃生",
    .shuffled_player_id_ = true,
};
uint64_t MaxPlayerNum(const MyGameOptions& options) { return 8; }
uint32_t Multiple(const MyGameOptions& options) { return 1; }
const MutableGenericOptions k_default_generic_options{
    .is_formal_{false},
};
const std::vector<RuleCommand> k_rule_commands = {};

bool AdaptOptions(MsgSenderBase& reply, MyGameOptions& game_options, const GenericOptions& generic_options_readonly, MutableGenericOptions& generic_options)
{
    if (GET_OPTION_VALUE(game_options, 特殊事件) == -1) {
        GET_OPTION_VALUE(game_options, 特殊事件) = rand() % 3 + 1;
    }
    if (generic_options_readonly.PlayerNum() > 6 && GET_OPTION_VALUE(game_options, 边长) < 12) {
        GET_OPTION_VALUE(game_options, 边长) = 12;
        reply() << "[警告] 玩家数 " << generic_options_readonly.PlayerNum() << " 超出普通地图限制，自动将地图边长调整为 12*12";
    }
    return true;
}

const std::vector<InitOptionsCommand> k_init_options_commands = {
    InitOptionsCommand("设定特殊事件或游戏模式",
            [] (MyGameOptions& game_options, MutableGenericOptions& generic_options, const int32_t mode)
            {
                switch (mode) {
                    case -1:
                    case 1:
                    case 2:
                    case 3:
                        GET_OPTION_VALUE(game_options, 特殊事件) = mode; break;
                    case 10:
                    case 12:
                        GET_OPTION_VALUE(game_options, 边长) = mode; break;
                    case 20:
                        GET_OPTION_VALUE(game_options, 大乱斗) = true; break;
                    case 21:
                    case 22:
                        GET_OPTION_VALUE(game_options, 隐匿) = mode - 20; break;
                    case 23:
                        GET_OPTION_VALUE(game_options, 点杀) = true; break;
                    case 24:
                        GET_OPTION_VALUE(game_options, BOSS) = true; break;
                    case 30:
                    case 31:
                    case 32:
                    case 33:
                        GET_OPTION_VALUE(game_options, 模式) = mode - 30; break;
                    case 100:
                        return NewGameMode::SINGLE_USER;
                    default:;
                }
                return NewGameMode::MULTIPLE_USERS;
            },
            AlterChecker<int32_t>({
                {"单机", 100}, {"随机", -1}, {"怠惰的园丁", 1}, {"营养过剩", 2}, {"雨天小故事", 3},
                {"10*10", 10}, {"12*12", 12}, {"大乱斗", 20}, {"回合隐匿", 21}, {"单步隐匿", 22}, {"点杀", 23}, {"BOSS", 24},
                {"标准", 30}, {"狂野", 31}, {"幻变", 32}, {"疯狂", 33}
            })),
};

// ========== GAME STAGES ==========

class RoundStage;

class MainStage : public MainGameStage<RoundStage>
{
  public:
    MainStage(StageUtility&& utility)
        : StageFsm(std::move(utility),
            MakeStageCommand(*this, "查看所有区块信息和图例", &MainStage::BlockInfo_, VoidChecker("地图")),
            MakeStageCommand(*this, "指定区块生成地图预览，逃生舱区块需要加 'E' 前缀，未知区块用 0 代替", &MainStage::MapPreview_,
                VoidChecker("预览"), RepeatableChecker<BasicChecker<string>>("序号", "2 3 0 11 E1 0 E2 7 9")))
        , round_(0)
        , player_scores_(Global().PlayerNum(), 0)
        , board(Global().ResourceDir(), GAME_OPTION(模式))
    {}

    virtual int64_t PlayerScore(const PlayerID pid) const override { return player_scores_[pid]; }

    std::vector<int64_t> player_scores_;

    // 回合数 
	int round_;
    // 地图
    Board board;
    // 无逃生舱最后生还胜利判定
    bool withoutE_win_ = false;
    // 无逃生舱最后生还判定
    bool withE_win_ = false;

    // 剩余玩家数
    int Alive_() const { return std::count_if(board.players.begin(), board.players.end(), [](const auto& player){ return player.out == 0; }); }
    
  private:
    CompReqErrCode BlockInfo_(const PlayerID pid, const bool is_public, MsgSenderBase& reply)
    {
        auto sender = reply();
        if (GAME_OPTION(特殊事件) > 0) {
            sender << UnitMaps::ShowSpecialEvent(GAME_OPTION(特殊事件)) << "\n";
        }
        if (GAME_OPTION(边长) > 9) {
            sender << "本局游戏地图为 " << GAME_OPTION(边长) << "x" << GAME_OPTION(边长) << "\n";
        }
        sender << Markdown(board.GetAllBlocksInfo(GAME_OPTION(特殊事件)), 1000);
        return StageErrCode::OK;
    }

    CompReqErrCode MapPreview_(const PlayerID pid, const bool is_public, MsgSenderBase& reply, const vector<string>& map_string)
    {
        vector<string> map_str = map_string;
        map_str.resize(board.unitMaps.pos.size(), "0");
        reply() << Markdown(board.MapGenerate(map_str), 60 * (GAME_OPTION(边长) + 1));
        return StageErrCode::OK;
    }
    
    void FirstStageFsm(SubStageFsmSetter setter)
    {
        // Global().SaveMarkdown(board.GetAllBlocksInfo(GAME_OPTION(特殊事件), GAME_OPTION(模式)), 1000);   // 用于生成全部地图列表
        srand((unsigned int)time(NULL));

        auto sender = Global().Boardcast();
        if (!GAME_OPTION(捉捕目标)) {
            sender << "【捉捕顺位】本局玩家的捉捕顺位为相反顺序，捉捕目标变更为上家\n\n";
        }
        if (GAME_OPTION(特殊事件) > 0) {    // 特殊事件
            switch (GAME_OPTION(特殊事件)) {
                case 1: board.unitMaps.SpecialEvent1(); break;
                case 2: board.unitMaps.SpecialEvent2(); break;
                case 3: board.unitMaps.SpecialEvent3(); break;
            }
            sender << UnitMaps::ShowSpecialEvent(GAME_OPTION(特殊事件)) << "\n\n";
        }
        if (GAME_OPTION(边长) == 10) {      // 边长10
            if (board.unitMaps.RandomizeBlockPosition(GAME_OPTION(边长))) {
                sender << "【10*10】本局游戏地图将更改为 " << GAME_OPTION(边长) << "x" << GAME_OPTION(边长) << " 大地图。9个区块在大地图随机排列，区块不会重叠。没有区块覆盖的地图空隙将变成普通道路。\n";
            } else {
                sender << "[错误] 生成10*10地图时发生错误：未能成功随机布局，游戏无法正常开始！";
                return;
            }
        }
        if (GAME_OPTION(边长) == 12) {      // 边长12
            board.unitMaps.SetMapPosition12();
            board.exit_num = 4;
            sender << "【12*12】本局游戏地图将更改为 " << GAME_OPTION(边长) << "x" << GAME_OPTION(边长) << " 大地图。使用 16 个区块铺满地图，逃生舱固定为 4 个。\n";
        }
        if (GAME_OPTION(点杀)) {    // 点杀
            sender << "【点杀模式】捕捉改为仅在回合结束时触发，路过不会触发捕捉\n";
        }
        if (GAME_OPTION(隐匿) == 1) {       // 隐匿
            sender << "【隐匿模式】回合隐匿：隐匿效果持续1回合，**仅可使用1次**，隐匿后当回合的行动转为私聊进行，不会发出声响，不会触发捕捉。\n";
        } else if (GAME_OPTION(隐匿) == 2) {
            sender << "【隐匿模式】单步隐匿：隐匿效果仅作用于下一步，**可使用" << hide_limit << "次**，隐匿后在私聊行动一步，不会发出声响，不会触发捕捉。\n";
        }
        if (GAME_OPTION(大乱斗)) {
            sender << "【大乱斗模式】所有的逃生舱改为随机传送！但仍会统计逃生分\n";
        }

        board.size = GAME_OPTION(边长);
        // 初始化玩家
        board.playerNum = Global().PlayerNum();
        board.players.reserve(board.playerNum);
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            board.players.emplace_back(pid, Global().PlayerName(pid), Global().PlayerAvatar(pid, 40), board.size);
            if (GAME_OPTION(隐匿) == 1) {       // 隐匿
                board.players[pid].hide_remaining = 1;
            } else if (GAME_OPTION(隐匿) == 2) {
                board.players[pid].hide_remaining = hide_limit;
            }
        }
        board.UpdatePlayerTarget(GAME_OPTION(捉捕目标));
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
        board.Initialize(GAME_OPTION(BOSS));

        if (GAME_OPTION(BOSS)) {
            board.boss.all_record = "<br>【开局】初始锁定玩家为 [" + to_string(board.boss.target) + "号]（巨响）";
            sender << "【BOSS】米诺陶斯现身于地图中，会在回合结束时追击最近的玩家。BOSS发出震耳欲聋的巨响！请所有玩家留意BOSS开局所在的方位！\n";
            sender << "当前BOSS锁定的玩家为 " << At(board.boss.target) << "\n";
        }

        sender << "[提示] 本局游戏人数为 " << board.playerNum << " 人，逃生舱数量为 " << board.exit_num << " 个。请留意私信发送的开局墙壁信息";

        // 开局私信墙壁信息
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            auto [info, md] = board.GetSurroundingWalls(pid);
            board.players[pid].private_record = "【开局】\n您所在位置的四周墙壁信息，按照 上下左右 顺序分别是：\n" + info;
            Global().Tell(pid) << board.players[pid].private_record << "\n" << Markdown(md, 110);
        }

        string mode_str[4] = {"标准", "狂野", "幻变", "疯狂"};
        sender << "\n\n【区块模式】" + mode_str[GAME_OPTION(模式)];
        if (GAME_OPTION(模式) > 0) {
            sender << "：本局可能出现的区块种类详见下图所示：\n";
            sender << Markdown(board.GetAllBlocksInfo(GAME_OPTION(特殊事件)), 1000);
        }

        setter.Emplace<RoundStage>(*this, ++round_);
    }

    void NextStageFsm(RoundStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter)
    {
        bool game_end = (Alive_() <= 1 && Global().PlayerNum() > 1) || ((Alive_() <= 0 || board.ExitCount() == 0) && Global().PlayerNum() == 1);
        if (!game_end && round_ < 20) {
            setter.Emplace<RoundStage>(*this, ++round_);
            return;
        }

        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            player_scores_[pid] = board.players[pid].score.FinalScore();
        }

        if (game_end) {
            if (Global().PlayerNum() > 1) {
                Global().Boardcast() << "玩家剩余 " + to_string(Alive_()) + " 人，游戏结束！";
            } else if (board.players[0].out == 2) {
                Global().Boardcast() << "经过 " + to_string(round_) + " 回合，您成功抵达了逃生舱！";
            }
        } else {
            Global().Boardcast() << "回合数到达上限，游戏结束";
        }
        Global().Boardcast() << "完整行动轨迹：\n" << Markdown(board.GetAllRecord(), 500);
        Global().Boardcast() << "玩家分数细则：\n" << Markdown(board.GetAllScore(), 800);
        Global().Boardcast() << Markdown(board.GetFinalBoard(), 120 * (GAME_OPTION(边长) + 1));
/* *********************************************************** */
        // 积分奖励发放
        std::regex pattern(R"(机器人\d+号)");
        bool hasBots = std::ranges::any_of(
            std::views::iota(0u, Global().PlayerNum()),
            [&](unsigned int pid) {
                return std::regex_match(Global().PlayerName(pid), pattern);
            }
        );
        if (hasBots) return;
        if (Global().PlayerNum() == 1) {
            if (board.players[0].out == 1) {
                Global().Boardcast() << "很遗憾，您被淘汰了，未能获得积分奖励";
                return;
            }
            if (round_ > 10) {
                Global().Boardcast() << "未能获得积分奖励，努力在10回合内抵达逃生舱吧！";
                return;
            }
        }
        string pt_message = "新游戏积分已记录";
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            auto name = Global().PlayerName(pid);
            auto start = name.find_last_of('('), end = name.find(')', start);
            std::string id = name.substr(start + 1, end - start - 1);
            int32_t point = (Global().PlayerNum() > 1) ? ((player_scores_[pid] + min(round_ - 1, 7) * 40) * 2) : (round_ <= 10 ? (11 - round_) * 60 : 0);
            point = GAME_OPTION(模式) == 0 ? point : (GAME_OPTION(模式) == 2 ? point * 1.2 : point * 1.3);
            if (point > 0) pt_message += "\n" + id + " " + to_string(point);
        }
        pt_message += "\n「#pt help」查看游戏积分帮助";
        Global().Boardcast() << pt_message;
/* *********************************************************** */
    }
};

class RoundStage : public SubGameStage<>
{
   public:
    RoundStage(MainStage& main_stage, const uint64_t round)
            : StageFsm(main_stage, "第 " + std::to_string(round) + " 回合" ,
                MakeStageCommand(*this, "选择方向，在迷宫中探路", &RoundStage::MakeMove_, AlterChecker<Direct>(direction_map)),
                MakeStageCommand(*this, "主动停止行动，并结束回合", &RoundStage::Stop_, VoidChecker("停止")),
                MakeStageCommand(*this, "使用“隐匿”技能（仅限隐匿模式）", &RoundStage::Hide_, VoidChecker("隐匿")),
                MakeStageCommand(*this, "查看当前回合进展情况", &RoundStage::Status_, VoidChecker("赛况")),
                MakeStageCommand(*this, "查看所有玩家完整行动轨迹", &RoundStage::AllStatus_, VoidChecker("完整赛况")))
    {
        step = 0;
        currentPlayer = 0;
        if (Main().Alive_() == 0) {
            Global().Boardcast() << "[错误] 发生了意料之外的错误：无可行动玩家但游戏仍未判定结束，请联系管理员或中断游戏！";
            return;
        }
        while (Main().board.players[currentPlayer].out > 0) {
            currentPlayer = currentPlayer + 1;
        }
        hide = false;
        active_stop = false;
    }

    // 当前行动玩家
    PlayerID currentPlayer;
    // 步数
    int step;
    // 隐匿状态
    bool hide;
    // 主动停止或超时
    bool active_stop;
    
    // 记录门的状态发生过改变
    bool door_modified = false;

    virtual void OnStageBegin() override
    {
        Global().SaveMarkdown(Main().board.GetBoard(Main().board.grid_map), 60 * (GAME_OPTION(边长) + 1));

        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            Main().board.players[pid].move_record = "";
            if (pid != currentPlayer) {
                Global().SetReady(pid);
            }
        }
        Global().Boardcast() << Markdown(Main().board.GetPlayerTable(Main().round_));
        uint32_t think_time = Main().board.players[currentPlayer].hook_status ? 30 : GAME_OPTION(思考时限);
        Global().Boardcast() << "请 " << At(currentPlayer) << " 在公屏选择方向移动（第一次行动前有 " << think_time << " 秒思考时间，行动开始后总时限为 " << GAME_OPTION(行动时限) << " 秒）";
        Global().StartTimer(think_time);
        if (Main().round_ == 1) {
            if (GAME_OPTION(BOSS)) SendSoundMessage(Main().board.boss.x, Main().board.boss.y, Sound::BOSS, true);
            Global().Boardcast() << "可尝试使用「预览」指令生成自定义地图来记录草稿，格式例如：预览 2 3 0 11 E1 0 E2（其中E前缀表示逃生舱）"
                                 << "\n\n!!!【重要提醒】!!! 请注意：当前版本主动停止或超时将无法得知四周墙壁信息！";
        }
    }

   private:
    AtomReqErrCode MakeMove_(const PlayerID pid, const bool is_public, MsgSenderBase& reply, Direct direction)
    {
        Player& player = Main().board.players[pid];
        if (player.out == 2) {
            reply() << "您已乘坐逃生舱撤离，无需继续行动";
            return StageErrCode::FAILED;
        }
        if (pid != currentPlayer) {
            reply() << "[错误] 不是您的回合，当前正在行动玩家是：" << Global().PlayerName(currentPlayer);
            return StageErrCode::FAILED;
        }
        if (!is_public && Global().PlayerNum() > 1) {
            if (GAME_OPTION(隐匿) == 0) {
                reply() << "[错误] 请全程在公屏进行行动";
                return StageErrCode::FAILED;
            }
            if (!hide) {
                reply() << "[错误] 您未使用隐匿技能，请在公屏进行行动";
                return StageErrCode::FAILED;
            }
        }

        if (player.hook_status) player.hook_status = false;
        
        bool success = Main().board.MakeMove(pid, direction, hide);
        step++;

        // 撞墙直接切换下一个玩家
        if (!success) {
            if (step == 1) {
                reply() << UnitMaps::RandomHint(UnitMaps::firststep_wall_hints) << "\n移动时碰撞【墙壁】，本回合结束！**请留意机器人私信发送的四周墙壁信息**";
            } else {
                reply() << UnitMaps::RandomHint(UnitMaps::wall_hints) << "\n移动时碰撞【墙壁】，本回合结束！请留意机器人私信发送的四周墙壁信息";
            }
            return StageErrCode::READY;
        }

        auto sender = reply();
        
        bool ready_status = HandleGridInteraction(player, sender);
        if (ready_status) return StageErrCode::READY;

        // 继续行动
        if (step == 1) {
            sender << "请继续行动（行动总时限为 " << GAME_OPTION(行动时限) << " 秒）";
            Global().StartTimer(GAME_OPTION(行动时限));
        } else {
            int time = std::chrono::duration_cast<std::chrono::seconds>(*Global().TimerFinishTime() - std::chrono::steady_clock::now()).count();
            sender << "请继续行动（剩余时间 " << time << " 秒）";
        }
        // 单步隐匿：消除隐匿状态
        if (GAME_OPTION(隐匿) == 2 && hide) { hide = false; }
        return StageErrCode::OK;
    }

    // 处理区块效果（返回玩家回合是否结束）
    bool HandleGridInteraction(Player& player, MsgSenderBase::MsgSenderGuard& sender)
    {
        if (player.subspace > 0) return false;  // 亚空间内不影响地图
        
        Grid& grid = Main().board.grid_map[player.x][player.y];
        // [逃生舱]
        if (grid.Type() == GridType::EXIT) {
            player.move_record += "(逃生)";
            int exited = Main().board.exit_num - Main().board.ExitCount();
            player.score.exit_score += Score::exit_order[exited];   // 逃生分
            grid.SetType(GridType::EMPTY);

            if (GAME_OPTION(大乱斗)) {
                player.move_record += "【传送】";
                Main().board.TeleportPlayer(player.pid);  // 大乱斗随机传送
                sender << UnitMaps::RandomHint(UnitMaps::exit_hints) << "\n\n您已抵达【逃生舱】！此逃生舱已失效，" << At(player.pid) << " 被随机传送至地图其他地方！";
            } else {
                player.out = 2;
                sender << UnitMaps::RandomHint(UnitMaps::exit_hints) << "\n\n您已抵达【逃生舱】！不再参与后续游戏，此逃生舱已失效";
                if (Main().Alive_() > 1) {
                    Main().board.UpdatePlayerTarget(GAME_OPTION(捉捕目标));   // 捕捉顺位变更
                    sender << "，剩余玩家捕捉目标顺位发生变更！\n";
                    sender << Markdown(Main().board.GetPlayerTable(Main().round_));
                }
            }
            // 隐匿状态公屏提示
            if (hide) {
                Global().Boardcast() << At(currentPlayer) << " 已抵达【逃生舱】！" << (GAME_OPTION(大乱斗) ? "被随机传送至地图其他地方" : "不再参与后续游戏") << "，此逃生舱已失效\n"
                                     << Markdown(Main().board.GetPlayerTable(Main().round_));
            }
            return true;
        }
        // [传送门]
        if (player.subspace == 0) {
            // 离开亚空间，传送门传送
            Main().board.RemovePlayerFromMap(player.pid);
            grid.PortalTeleport(player);
            Main().board.player_map[player.x][player.y].push_back(player.pid);
        } else if (grid.Type() == GridType::PORTAL) {
            if (player.subspace == -1) {
                // 进入亚空间
                player.subspace = 2;
                // 出口是[单向传送门]则交换
                pair<int, int> pRelPos = Main().board.grid_map[player.x][player.y].PortalPos();
                Grid& target_grid = Main().board.grid_map[player.x + pRelPos.first][player.y + pRelPos.second];
                if (target_grid.Type() == GridType::ONEWAYPORTAL) {
                    grid.SetType(GridType::ONEWAYPORTAL);
                    target_grid.SetType(GridType::PORTAL);
                }
            }
        }
        // [陷阱]
        if (grid.Type() == GridType::TRAP) {
            grid.TrapTrigger();
            if (grid.TrapStatus()) {
                player.move_record += "(陷阱)";
                sender << UnitMaps::RandomHint(UnitMaps::trap_hints) << "\n\n移动触发【陷阱】，本回合被强制停止行动！";
                return true;
            }
        }
        // [热源]
        string step_info, heat_message;
        if (Main().board.HeatNotice(player.pid)) {
            step_info = "[热浪(第" + to_string(step) +  "步)]";
            heat_message = UnitMaps::RandomHint(UnitMaps::heat_wave_hints) + "\n移动进入【热浪范围】，当前位置附近存在热源";
        }
        if (grid.Type() == GridType::HEAT) {
            if (player.heated) {
                player.move_record += "(热源)";
                sender << UnitMaps::RandomHint(UnitMaps::heat_active_hints) << "\n\n您本局游戏已进入过【热源】，高温难耐，本回合无法继续前进！";
                return true;
            } else {
                player.heated = true;
                step_info = "[热源(第" + to_string(step) +  "步)]";
                heat_message = UnitMaps::RandomHint(UnitMaps::heat_core_hints) + "\n移动进入【热源】！请注意，在下一次进入热源时，将公开热源并强制停止行动";
            }
        }
        if (heat_message != "") {
            Global().Tell(player.pid) << step_info << heat_message;
            player.private_record += "\n" + step_info;
        }
        // [按钮]
        if (grid.Type() == GridType::BUTTON) {
            Grid::ButtonTarget target = grid.ButtonTargetPos();
            const int s = Main().board.size;
            // 切换[门]状态
            if (target.dir.has_value()) {
                int tx = player.x + target.dx;
                int ty = player.y + target.dy;
                const Direct dir = target.dir.value();
                Main().board.grid_map[tx][ty].switchDoor(dir);
                int d = static_cast<int>(dir);
                int nx = (tx + k_DX_Direct[d] + s) % s;
                int ny = (ty + k_DY_Direct[d] + s) % s;
                Main().board.grid_map[nx][ny].switchDoor(opposite(dir));
                door_modified = true;
            }
        }
        // 声响 Sound
        Sound sound = Main().board.GetSound(grid, GAME_OPTION(特殊事件) == 3);
        if (sound == Sound::SHASHA) {
            if (hide) {
                sender << "移动进入【树丛】（隐匿中，不会向其他人发出声响）\n\n";
            } else {
                player.move_record += "[沙沙]";
                sender << UnitMaps::RandomHint(UnitMaps::grass_hints) << "\n移动进入【树丛】，请其他玩家留意私信声响信息！\n\n";
                SendSoundMessage(player.x, player.y, sound, false);
            }
        } else if (sound == Sound::PAPA) {
            if (hide) {
                sender << "移动发出【啪啪声】（隐匿中，不会向其他人发出声响）\n\n";
            } else {
                player.move_record += "[啪啪]";
                sender << UnitMaps::RandomHint(UnitMaps::papa_hints) << "\n移动发出【啪啪声】，请其他玩家留意私信声响信息！\n\n";
                SendSoundMessage(player.x, player.y, sound, false);
            }
        }
        // 非点杀模式检测玩家捕捉（隐匿状态不能捕捉）
        if (!GAME_OPTION(点杀)) {
            if (PlayerCatch(player, sender)) {
                return true;
            }
        }
        // 玩家可继续行动
        return false;
    }

    AtomReqErrCode Stop_(const PlayerID pid, const bool is_public, MsgSenderBase& reply)
    {
        Player& player = Main().board.players[pid];
        if (player.out == 2) {
            reply() << "您已乘坐逃生舱撤离，无需继续行动";
            return StageErrCode::FAILED;
        }
        if (pid != currentPlayer) {
            reply() << "[错误] 不是您的回合，当前正在行动玩家是：" << Global().PlayerName(currentPlayer);
            return StageErrCode::FAILED;
        }
        if (!is_public && Global().PlayerNum() > 1) {
            if (GAME_OPTION(隐匿) == 0) {
                reply() << "[错误] 请全程在公屏进行行动";
            } else {
                reply() << "[错误] 您未使用隐匿技能，请在公屏进行行动";
            }
            return StageErrCode::FAILED;
        }
        if (!hide) player.move_record += "(停止)";
        active_stop = true;
        reply() << "您选择主动停止行动，本回合结束！主动停止无法获得四周墙壁信息";
        return StageErrCode::READY;
    }

    AtomReqErrCode Hide_(const PlayerID pid, const bool is_public, MsgSenderBase& reply)
    {
        if (GAME_OPTION(隐匿) == 0) {
            reply() << "[错误] 本局游戏未开启隐匿技能";
            return StageErrCode::FAILED;
        }
        Player& player = Main().board.players[pid];
        if (player.out == 2) {
            reply() << "您已乘坐逃生舱撤离，无需继续行动";
            return StageErrCode::FAILED;
        }
        if (pid != currentPlayer) {
            reply() << "[错误] 不是您的回合，当前正在行动玩家是：" << Global().PlayerName(currentPlayer);
            return StageErrCode::FAILED;
        }
        if (hide) {
            reply() << "[错误] 您已经处于隐匿状态，请在私信选择行动";
            return StageErrCode::FAILED;
        }
        if (player.hide_remaining == 0) {
            reply() << "[错误] 隐匿技能次数已耗尽";
            return StageErrCode::FAILED;
        }
        if (!is_public && Global().PlayerNum() > 1) {
            reply() << "[错误] 请在公屏使用隐匿技能";
            return StageErrCode::FAILED;
        }
        
        hide = true;
        player.hide_remaining--;
        if (GAME_OPTION(隐匿) == 1) {
            player.move_record += "【隐匿行动】";
            reply() << "使用隐匿技能，本回合剩余时间转为私聊行动，不会发出声响，不会触发捕捉。剩余次数：" << player.hide_remaining;
        } else if (GAME_OPTION(隐匿) == 2) {
            player.move_record += "�";
            reply() << "使用隐匿技能，下一步请在私聊行动，不会发出声响，不会触发捕捉。剩余次数：" << player.hide_remaining;
        }
        return StageErrCode::OK;
    }

    AtomReqErrCode Status_(const PlayerID pid, const bool is_public, MsgSenderBase& reply)
    {
        auto sender = reply();
        if (is_public) {
            if (GAME_OPTION(特殊事件) > 0) {
                sender << UnitMaps::ShowSpecialEvent(GAME_OPTION(特殊事件)) << "\n";
            }
            if (GAME_OPTION(边长) > 9) {
                sender << "本局游戏地图为 " << GAME_OPTION(边长) << "x" << GAME_OPTION(边长) << "\n";
            }
            sender << Markdown(Main().board.GetPlayerTable(Main().round_));
            for (PlayerID pid = 0; pid < currentPlayer.Get(); ++pid) {
                if (Main().board.players[pid].out == 0) {
                    sender << "\n[" << pid.Get() << "号]本回合行动轨迹：\n" << Main().board.players[pid].move_record;
                }
            }
            sender << "\n[" << currentPlayer.Get() << "号]正在行动中：\n" << Main().board.players[currentPlayer].move_record;
        } else {
            sender << Main().board.players[pid].private_record;
        }
        return StageErrCode::OK;
    }

    AtomReqErrCode AllStatus_(const PlayerID pid, const bool is_public, MsgSenderBase& reply)
    {
        auto sender = reply();
        if (GAME_OPTION(特殊事件) > 0) {
            sender << UnitMaps::ShowSpecialEvent(GAME_OPTION(特殊事件)) << "\n";
        }
        if (GAME_OPTION(边长) > 9) {
            sender << "本局游戏地图为 " << GAME_OPTION(边长) << "x" << GAME_OPTION(边长) << "\n";
        }
        sender << Markdown(Main().board.GetAllRecord(), 500);
        sender << "\n[" << currentPlayer.Get() << "号]正在行动中：\n" << Main().board.players[currentPlayer].move_record;
        if (!is_public) {
            sender << "\n\n" << Main().board.players[pid].private_record;
        }
        return StageErrCode::OK;
    }

    bool PlayerCatch(Player& player, MsgSenderBase::MsgSenderGuard& sender)
    {
        vector<PlayerID> list = Main().board.player_map[player.x][player.y];
        PlayerID t = player.target;
        if (find(list.begin(), list.end(), t) != list.end() && !hide) {
            active_stop = false;
            sender << UnitMaps::RandomHint(UnitMaps::catch_hints) << "\n\n";
            sender << At(t) << " 被捕捉！";

            if (Main().round_ == 1) {
                player.move_record += "(首轮捕捉)【传送】";
                Main().board.players[t].all_record += (Main().board.players[t].all_record == "" ? "<br>" : "") + string("【首轮被抓传送】");
                Main().board.TeleportPlayer(t);  // 随机传送被捉方
                Main().board.TeleportPlayer(player.pid);  // 随机传送捕捉方
                sender << "\n【首轮玩家保护】\n首轮捕捉不生效：双方均被随机传送至地图其他地方！";
                return true;
            }

            player.move_record += "(捕捉)";
            Main().board.players[t].out = 1;
            Global().Eliminate(t);
            player.score.catch_score += 100;        // 抓人分
            Main().board.players[t].score.catch_score -= 100;

            if (Main().Alive_() > 1) {
                player.move_record += "【传送】";
                Main().board.TeleportPlayer(player.pid);  // 随机传送捕捉方
                Main().board.UpdatePlayerTarget(GAME_OPTION(捉捕目标));   // 捕捉顺位变更
                sender << "\n" << At(player.pid) << " 被随机传送至地图其他地方，捕捉目标顺位发生变更！\n";
                sender << Markdown(Main().board.GetPlayerTable(Main().round_));
            }
            
            if (Main().Alive_() == 1) {
                // 无逃生舱最后生还判定
                if (Main().board.ExitCount() == 0) Main().withoutE_win_ = true;
                // 有逃生舱但死斗取胜判定
                if (Main().board.ExitCount() > 0) Main().withE_win_ = true;
            }
            return true;
        }
        return false;
    }

    // 私信其他玩家发送声响信息
    void SendSoundMessage(const int fromX, const int fromY, const Sound sound, const bool to_all)
    {
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            if ((pid != currentPlayer || to_all) && Main().board.players[pid].out == 0) {
                string direction = Main().board.GetSoundDirection(fromX, fromY, Main().board.players[pid]);
                string step_info = "[" + to_string(currentPlayer) + "号(第" + to_string(step) +  "步)]";
                string sound_message;
                if (direction == "") {
                    if (Main().board.players[currentPlayer].target != pid || GAME_OPTION(点杀)) {
                        switch (sound) {
                            case Sound::SHASHA: sound_message = step_info + "\n" + UnitMaps::RandomHint(UnitMaps::grass_sound_hints); break;
                            case Sound::PAPA:   sound_message = step_info + "\n" + UnitMaps::RandomHint(UnitMaps::papa_sound_hints); break;
                            default: sound_message = "[错误] 未知声音类型：相同格子的未知声音";
                        }
                    }
                } else {
                    switch (sound) {
                        case Sound::SHASHA: sound_message = step_info + "你听见了来自【" + direction + "方】的沙沙声！"; break;
                        case Sound::PAPA:   sound_message = step_info + "你听见了来自【" + direction + "方】的啪啪声！"; break;
                        case Sound::BOSS:   sound_message = "[BOSS] 你听见了来自【" + direction + "方】的巨大响声！"; break;
                        default: sound_message = "[错误] 未知声音类型：不同格子来自【" + direction + "方】的未知声音";
                    }
                }
                Main().board.players[pid].private_record += "\n" + sound_message;
                Global().Tell(pid) << sound_message;
            }
        }
    }

    virtual CheckoutErrCode OnStageTimeout() override
    {
        Main().board.players[currentPlayer].move_record += "(超时)";
        active_stop = true;
        if (step == 0) {
            Main().board.players[currentPlayer].hook_status = true;
            Global().Boardcast() << "玩家 " << At(PlayerID(currentPlayer)) << " 超时未行动，已进入挂机状态，再次行动前仅有30秒等待时间";
        } else {
            Global().Boardcast() << "玩家 " << At(PlayerID(currentPlayer)) << " 行动超时，切换下一个玩家";
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
            Main().board.UpdatePlayerTarget(GAME_OPTION(捉捕目标));   // 捕捉顺位变更
            sender << "，捕捉目标顺位发生变更！\n";
            sender << Markdown(Main().board.GetPlayerTable(Main().round_));
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
        // 点杀模式：回合结束才触发捉捕
        if (GAME_OPTION(点杀)) {
            vector<PlayerID> list = Main().board.player_map[player.x][player.y];
            if (find(list.begin(), list.end(), player.target) != list.end() && !hide) {
                auto sender = Global().Boardcast();
                PlayerCatch(player, sender);
            }
        }
        Global().Boardcast() << "[" << currentPlayer.Get() << "号]玩家本回合的完整行动轨迹：\n" << player.move_record;
        // 记录历史行动轨迹
        player.all_record += "<br>【第 " + to_string(Main().round_) + " 回合】<br>" + player.move_record;
        // 仅剩1玩家，游戏结束
        if ((Main().Alive_() == 1 && Global().PlayerNum() > 1) || (Main().Alive_() == 0 && Global().PlayerNum() == 1)) {
            if (Main().withoutE_win_) {     // 无逃生舱最后生还胜利
                Global().Boardcast() << At(currentPlayer) << "\n" << UnitMaps::RandomHint(UnitMaps::withoutE_win_hints);
            }
            if (Main().withE_win_) {        // 有逃生舱但死斗取胜
                Global().Boardcast() << At(currentPlayer) << "\n" << UnitMaps::RandomHint(UnitMaps::withE_win_hints);
            }
            return StageErrCode::CHECKOUT;
        }
        // 私信发送四周墙壁信息（主动停止或超时不发送）
        if (player.out == 0 && !active_stop) {
            auto [info, md] = Main().board.GetSurroundingWalls(currentPlayer);
            player.private_record = "【第 " + to_string(Main().round_) + " 回合】\n您所在位置的四周墙壁信息，按照 上下左右 顺序分别是：\n" + info;
            Global().Tell(currentPlayer) << player.private_record << "\n" << Markdown(md, 110);
        }
        step = 0;
        hide = false;
        active_stop = false;
        // 下一个玩家行动
        do {
            currentPlayer = (currentPlayer + 1) % Global().PlayerNum();
            if (currentPlayer == 0) break;
        } while (Main().board.players[currentPlayer].out > 0);
        if (currentPlayer != 0) {
            uint32_t think_time = Main().board.players[currentPlayer].hook_status ? 30 : GAME_OPTION(思考时限);
            Global().Boardcast() << "请 " << At(currentPlayer) << " 在公屏选择方向移动（第一次行动前有 " << think_time << " 秒思考时间，行动开始后总时限为 " << GAME_OPTION(行动时限) << " 秒）";
            Global().ClearReady(currentPlayer);
            Global().StartTimer(think_time);
            return StageErrCode::CONTINUE;
        }

        // [回合结束] 所有玩家都行动后结束本回合
        // BOSS相关结算
        if (GAME_OPTION(BOSS)) {
            string boss_record = "<br>【第 " + to_string(Main().round_) + " 回合】";
            auto& boss = Main().board.boss;
            auto sender = Global().Boardcast();
            sender << "【回合结束[BOSS行动]】";

            if (Main().board.BossChangeTarget(false)) {
                // 未更换目标，执行移动
                bool is_catch = Main().board.BossMove();
                // 抓住玩家
                if (is_catch) {
                    for (auto pid: Main().board.player_map[boss.x][boss.y]) {
                        if (Main().board.players[pid].out > 0) continue;
                        Main().board.players[pid].all_record += "【BOSS捕捉】";
                        Main().board.players[pid].out = 1;
                        if (Global().PlayerNum() > 1) Global().Eliminate(pid);
                        Main().board.players[pid].score.catch_score -= 100;        // 抓人分
                        boss_record += "[" + to_string(pid) + "号] ";
                        sender << "\n" << At(pid);
                    }
                    boss_record += "被BOSS捕捉出局！";
                    sender << "\n被BOSS捕捉出局！";
                    if (Main().Alive_() > 1) {
                        Main().board.BossChangeTarget(true);    // 重置锁定目标
                        Main().board.UpdatePlayerTarget(GAME_OPTION(捉捕目标));   // 捕捉顺位变更
                        boss_record += "变更目标至 [" + to_string(boss.target) + "号]";
                        sender << "\n\nBOSS更换锁定目标至 " << At(boss.target) << "，同时玩家捕捉目标顺位发生变更！\n";
                        sender << Markdown(Main().board.GetPlayerTable(Main().round_));
                    }
                } else {
                    boss_record += "向 [" + to_string(boss.target) + "号] 移动了 " + to_string(boss.steps) + " 步";
                    sender << "【回合结束】\nBOSS向 " << At(boss.target) << " 移动了 " << boss.steps << " 步";
                }
                // BOSS移动后发出巨响
                if (Main().Alive_() > 1 || (Global().PlayerNum() == 1 && Main().board.players[0].out == 0)) {
                    boss_record += "（巨响）";
                    sender << "\n\nBOSS发出震耳欲聋的巨响！请所有玩家留意私信声响信息！";
                    SendSoundMessage(boss.x, boss.y, Sound::BOSS, true);
                }
            } else {
                // 更换目标，重置步数
                boss_record += "发现更近的目标，变更目标至 [" + to_string(boss.target) + "号]";
                sender << "\nBOSS发现了距离更近的玩家，变更锁定目标至 " << At(boss.target);
            }

            // BOSS周围8格内获得喘息提示
            for (auto& player : Main().board.players) {
                if (Main().board.IsBossNearby(player) && player.out == 0) {
                    player.private_record += "\n[BOSS] 你听到来自BOSS沉重的喘息声！";
                    Global().Tell(player.pid) << "「呼……呼……」你听到来自BOSS沉重的喘息声！";
                }
            }
            boss.all_record += boss_record;
        }
        // 门变更过进行提示
        if (door_modified) {
            Global().Boardcast() << "【注意】在本回合内，有门的状态发生过变化，但存在恢复原状的可能";
            door_modified = false;
        }

        return StageErrCode::CHECKOUT;
    }
    
    virtual AtomReqErrCode OnComputerAct(const PlayerID pid, MsgSenderBase& reply) override
    {
        if (Global().IsReady(pid)) {
            return StageErrCode::OK;
        }
        Main().board.players[pid].out = 1;
        Main().board.players[pid].score.quit_score -= 300;  // 退出分

        auto sender = Global().Boardcast();
        sender << "笨笨的机器人退出了游戏";
        if (Main().Alive_() > 1) {
            Main().board.UpdatePlayerTarget(GAME_OPTION(捉捕目标));   // 捕捉顺位变更
            sender << "，捕捉目标顺位发生变更！\n";
            sender << Markdown(Main().board.GetPlayerTable(Main().round_));
        }
        return StageErrCode::READY;
    }
};

auto* MakeMainStage(MainStageFactory factory) { return factory.Create<MainStage>(); }

} // namespace GAME_MODULE_NAME

} // namespace game

} // namespace lgtbot

