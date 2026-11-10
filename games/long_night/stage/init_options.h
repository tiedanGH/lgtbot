// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).
//
// 开局快捷配置指令（InitOptionsCommand）
// 由 mygame.cc 在游戏命名空间之外包含，依赖框架的 CustomOptions / InitOptionsCommand 及 options.h 生成的配置项

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

// 快捷配置项：名称 + 对游戏配置的修改（single_user 以单机模式开局）
struct InitShortcut
{
    const char* name;
    void (*apply)(CustomOptions& options, bool& single_user);
};

// 全部快捷配置：按输入顺序依次应用，冲突配置以靠后的为准
const InitShortcut k_init_shortcuts[] = {
    // ===== 区块模式 =====
    {"经典",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 模式) = BlockMode::CLASSIC; }},
    {"幻变",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 模式) = BlockMode::TWIST; }},
    {"狂野",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 模式) = BlockMode::WILD; }},
    {"疯狂",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 模式) = BlockMode::CRAZY; }},
    {"按钮",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 模式) = BlockMode::BUTTON; }},
    {"陷阱",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 模式) = BlockMode::TRAP; }},
    {"空旷",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 模式) = BlockMode::OPEN; }},
    {"无声",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 模式) = BlockMode::SILENT; }},
    {"拓展",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 拓展) = true; }},

    // ===== 边长 =====
    {"10x10",      [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 边长) = 10; }},
    {"11x11",      [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 边长) = 11; }},
    {"12x12",      [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 边长) = 12; }},

    // ===== 特殊事件 =====
    {"随机",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 特殊事件) = SpecialEvent::RANDOM; }},
    {"怠惰的园丁", [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 特殊事件) = SpecialEvent::LAZYGARDENER; }},
    {"营养过剩",   [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 特殊事件) = SpecialEvent::OVERGROWTH; }},
    {"雨天小故事", [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 特殊事件) = SpecialEvent::RAINSTORY; }},
    {"十面埋伏",   [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 特殊事件) = SpecialEvent::AMBUSH; }},

    // ===== 游戏模式 =====
    {"大乱斗",     [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 大乱斗) = true; }},
    {"回合隐匿",   [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 隐匿) = HideMode::TURN; }},
    {"单步隐匿",   [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 隐匿) = HideMode::STEP; }},
    {"点杀",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 点杀) = true; }},
    {"关闭点杀",   [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 点杀) = false; }},
    {"谋定后动",   [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 谋定后动) = true; }},
    {"炸弹人",     [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 炸弹) = 1; }},
    {"静音",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 静音) = true; }},
    {"摸黑",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 摸黑) = true; }},
    {"传送紊乱",   [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 传送紊乱) = true; }},

    // ===== BOSS：多个可同时生效 =====
    {"米诺陶斯",   [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, BOSS).push_back(BossType::MINOTAUR); }},
    {"邦邦",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, BOSS).push_back(BossType::BANGBANG); }},
    {"暴君",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, BOSS).push_back(BossType::TYRANT); }},

    // ===== 其他配置 =====
    {"上家",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 捕捉目标) = Target::PREVIOUS; }},
    {"下家",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 捕捉目标) = Target::NEXT; }},
    {"停止私信",   [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 停止信息) = StopInfo::PRIVATE; }},
    {"停止公开",   [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 停止信息) = StopInfo::PUBLIC; }},
    {"复古",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 纹理) = Texture::RETRO; }},
    {"清新",       [](CustomOptions& o, bool&) { GET_OPTION_VALUE(o, 纹理) = Texture::FRESH; }},

    // ===== 启动模式 =====
    {"单机",       [](CustomOptions&, bool& single_user) { single_user = true; }},
    {"教程",       [](CustomOptions& o, bool& single_user) { GET_OPTION_VALUE(o, 教程) = true; single_user = true; }},
};

const std::vector<InitOptionsCommand> k_init_options_commands = {
    InitOptionsCommand("一键设定特殊事件或游戏模式：空格分隔，冲突配置以靠后的为准",
            [] (CustomOptions& game_options, MutableGenericOptions& generic_options, const vector<int>& shortcut_ids)
            {
                bool single_user = false;
                for (const int id : shortcut_ids) {
                    k_init_shortcuts[id].apply(game_options, single_user);
                }
                if (GET_OPTION_VALUE(game_options, 教程)) generic_options.bench_computers_to_player_num_ = k_tutorial_player_num;
                return single_user ? NewGameMode::SINGLE_USER : NewGameMode::MULTIPLE_USERS;
            },
            RepeatableChecker<AlterChecker<int>>([] {
                map<string, int> names;     // 名称 → k_init_shortcuts 下标
                for (int i = 0; i < static_cast<int>(std::size(k_init_shortcuts)); ++i) {
                    names.emplace(k_init_shortcuts[i].name, i);
                }
                return names;
            }())),
};

} // namespace GAME_MODULE_NAME

} // namespace game

} // namespace lgtbot
