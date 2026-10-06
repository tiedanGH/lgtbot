// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).
//
// RoundStage 类外成员实现

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

// 执行一步移动：输出移动结果并结算格子效果（返回本回合是否结束）
bool RoundStage::MoveOneStep(Player& player, const Direct direction, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender, const bool multiple_mode)
{
    const bool success = Main().board.MakeMove(player.pid, direction, hide, last_direct);
    step++;
    // 撞墙直接结束本回合
    if (!success) {
        sender << "[第 " << step << " 步] 尝试向 " << dir_cn[static_cast<int>(direction)] << " 移动\n";
        if (step == 1) {
            sender << GetRandomHint(firststep_wall_hints) << "\n移动时碰撞【墙壁】，本回合结束！**请留意机器人私信发送的四周墙壁信息**";
        } else {
            sender << GetRandomHint(wall_hints) << "\n移动时碰撞【墙壁】，本回合结束！请留意机器人私信发送的四周墙壁信息";
        }
        return true;
    }
    turn_record.moves++;    // 成功移动的步数（[暴君]铁律、全员未移动停止判定）
    sender << "[第 " << step << " 步] 向 " << dir_cn[static_cast<int>(direction)] << " 移动";
    return HandleGridInteraction(player, sender, multiple_mode);
}

// 捕捉顺位变更：按配置刷新全员捕捉目标，并附上最新玩家表
void RoundStage::RefreshCatchOrder(lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
{
    Main().board.UpdatePlayerTarget(GAME_OPTION(捕捉目标));
    sender << Markdown(Main().board.GetPlayerTable(Main().round_));
}

// [巨大的心脏] 心跳节拍（返回本步是否发出砰砰声）
// 第 1/4/7/10/13/16/19 步发出全图无方向心跳，共 7 次；不计入 trigger_sound
bool RoundStage::HandleHeartBeat(Player& player, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
{
    if (GAME_OPTION(静音)) return false;    // [静音] 心脏同样被静音（屏蔽器范围内也不再提示心跳）
    if (!Main().board.has_heart || step % 3 != 1 || step > 19) return false;

    const string beat_text = "❤️" PENGPENG_STR "——巨大的心跳声响彻了整个迷宫！";
    if (hide) {
        Global().Tell(player.pid) << "[第 " << step << " 步] " << beat_text << "（隐匿中，心跳不会在公屏显示）";
    } else {
        // 所有心跳步的箭头一律标红：无论本步是否掩盖了声响
        player.MarkHeartMasked();
        sender << "\n" << beat_text;
    }
    // 屏蔽器范围内的玩家：每回合仅第一声心跳私信提醒
    for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
        if (Main().board.players[pid].out == 0 && !heart_jammer_notified[pid] && Main().board.IsNearJammer(pid)) {
            heart_jammer_notified[pid] = true;
            Global().Tell(pid) << GetRandomHint(heart_jammer_hints) << "\n（本回合只提醒第一声" PENGPENG_STR "，后续省略）";
        }
    }
    return true;
}

// 处理区块效果（返回玩家回合是否结束）
bool RoundStage::HandleGridInteraction(Player& player, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender, const bool multiple_mode)
{
    const string prefix = "\n";

    // [巨大的心脏] 心跳节拍 * 先于亚空间判定：亚空间内的步数也计入节拍 *
    const bool heart_beat = HandleHeartBeat(player, sender);

    // [炸弹爆炸] * 先于亚空间判定：踩中传送门入口的炸弹后，亚空间内的移动同样引爆 *
    if (player.bomb_trigger) {
        // 炸弹爆炸（未实际移动，玩家仍在炸弹所在格）
        player.UpdateEndRecord("炸飞出局");
        player.out = 1;
        player.score.catch_score -= 100;
        Main().board.grid_map[player.x][player.y].SetAttach(AttachType::EMPTY);
        sender << prefix << GetRandomHint(bomb_hints) << "\n\n尝试移动时触发【炸弹】，被炸飞出局，失去行动能力！";
        Main().EliminatePlayer(player.pid);
        if (Main().Alive_() > 1) {
            sender << "\n\n剩余玩家捕捉目标顺位发生变更！\n";
            RefreshCatchOrder(sender);  // 捕捉顺位变更
        }
        return true;
    }

    // [亚空间] * 优先级仅次于心脏、炸弹 *
    Grid& former_grid = Main().board.grid_map[player.x][player.y];
    // 亚空间内不影响地图
    if (player.InSubspace()) return false;
    // 离开亚空间，传送门传送
    bool this_move_teleport = false;
    if (player.subspace == 0) {
        Main().board.RemovePlayerFromMap(player.pid);
        former_grid.PortalTeleport(player);
        this_move_teleport = true;
        Main().board.player_map[player.x][player.y].push_back(player.pid);
    }

    // 玩家当前所在格子
    Grid& grid = Main().board.grid_map[player.x][player.y];

    // 成就[乒铃乓啷]辅助
    player.achievement.visitGrid(grid.Type());
    player.achievement.visitAttach(grid.Attach());
    /* ========== AttachType ========== */
    // [按钮]
    if (grid.Attach() == AttachType::BUTTON) {
        Main().board.PressButton(player.x, player.y);
        door_modified++;
    }
    // [炸弹触发] 下一步尝试离开时引爆，在炸弹上结束行动则拆除
    if (grid.Attach() == AttachType::BOMB) {
        player.bomb_trigger = true;
    }
    // [金币] 静默拾取：本人也不会收到提示，回合结束统一公示
    if (grid.Attach() == AttachType::COIN) {
        grid.SetAttach(AttachType::EMPTY);
        player.coins++;
        player.coins_this_round++;
        player.score.coin_score += Score::COIN_SCORE;
    }
    /* ========== GridType ========== */
    // [逃生舱]
    if (grid.Type() == GridType::EXIT) {
        turn_record.escaped = true;     // [暴君]铁律：乘坐逃生舱（含大乱斗）
        player.NewContentRecord("(逃生)", "escape");
        int exited = Main().board.exit_num - Main().board.TypeCount(GridType::EXIT);
        player.score.exit_score += Score::exit_order[exited];   // 逃生分
        grid.SetType(GridType::EMPTY);
        if (!player.achievement.trigger_sound) player.achievement.exit_without_sound = true;    // 成就【悄无声息】
        if (Main().round_ == 1) player.achievement.exit_first_round = true; // 成就【我赶时间】

        if (GAME_OPTION(大乱斗)) {
            player.NewContentRecord("[传送]", "teleport");
            Main().board.TeleportPlayer(player.pid);  // 大乱斗随机传送
            sender << prefix << GetRandomHint(exit_hints) << "\n\n您已抵达【逃生舱】！此逃生舱已失效，" << At(player.pid) << " 被随机传送至地图其他地方！";
        } else {
            player.out = 2;
            sender << prefix << GetRandomHint(exit_hints) << "\n\n您已抵达【逃生舱】！不再参与后续游戏，此逃生舱已失效";
            if (Main().Alive_() > 1) {
                sender << "，剩余玩家捕捉目标顺位发生变更！\n";
                RefreshCatchOrder(sender);  // 捕捉顺位变更
            }
        }
        // 隐匿状态公屏提示
        if (hide) {
            Global().Boardcast() << At(currentPlayer) << " 已抵达【逃生舱】！" << (GAME_OPTION(大乱斗) ? "被随机传送至地图其他地方" : "不再参与后续游戏") << "，此逃生舱已失效\n"
                                 << Markdown(Main().board.GetPlayerTable(Main().round_));
        }
        return true;
    }
    // [传送门] 仅本回合未传送时触发
    if (grid.Type() == GridType::PORTAL && !this_move_teleport) {
        // 进入亚空间
        player.subspace = 2;
        // 出口是[单向传送门]则交换
        Main().board.SwapOneWayPortal(player.x, player.y);
    }
    // [陷阱]
    if (grid.Type() == GridType::TRAP) {
        grid.TrapTrigger();
        if (grid.TrapStatus()) {
            step++; // 成就【守株待兔】：强制停止计入额外步数
            player.NewContentRecord("(陷阱)");
            sender << prefix << GetRandomHint(trap_hints) << "\n\n移动触发【陷阱】，本回合被强制停止行动！";
            return true;
        }
    }
    /* ========== Sound ========== */
    Sound sound = Main().board.GetSound(grid, GAME_OPTION(特殊事件));
    // [静音] 一切归于死寂：不产生任何声响提示，但踩入声响地形仍计入[无声]相关成就
    if (GAME_OPTION(静音) && sound != Sound::NONE) {
        if (!hide) player.achievement.trigger_sound = true;
        sound = Sound::NONE;
    }
    if (sound == Sound::SHASHA || sound == Sound::PAPA) {
        if (!hide) turn_record.made_sound = true;   // [暴君]铁律：发出声响（含首次发声、被心跳掩盖）
        const bool is_shasha = (sound == Sound::SHASHA);
        const char* terrain_label = is_shasha ? "【树丛】" : "【" PAPA_STR "声】";
        const char* action_verb   = is_shasha ? "移动进入" : "移动发出";

        if (hide) {
            sender << prefix << action_verb << terrain_label << "（隐匿中，不会向其他人发出声响）";
        } else if (heart_beat) {
            // [巨大的心脏] 心跳完全掩盖本步声响：公屏文本无痕迹，仅私信本人掩盖详情
            Global().Tell(player.pid) << "[第 " << step << " 步] " << action_verb << terrain_label
                                      << "，声响被巨大的心跳声掩盖，其他玩家未能察觉";
            player.UpdateExtraPriContent(string("掩盖[") + (is_shasha ? SHASHA_STR : PAPA_STR) + "]", "masked");
            player.achievement.trigger_sound = true;
        } else {
            const bool is_first_sound = !player.achievement.trigger_sound;
            player.UpdateSoundRecord(sound);
            sender << prefix
                   << GetRandomHint(is_shasha ? std::span<const std::string_view>(grass_hints)
                                              : std::span<const std::string_view>(papa_hints))
                   << "\n" << action_verb << terrain_label;
            if (is_first_sound) {
                sender << "！本局首次发声，不会向其他人发出声响";
            } else {
                sender << "，请其他玩家留意私信声响信息！";
            }
            SendSoundMessage(player.x, player.y, sound, false, is_first_sound);
            player.achievement.trigger_sound = true;
        }
        // [浆果丛] 私信告知踩入玩家
        if (grid.Type() == GridType::BERRY) {
            Global().Tell(player.pid) << GetRandomHint(berry_hints) << "\n"
                                      << Image(Main().board.GetGridImagePath(GridType::BERRY));
            if (!hide) {
                if (heart_beat) player.NewExtraPriContent("浆果丛", "berry");   // 掩盖步：浆果丛独立显示
                else player.UpdateExtraPriContent("浆果丛", "berry");
            }
        }
    }
    // [热源]
    string step_info, heat_message;
    bool heat_core_entered = false;     // 首次进入热源
    if (Main().board.HeatWaveNotice(player.pid)) {
        step_info = "[热浪(第" + to_string(step) + "步)]";
        heat_message = GetRandomHint(heat_wave_hints) + "\n移动进入【热浪范围】，当前位置附近存在热源";
        player.NewExtraPriContent("热浪",  "heat-wave");
    }
    if (grid.Type() == GridType::HEAT) {
        if (player.heated) {
            step++; // 成就【守株待兔】：强制停止计入额外步数
            player.NewContentRecord("(热源)");
            sender << prefix << GetRandomHint(heat_active_hints) << "\n\n您本局游戏已进入过【热源】，高温难耐，本回合无法继续前进！";
            return true;
        } else {
            player.heated = true;
            step_info = "[热源(第" + to_string(step) + "步)]";
            heat_message = GetRandomHint(heat_core_hints) + "\n移动进入【热源】！请注意，在下一次进入热源时，将公开热源并强制停止行动";
            player.UpdateExtraPriContent("热源", "heat-core");
            heat_core_entered = true;
        }
    }
    if (heat_message != "") {
        auto tell = Global().Tell(player.pid);
        tell << step_info << heat_message;
        if (heat_core_entered) tell << "\n" << Image(Main().board.GetGridImagePath(GridType::HEAT));
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

// 捕捉对象：同格的捕捉目标优先，其次为同格被通缉的玩家（[暴君]通缉：可被所有玩家捕捉）；已出局的玩家不会被捕捉
std::optional<PlayerID> RoundStage::FindCatchVictim(const Player& player)
{
    const vector<PlayerID>& list = Main().board.player_map[player.x][player.y];
    if (find(list.begin(), list.end(), player.target) != list.end() && Main().board.players[player.target].out == 0) return player.target;
    for (const PlayerID pid : list) {
        const Player& other = Main().board.players[pid];
        if (pid == player.pid || other.out != 0) continue;
        for (const auto& boss : Main().board.bosses) {
            if (boss->IsWanted(other)) return pid;
        }
    }
    return std::nullopt;
}

// 捕捉：坐标重合，玩家没有隐匿且未出局
bool RoundStage::PlayerCatch(Player& player, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
{
    const std::optional<PlayerID> victim = FindCatchVictim(player);
    if (!victim.has_value() || hide || player.out != 0) {
        return false;
    }
    const PlayerID t = *victim;
    turn_record.caught_player = true;   // [暴君]铁律：捕捉过玩家

    active_stop = false;    // 捕捉成功不视为主动停止
    forced_stop = false;    // [谋定后动] 捕捉方照常获得墙壁信息

    sender << GetRandomHint(catch_hints) << "\n\n";
    sender << At(t) << " 被捕捉！";

    if (step == 1) player.achievement.catch_without_moving = true;  // 成就【守株待兔】

    // [金币] 抢夺目标持有的全部金币（首轮保护不淘汰玩家，但金币照常被抢夺）
    const auto steal_coins = [&]() {
        if (const int stolen = player.StealCoinsFrom(Main().board.players[t]); stolen > 0) {
            sender << "\n" << At(player.pid) << " 抢夺了 " << At(t) << " 持有的 " << stolen << " 枚【金币】！";
        }
    };

    if (Main().round_ == 1) {
        player.NewContentRecord("(首轮捕捉)", "catch");
        player.NewContentRecord("[传送]", "teleport");

        Player& target = Main().board.players[t];
        target.NewContentRecord("[首轮被抓传送]", "teleport");
        if (t.Get() < currentPlayer.Get()) {    // 被抓玩家在前面，强制刷新完整赛况
            target.all_record.back() = target.move_record;
        }

        steal_coins();

        Main().board.TeleportPlayer(t);     // 随机传送被捉方
        Main().board.TeleportPlayer(player.pid);    // 随机传送捕捉方
        player.achievement.catch_first_round = true;    // 成就【饥渴难耐】
        sender << "\n【首轮玩家保护】\n首轮捕捉不生效：双方均被随机传送至地图其他地方！";
        return true;
    }

    player.NewContentRecord("(捕捉)", "catch");
    Main().board.players[t].out = 1;
    Main().EliminatePlayer(t);
    player.score.catch_score += 100;        // 抓人分
    Main().board.players[t].score.catch_score -= 100;
    player.achievement.recordCatch(Global().PlayerNum());   // 成就[嗜杀成性]辅助

    steal_coins();
    // [BOSS] 捕捉后结算
    for (const auto& boss : Main().board.bosses) {
        boss->HandlePlayerCaught(player, Main().board.players[t], sender);
    }

    if (Main().Alive_() > 1) {
        player.NewContentRecord("[传送]", "teleport");
        Main().board.TeleportPlayer(player.pid);  // 随机传送捕捉方
        sender << "\n" << At(player.pid) << " 被随机传送至地图其他地方，捕捉目标顺位发生变更！\n";
        RefreshCatchOrder(sender);  // 捕捉顺位变更
    }

    if (Main().Alive_() == 1) {
        // 无逃生舱最后生还判定
        if (Main().board.TypeCount(GridType::EXIT) == 0) Main().withoutE_win_ = true;
        // 有逃生舱但死斗取胜判定
        if (Main().board.TypeCount(GridType::EXIT) > 0) Main().withE_win_ = true;
    }
    return true;
}

// [金币] 回合结束公示本回合获得金币的玩家（含捕捉抢夺所得）
void RoundStage::AnnounceCoinGains()
{
    bool any = false;
    for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
        if (Main().board.players[pid].coins_this_round > 0) { any = true; break; }
    }
    if (any) {
        string extra_record = "<br>【第 " + to_string(Main().round_) + " 回合】金币公示：";
        auto sender = Global().Boardcast();
        sender << "【金币公示】";
        for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
            Player& p = Main().board.players[pid];
            if (p.coins_this_round > 0) {
                sender << "\n" << At(pid) << " 获得了 " << p.coins_this_round << " 枚金币（当前共持有 " << p.coins << " 枚）";
                extra_record += "[" + to_string(pid) + "号]+" + to_string(p.coins_this_round) + "枚 ";
                p.coins_this_round = 0;
            }
        }
        Main().board.all_extra_record += extra_record;
    }
    // 公示完成后同步已公示数量：赛况徽章此后才展示最新持有量
    for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
        Main().board.players[pid].coins_public = Main().board.players[pid].coins;
    }
}

// 将玩家四周墙壁信息写入私信赛况并以流式输出到 sender
void RoundStage::AppendSurroundingWalls(Player& player, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
{
    auto [info, md] = Main().board.GetSurroundingWalls(player.pid);
    player.private_record = "【第 " + to_string(Main().round_) + " 回合】\n您所在位置的四周墙壁信息，按照 上下左右 顺序分别是：\n" + info;
    sender << player.private_record << "\n" << Markdown(md, (GRID_SIZE + WALL_SIZE * 2) + 40);
}

// 私信其他玩家发送声响信息
void RoundStage::SendSoundMessage(const int fromX, const int fromY, const Sound sound, const bool to_all,
        const bool is_first_sound, Boss* const sound_boss)
{
    // 声响传播方向的归属：BOSS声响记入发声BOSS，其余记入当前行动玩家
    const auto AddPropagation = [&](const string& direct_str) {
        if (sound == Sound::BOSS) {
            if (sound_boss != nullptr) sound_boss->AddSoundPropagation(direct_str);
        } else {
            Main().board.players[currentPlayer].AddSoundPropagation(direct_str);
        }
    };
    // 发声BOSS的名称前缀（多BOSS区分是谁发出的声响）
    const string boss_tag = sound_boss != nullptr ? sound_boss->GetBossTag() : "[BOSS]";
    // 首次声响保护：仅占位填充传播记录（保持 propagation.size() == PlayerNum 不变量），不私信任何方向
    if (is_first_sound) {
        const auto n = Global().PlayerNum();
        for (PlayerID pid = 0; pid < n; ++pid) {
            AddPropagation("首次");
        }
        return;
    }
    for (PlayerID pid = 0; pid < Global().PlayerNum(); ++pid) {
        if ((pid != currentPlayer || to_all) && Main().board.players[pid].out == 0) {
            string direction = Main().board.GetSoundDirection(fromX, fromY, Main().board.players[pid]);
            string step_info = "[" + to_string(currentPlayer) + "号(第" + to_string(step) + "步)]";

            // 记录声响信息
            string sound_message;
            if (Main().board.IsNearJammer(pid)) {
                // 被[屏蔽器]影响：声响传播方向被屏蔽
                sound_message = GetSoundMessage(sound, step_info, boss_tag, direction, true);
                AddPropagation("被屏蔽");
            } else {
                // 同格声响仅提示非捕捉目标（点杀模式下均提示）
                if (direction != "同格" || Main().board.players[currentPlayer].target != pid || GAME_OPTION(点杀)) {
                    sound_message = GetSoundMessage(sound, step_info, boss_tag, direction, false);
                }
                // 记录声响传播方向记录
                AddPropagation(direction);
            }
            // 发送声响私信
            if (!sound_message.empty()) {
                Global().Tell(pid) << sound_message;
            }
        } else {
            Main().board.players[currentPlayer].AddSoundPropagation("错误");
        }
    }
}

} // namespace GAME_MODULE_NAME

} // namespace game

} // namespace lgtbot
