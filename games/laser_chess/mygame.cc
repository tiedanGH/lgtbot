// Copyright (c) 2018-present, Chang Liu <github.com/slontia>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).

#include <array>
#include <map>
#include <functional>
#include <memory>
#include <vector>
#include <random>

#include "game_framework/stage.h"
#include "game_framework/util.h"
#include "utility/html.h"
#include "utility/coding.h"
#include "game_util/laser_chess.h"

#include "game_maps.h"

using namespace lgtbot::game_util::laser_chess;

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

class MainStage;
template <typename... SubStages> using SubGameStage = StageFsm<MainStage, SubStages...>;
template <typename... SubStages> using MainGameStage = StageFsm<void, SubStages...>;
const GameProperties k_properties { 
    .name_ = "镭射象棋",
    .developer_ = "森高",
    .description_ = "调整镜面，发射激光消灭对方棋子的游戏",
};
uint64_t MaxPlayerNum(const MyGameOptions& options) { return 2; } /* 0 means no max-player limits */
uint32_t Multiple(const MyGameOptions& options) { return std::min(3U, GET_OPTION_VALUE(options, 回合数) / 10); }
const MutableGenericOptions k_default_generic_options;
const std::vector<RuleCommand> k_rule_commands = {};

bool AdaptOptions(MsgSenderBase& reply, MyGameOptions& game_options, const GenericOptions& generic_options_readonly, MutableGenericOptions& generic_options)
{
    if (generic_options_readonly.PlayerNum() != 2) {
        reply() << "该游戏为双人游戏，必须为2人参加，当前玩家数为" << generic_options_readonly.PlayerNum();
        return false;
    }
    return true;
}

const std::vector<InitOptionsCommand> k_init_options_commands = {
    InitOptionsCommand("独自一人开始游戏",
            [] (MyGameOptions& game_options, MutableGenericOptions& generic_options)
            {
                generic_options.bench_computers_to_player_num_ = 2;
                return NewGameMode::SINGLE_USER;
            },
            VoidChecker("单机")),
};

// ========== GAME STAGES ==========

enum class Choise { UP, RIGHT, DOWN, LEFT, RIGHT_UP, LEFT_UP, RIGHT_DOWN, LEFT_DOWN, CLOCKWISE, ANTICLOCKWISE, _MAX };

static std::ostream& operator<<(std::ostream& os, const Coor& coor) { return os << ('A' + coor.m_) << coor.n_; }

// function order should be same as enum order
std::array<Board(*)(std::string), GameMap::Count() - 1> game_map_initers =
    {InitAceBoard, InitCuriosityBoard, InitGrailBoard, InitMercuryBoard, InitSophieBoard, InitGeniusBoard, InitRefractionBoard, InitGeminiBoard, InitDaisukeBoard};

class MainStage : public MainGameStage<>
{
  public:
    MainStage(StageUtility&& utility)
        : StageFsm(std::move(utility),
                MakeStageCommand(*this, "查看盘面情况，可用于图片重发", &MainStage::Info_, VoidChecker("赛况")),
                MakeStageCommand(*this, "跳过行动", &MainStage::Pass_, VoidChecker("pass")),
                MakeStageCommand(*this, "移动棋子", &MainStage::Set_,
                    AnyArg("棋子当前位置", "B5"), AlterChecker<Choise>(
                            {{ "上", Choise::UP },
                            { "右", Choise::RIGHT },
                            { "下", Choise::DOWN },
                            { "左", Choise::LEFT },
                            { "右上", Choise::RIGHT_UP },
                            { "左上", Choise::LEFT_UP },
                            { "右下", Choise::RIGHT_DOWN },
                            { "左下", Choise::LEFT_DOWN },
                            { "顺", Choise::CLOCKWISE },
                            { "逆", Choise::ANTICLOCKWISE }}
                        )))
        , map_(GAME_OPTION(地图) == GameMap::随机 ? GameMap::Members()[rand() % (GameMap::Count() - 1)] : GAME_OPTION(地图))
        , board_(game_map_initers[map_.ToUInt()](Global().ResourceDir()))
        , round_(0)
        , scores_{0}
    {
    }

    virtual void OnStageBegin()
    {
        Global().StartTimer(GAME_OPTION(局时));
        board_html_ = board_.ToHtml();
        Global().Boardcast() << Markdown(ShowInfo_());
        Global().Boardcast() << "请双方行动，" << GAME_OPTION(局时)
                    << "秒未行动自动 pass\n格式：棋子位置 行动方式";
    }

    virtual AtomReqErrCode OnComputerAct(const PlayerID pid, MsgSenderBase& reply)
    {
        if (Global().IsReady(pid)) {
            return StageErrCode::OK;
        }
        const Coor coor = [&]() -> Coor
            {
                while (true) {
                    const Coor coor(rand() % board_.max_m(), rand() % board_.max_n());
                    if (Act_(pid, coor, static_cast<Choise>(rand() % static_cast<uint32_t>(Choise::_MAX)), EmptyMsgSender::Get())) {
                        return coor;
                    }
                }
                return {};
            }();
        return StageErrCode::READY;
    }

    int64_t PlayerScore(const PlayerID pid) const
    {
        return scores_[pid];
    }

  private:
    bool Act_(const PlayerID pid, const Coor& coor, const Choise choise, MsgSenderBase& reply)
    {
        const auto coor_to_str = [](const Coor& coor) { return char('A' + coor.m_) + std::to_string(coor.n_); };
        if (choise == Choise::CLOCKWISE || choise == Choise::ANTICLOCKWISE) {
            if (const auto errmsg = board_.Rotate(coor, choise == Choise::CLOCKWISE, pid); !errmsg.empty()) {
                reply() << "行动失败：" << errmsg;
                return false;
            }
            reply() << "您将 " << coor_to_str(coor) << " 位置的棋子"
                << (choise == Choise::CLOCKWISE ? "顺" : "逆") << "时针旋转";
        } else {
            const auto new_coor = coor + (
                        choise == Choise::UP ? Coor{-1, 0} :
                        choise == Choise::DOWN ? Coor{1, 0} :
                        choise == Choise::LEFT ? Coor{0, -1} :
                        choise == Choise::RIGHT ? Coor{0, 1} :
                        choise == Choise::LEFT_UP ? Coor{-1, -1} :
                        choise == Choise::RIGHT_UP ? Coor{-1, 1} :
                        choise == Choise::LEFT_DOWN ? Coor{1, -1} :
                        choise == Choise::RIGHT_DOWN ? Coor{1, 1} : (assert(false), Coor())
                    );
            if (const auto errmsg = board_.Move(coor, new_coor, pid); !errmsg.empty()) {
                reply() << "行动失败：" << errmsg;
                return false;
            }
            reply() << "您将 " << coor_to_str(coor) << " 位置的棋子移动至 " << coor_to_str(new_coor) << " 位置";
        }
        return true;
    }

    AtomReqErrCode Pass_(const PlayerID pid, const bool is_public, MsgSenderBase& reply)
    {
        if (is_public) {
            reply() << "行动失败：请私信裁判行动";
            return StageErrCode::FAILED;
        }
        if (Global().IsReady(pid)) {
            reply() << "行动失败：您已经行动完成";
            return StageErrCode::FAILED;
        }
        reply() << "您选择跳过本回合行动";
        return StageErrCode::READY;
    }

    AtomReqErrCode Set_(const PlayerID pid, const bool is_public, MsgSenderBase& reply, const std::string& coor_str, const Choise choise)
    {
        if (is_public) {
            reply() << "行动失败：请私信裁判行动";
            return StageErrCode::FAILED;
        }
        if (Global().IsReady(pid)) {
            reply() << "行动失败：您已经行动完成";
            return StageErrCode::FAILED;
        }
        const auto coor_ret = DecodePos(coor_str);
        if (const auto* const s = std::get_if<std::string>(&coor_ret)) {
            reply() << "行动失败：" << *s;
            return StageErrCode::FAILED;
        }
        const auto& coor_pair = std::get<std::pair<uint32_t, uint32_t>>(coor_ret);
        if (!Act_(pid, Coor{static_cast<int32_t>(coor_pair.first), static_cast<int32_t>(coor_pair.second)}, choise, reply)) {
            return StageErrCode::FAILED;
        }
        return StageErrCode::READY;
    }

    AtomReqErrCode Info_(const PlayerID pid, const bool is_public, MsgSenderBase& reply)
    {
        reply() << Markdown(ShowInfo_());
        return StageErrCode::OK;
    }

    std::string ShowInfo_() const
    {
        std::string str = "## 第 " + std::to_string(round_) + " / " + std::to_string(GAME_OPTION(回合数)) + " 回合\n\n";
        html::Table player_table(2, 4);
        player_table.MergeDown(0, 0, 2);
        player_table.MergeDown(0, 2, 2);
        player_table.SetTableStyle(" align=\"center\" cellpadding=\"1\" cellspacing=\"1\" ");
        const auto print_player = [&](const PlayerID pid)
            {
                player_table.Get(0, 0 + pid * 2).SetContent(
                        std::string("![](file:///") + Global().ResourceDir() + std::string("king_") + bool_to_rb(pid) + ".png)");
                player_table.Get(0, 1 + pid * 2).SetContent("**" + Global().PlayerName(pid) + "**");
                player_table.Get(1, 1 + pid * 2).SetContent("棋子数量： " HTML_COLOR_FONT_HEADER(blue) + std::to_string(board_.ChessCount(pid)) + HTML_FONT_TAIL);
            };
        print_player(0);
        print_player(1);
        str += player_table.ToString();
        str += "\n\n";
        str += "<center>\n\n **地图：** " HTML_COLOR_FONT_HEADER(red) " **";
        str += map_.ToString();
        str += "** " HTML_FONT_TAIL " \n\n</center>\n\n";
        str += board_html_;
        return str;
    }

    virtual CheckoutErrCode OnStageOver()
    {
        return CheckoutErrCode::Condition(ShootAndSettle_(), StageErrCode::CHECKOUT, StageErrCode::CONTINUE);
    }

    bool ShootAndSettle_()
    {
        bool finish = true;
        ++round_;
        Global().HookUnreadyPlayers();

        const auto settle_ret = board_.Settle();
        board_html_ = settle_ret.html_;

        Global().Boardcast() << Markdown(ShowInfo_());
        if (settle_ret.king_alive_num_[0] == 0 && settle_ret.king_alive_num_[1] == 0) {
            Global().Boardcast() << "双方王同归于尽，根据剩余棋子数量计算胜负";
            scores_[0] = board_.ChessCount(0);
            scores_[1] = board_.ChessCount(1);
        } else if (settle_ret.king_alive_num_[0] == 0) {
            Global().Boardcast() << "玩家" << At(PlayerID{0}) << "的王被命中，输掉了比赛";
            scores_[1] = 1;
        } else if (settle_ret.king_alive_num_[1] == 0) {
            Global().Boardcast() << "玩家" << At(PlayerID{1}) << "的王被命中，输掉了比赛";
            scores_[0] = 1;
        } else if (round_ >= GAME_OPTION(回合数)) {
            Global().Boardcast() << "达到最大回合数，根据剩余棋子数量计算胜负";
            scores_[0] = board_.ChessCount(0);
            scores_[1] = board_.ChessCount(1);
        } else {
            auto sender = Global().Boardcast();
            if (settle_ret.crashed_) {
                sender << "双方移动的棋子因碰撞而湮灭\n\n";
            }
            if (settle_ret.chess_dead_num_[0]) {
                sender << "玩家" << At(PlayerID{0}) << "被命中了 " << settle_ret.chess_dead_num_[0] << " 枚棋子\n\n";
            }
            if (settle_ret.chess_dead_num_[1]) {
                sender << "玩家" << At(PlayerID{1}) << "被命中了 " << settle_ret.chess_dead_num_[1] << " 枚棋子\n\n";
            }
            finish = false;
            Global().ClearReady();
            Global().StartTimer(GAME_OPTION(局时));
            sender << "请双方行动，" << GAME_OPTION(局时) << "秒未行动默认 pass\n格式：棋子位置 行动方式";
        }
        return finish;
    }

    CheckoutErrCode OnStageTimeout()
    {
        return CheckoutErrCode::Condition(ShootAndSettle_(), StageErrCode::CHECKOUT, StageErrCode::CONTINUE);
    }

    GameMap map_;
    Board board_;
    uint32_t round_;
    std::array<int64_t, 2> scores_;
    std::string board_html_;
};

auto* MakeMainStage(MainStageFactory factory) { return factory.Create<MainStage>(); }

} // namespace GAME_MODULE_NAME

} // namespace game

} // gamespace lgtbot

