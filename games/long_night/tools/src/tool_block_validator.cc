// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).
//
// 【工具】校验「漫漫长夜」区块数据（map/block_data.h）
// 游戏运行时只按格式直接加载区块数据，不做任何校验；修改区块数据后、实装前须运行本工具，无错误后再构建。
//
// ============================== 如何运行 ==============================
//
// 【方式一（推荐）】一键脚本，自动完成编译 + 校验（参数说明见 tool_block_validator.sh）：
//     bash games/long_night/tools/tool_block_validator.sh                      # 校验内置的 block_data.h
//     bash games/long_night/tools/tool_block_validator.sh build 草稿本.json     # 校验外部 JSON 文件
//
// 【方式二】手动分步，以下命令均在仓库根目录执行：
//   1. 编译
//        g++ -std=c++23 -I. -Ithird_party -Ithird_party/json/include -O1 -DGAME_MODULE_NAME=long_night \
//            -o build/tool_block_validator \
//            games/long_night/tools/src/tool_block_validator.cc utility/html.cc
//   2. 校验
//        ./build/tool_block_validator                     # 内置数据
//        ./build/tool_block_validator --json <文件>        # 外部 JSON 文件
//
// 【校验内容】
//   格式：JSON 语法、重复的键、区块编号（1 / E1 / S1）、必填字段与取值（名称、地形、附着、墙面、坐标）、
//         墙面的 wall 与 type 一致（无墙时 type 为「空」）、未知字段（警告）
//   格子：每个区块 9 格且坐标不重复；区块内相邻两格共用的墙面须一致（左格的右墙 = 右格的左墙，上格的下墙 = 下格的上墙）；
//         墙面附着写在其中一侧即可，两侧都写时须相同
//   传送门：须有 portal_to，目标须为传送门且互相指向；非传送门不应有 portal_to
//   按钮：须有 button_doors，目标墙面须为门，且不可重复列出同一扇门；非按钮不应有 button_doors
//   目录：含热源的区块主要地形须为热源，含小太阳的区块主要附着须为小太阳（热源与小太阳互斥规则依赖此标记）
//   模式：各模式区块池引用的编号均存在、逃生舱成对且足够；经典模式至少 12 个普通区块与 4 个逃生舱
//   加载：用游戏的加载器完整加载一遍
//
// 退出码：无错误为 0（或存在警告），有错误为 1
// ====================================================================

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <span>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "bot_core/id.h"
#include "bot_core/msg_sender.h"   // ChildMsgSenderBase
#include "utility/html.h"

using namespace std;

#include "games/long_night/common/constants.h"
#include "games/long_night/actor/player.h"
#include "games/long_night/map/grid.h"
#include "games/long_night/map/map.h"

namespace {

using Json = BlockLoader::Json;
constexpr int k_size = 3;

string Arg(int argc, char** argv, const string& key, const string& fallback)
{
    for (int i = 1; i + 1 < argc; i++) {
        if (argv[i] == key) return argv[i + 1];
    }
    return fallback;
}

// 坐标沿用 JSON 的写法：y 为行，x 为列
string PosText(const int row, const int col)
{
    return "(y=" + to_string(row) + ", x=" + to_string(col) + ")";
}

template <typename T, size_t N>
optional<T> LookupName(const std::pair<const char*, T> (&names)[N], const string& name)
{
    for (const auto& [n, value] : names) {
        if (name == n) return value;
    }
    return nullopt;
}

string WallText(const WallType type)
{
    for (const auto& [name, value] : BlockLoader::k_wall_names) {
        if (value == type) return name;
    }
    return "?";
}

int SideIndex(const string& side)
{
    for (int d = 0; d < 4; ++d) {
        if (side == BlockLoader::k_side_keys[d]) return d;
    }
    return -1;
}

bool IsPortal(const GridType type) { return type == GridType::PORTAL || type == GridType::ONEWAYPORTAL; }
bool IsDoor(const WallType type) { return type == WallType::DOOR || type == WallType::DOOROPEN; }

class Validator
{
  public:
    vector<string> errors;
    vector<string> warnings;
    // 统计（用于报告）
    int normal_num = 0, exit_num = 0, special_num = 0;
    int portal_num = 0, button_num = 0, door_num = 0, cell_label_num = 0, wall_label_num = 0;

    // 解析 JSON，同时检查重复的键（nlohmann 遇到重复键会静默只保留一个）
    optional<Json> Parse(const string& text, const bool embedded)
    {
        struct Frame { std::set<string> keys; };
        vector<Frame> stack;
        string current_block;
        const auto callback = [&](int /*depth*/, Json::parse_event_t event, Json& parsed) {
            switch (event) {
                case Json::parse_event_t::object_start:
                case Json::parse_event_t::array_start:  stack.emplace_back(); break;
                case Json::parse_event_t::object_end:
                case Json::parse_event_t::array_end:    stack.pop_back(); break;
                case Json::parse_event_t::key: {
                    const string key = parsed.get<string>();
                    if (stack.size() == 1) current_block = key;
                    if (!stack.back().keys.insert(key).second) {
                        if (stack.size() == 1) Error("区块 " + key, "区块编号重复（只会保留其中一个）");
                        else Error("区块 " + current_block, "同一对象中重复的字段「" + key + "」");
                    }
                    break;
                }
                default: break;
            }
            return true;
        };
        try {
            return Json::parse(text, callback);
        } catch (const Json::parse_error& e) {
            // e.byte 为出错字符的位置（从 1 开始），换算为行号
            const size_t end = std::min<size_t>(e.byte > 0 ? e.byte - 1 : 0, text.size());
            const auto newlines = std::count(text.begin(), text.begin() + end, '\n');
            Error(embedded ? "JSON 语法错误（block_data.h 第 " + to_string(k_block_json_line + newlines) + " 行）"
                           : "JSON 语法错误（第 " + to_string(newlines + 1) + " 行）", e.what());
            return nullopt;
        }
    }

    // 校验全部区块
    void CheckBlocks(const Json& root)
    {
        if (!root.is_object()) {
            Error("JSON", "顶层应为以区块编号为键的对象");
            return;
        }
        for (const auto& [key, block] : root.items()) {
            if (!key.empty() && key[0] == 'E') ++exit_num;
            else if (!key.empty() && key[0] == 'S') ++special_num;
            else ++normal_num;
            CheckBlock(key, block);
        }
    }

    // 校验各模式区块池（区块池写在 map.h 的 UnitMaps 中）
    void CheckModes(const Json& root)
    {
        if (!root.is_object()) return;
        std::set<string> keys;
        for (const auto& [key, block] : root.items()) keys.insert(key);

        std::unique_ptr<UnitMaps> unit_maps;
        try {
            unit_maps = std::make_unique<UnitMaps>();
        } catch (const std::exception& e) {
            Error("模式区块池", string("内置区块数据加载失败，跳过检查：") + e.what());
            return;
        }
        const UnitMaps& um = *unit_maps;
        if (normal_num < um.k_map_num) Error("经典模式", "至少需要 " + to_string(um.k_map_num) + " 个普通区块，当前 " + to_string(normal_num) + " 个");
        if (exit_num < um.k_exit_num) Error("经典模式", "至少需要 " + to_string(um.k_exit_num) + " 个逃生舱，当前 " + to_string(exit_num) + " 个");

        const vector<pair<string, const vector<string>*>> modes = {
            {"幻变", &um.twist_mode_ids}, {"按钮", &um.button_mode_ids}, {"陷阱", &um.trap_mode_ids},
            {"空旷", &um.open_mode_ids}, {"无声", &um.silent_mode_ids},
        };
        for (const auto& [mode, ids] : modes) {
            const string where = "「" + mode + "」模式区块池";
            std::set<string> seen;
            int exit_count = 0;
            for (const auto& id : *ids) {
                if (!keys.count(id)) Error(where, "引用的区块 " + id + " 不存在");
                if (!seen.insert(id).second) Warn(where, "区块 " + id + " 重复");
                if (!id.empty() && id[0] == 'E') ++exit_count;
            }
            if (exit_count % 2 != 0) {
                Error(where, "逃生舱应两两成对，当前 " + to_string(exit_count) + " 个（为奇数时该模式抽不到逃生舱）");
            } else if (exit_count / 2 < um.k_exit_num / 2) {
                Error(where, "逃生舱至少需要 " + to_string(um.k_exit_num / 2) + " 对，当前 " + to_string(exit_count / 2) + " 对");
            }
        }
    }

    // 用游戏的加载器完整加载一遍
    void CheckLoad(const string& text)
    {
        try {
            BlockLoader::Parse(text);
        } catch (const std::exception& e) {
            Error("游戏加载器", e.what());
        }
    }

  private:
    // 解析后的格子信息，供格子之间交叉校验
    struct Cell {
        bool valid = false;                 // 地形、附着与四面墙均解析成功
        GridType type = GridType::EMPTY;
        AttachType attach = AttachType::EMPTY;
        WallType walls[4] = {WallType::EMPTY, WallType::EMPTY, WallType::EMPTY, WallType::EMPTY};
        WallAttachType wall_attaches[4] = {WallAttachType::EMPTY, WallAttachType::EMPTY, WallAttachType::EMPTY, WallAttachType::EMPTY};
        const Json* ground = nullptr;
    };
    using Cells = std::array<std::array<Cell, k_size>, k_size>;

    void Error(const string& where, const string& what) { errors.push_back(where + "：" + what); }
    void Warn(const string& where, const string& what) { warnings.push_back(where + "：" + what); }

    // 只允许出现 allowed 中的字段，其余给出警告（多为拼写错误）
    void CheckKeys(const string& where, const Json& obj, const std::initializer_list<const char*> allowed)
    {
        if (!obj.is_object()) return;
        for (const auto& [key, value] : obj.items()) {
            if (std::none_of(allowed.begin(), allowed.end(), [&](const char* k) { return key == k; })) {
                Warn(where, "未知字段「" + key + "」");
            }
        }
    }

    // 读取字符串字段：缺失（必填时）或类型不对则报错
    optional<string> GetString(const string& where, const Json& obj, const char* key, const bool required)
    {
        if (!obj.is_object() || !obj.contains(key)) {
            if (required) Error(where, string("缺少字段 ") + key);
            return nullopt;
        }
        const Json& value = obj.at(key);
        if (!value.is_string()) {
            Error(where, string(key) + " 应为字符串");
            return nullopt;
        }
        return value.get<string>();
    }

    // 读取坐标 {"y": 行, "x": 列}，须在区块范围内
    optional<pair<int, int>> GetPos(const string& where, const Json& pos)
    {
        if (!pos.is_object() || !pos.contains("y") || !pos.contains("x")) {
            Error(where, "坐标应为 {\"y\": 行, \"x\": 列}");
            return nullopt;
        }
        if (!pos.at("y").is_number_integer() || !pos.at("x").is_number_integer()) {
            Error(where, "坐标应为整数");
            return nullopt;
        }
        const int row = pos.at("y").get<int>(), col = pos.at("x").get<int>();
        if (row < 0 || row >= k_size || col < 0 || col >= k_size) {
            Error(where, "坐标 " + PosText(row, col) + " 超出区块范围（0~2）");
            return nullopt;
        }
        return pair{row, col};
    }

    void CheckBlock(const string& key, const Json& block)
    {
        const string where = "区块 " + key;
        if (!std::regex_match(key, std::regex("(E|S)?[1-9][0-9]*"))) Error(where, "编号应为 1 / E1 / S1 的形式");
        if (!block.is_object()) {
            Error(where, "应为包含 name / type / cells 的对象");
            return;
        }
        CheckKeys(where, block, {"name", "type", "attach", "cells"});

        if (const auto name = GetString(where, block, "name", true); name && name->empty()) Error(where, "name 不能为空");
        optional<GridType> main_type;
        if (const auto type = GetString(where, block, "type", true)) {
            main_type = LookupName(BlockLoader::k_grid_names, *type);
            if (!main_type) Error(where, "未知主要地形「" + *type + "」");
        }
        optional<AttachType> main_attach = AttachType::EMPTY;
        if (const auto attach = GetString(where, block, "attach", false)) {
            main_attach = LookupName(BlockLoader::k_attach_names, *attach);
            if (!main_attach) Error(where, "未知主要附着「" + *attach + "」");
        }

        if (!block.contains("cells")) {
            Error(where, "缺少字段 cells");
            return;
        }
        const Json& cells = block.at("cells");
        if (!cells.is_array()) {
            Error(where, "cells 应为 9 个格子组成的数组");
            return;
        }
        if (cells.size() != k_size * k_size) Error(where, "cells 应为 9 个格子，当前 " + to_string(cells.size()) + " 个");

        Cells grid{};
        std::array<std::array<bool, k_size>, k_size> seen{};
        for (size_t i = 0; i < cells.size(); ++i) {
            const Json& cell = cells[i];
            const string cell_where = where + " 第 " + to_string(i + 1) + " 格";
            if (!cell.is_object() || !cell.contains("pos")) {
                Error(cell_where, "缺少坐标 pos");
                continue;
            }
            CheckKeys(cell_where + " pos", cell.at("pos"), {"y", "x"});
            const auto pos = GetPos(cell_where + " pos", cell.at("pos"));
            if (!pos) continue;
            const auto [row, col] = *pos;
            const string at = where + " " + PosText(row, col);
            if (seen[row][col]) {
                Error(at, "格子重复");
                continue;
            }
            seen[row][col] = true;
            CheckCell(at, cell, grid[row][col]);
        }
        for (int row = 0; row < k_size; ++row) {
            for (int col = 0; col < k_size; ++col) {
                if (!seen[row][col]) Error(where, "缺少格子 " + PosText(row, col));
            }
        }

        CheckSharedWalls(where, grid);
        for (int row = 0; row < k_size; ++row) {
            for (int col = 0; col < k_size; ++col) {
                if (!grid[row][col].valid) continue;
                const string at = where + " " + PosText(row, col);
                CheckPortal(at, grid, row, col);
                CheckButton(at, grid, row, col);
            }
        }
        CheckCatalog(where, grid, main_type, main_attach);
    }

    // 格子：地形、附着、标注与四面墙
    void CheckCell(const string& at, const Json& cell, Cell& out)
    {
        CheckKeys(at, cell, {"pos", "ground", "top", "bottom", "left", "right"});
        bool valid = true;
        if (!cell.contains("ground") || !cell.at("ground").is_object()) {
            Error(at, "缺少地面 ground");
            valid = false;
        } else {
            const Json& ground = cell.at("ground");
            const string ground_at = at + " ground";
            out.ground = &ground;
            CheckKeys(ground_at, ground, {"type", "attach", "label", "portal_to", "button_doors"});
            if (const auto type = GetString(ground_at, ground, "type", true)) {
                if (const auto t = LookupName(BlockLoader::k_grid_names, *type)) out.type = *t;
                else { Error(ground_at, "未知地形「" + *type + "」"); valid = false; }
            } else {
                valid = false;
            }
            if (const auto attach = GetString(ground_at, ground, "attach", false)) {
                if (const auto a = LookupName(BlockLoader::k_attach_names, *attach)) out.attach = *a;
                else { Error(ground_at, "未知附着「" + *attach + "」"); valid = false; }
            } else if (ground.contains("attach")) {
                valid = false;
            }
            if (GetString(ground_at, ground, "label", false)) ++cell_label_num;
        }
        for (int d = 0; d < 4; ++d) {
            const char* side_key = BlockLoader::k_side_keys[d];
            const string side_at = at + " " + side_key;
            if (!cell.contains(side_key) || !cell.at(side_key).is_object()) {
                Error(at, string("缺少墙面 ") + side_key);
                valid = false;
                continue;
            }
            const Json& side = cell.at(side_key);
            CheckKeys(side_at, side, {"wall", "type", "label", "attach"});
            if (!side.contains("wall") || !side.at("wall").is_boolean()) {
                Error(side_at, "wall 应为 true / false");
                valid = false;
                continue;
            }
            // 游戏按 type 解析墙面，wall 供草稿本绘制，两者须一致
            if (const auto type = GetString(side_at, side, "type", true)) {
                if (const auto w = LookupName(BlockLoader::k_wall_names, *type)) {
                    out.walls[d] = *w;
                    const bool has_wall = side.at("wall").get<bool>();
                    if (!has_wall && *w != WallType::EMPTY) Error(side_at, "wall 为 false 时 type 应为「空」，当前为「" + *type + "」");
                    if (has_wall && *w == WallType::EMPTY) Error(side_at, "wall 为 true 时 type 不能为「空」");
                } else {
                    Error(side_at, "未知墙面类型「" + *type + "」");
                    valid = false;
                }
            } else {
                valid = false;
            }
            if (GetString(side_at, side, "label", false)) ++wall_label_num;
            if (const auto attach = GetString(side_at, side, "attach", false)) {
                if (const auto a = LookupName(BlockLoader::k_wall_attach_names, *attach)) out.wall_attaches[d] = *a;
                else { Error(side_at, "未知墙面附着「" + *attach + "」"); valid = false; }
            } else if (side.contains("attach")) {
                valid = false;
            }
        }
        out.valid = valid;
    }

    // 区块内相邻两格共用的墙面须一致（区块之间的边界在拼接地图时按优先级合并，不在此校验）
    // 墙面附着写在其中一侧即可，两侧都写时须相同
    void CheckSharedWalls(const string& where, const Cells& grid)
    {
        const auto check = [&](const int row, const int col, const Direct dir, const int next_row, const int next_col) {
            const Cell& a = grid[row][col];
            const Cell& b = grid[next_row][next_col];
            if (!a.valid || !b.valid) return;
            const int d = static_cast<int>(dir), od = static_cast<int>(opposite(dir));
            const string a_side = PosText(row, col) + " 的 " + BlockLoader::k_side_keys[d];
            const string b_side = PosText(next_row, next_col) + " 的 " + BlockLoader::k_side_keys[od];
            if (a.walls[d] != b.walls[od]) {
                Error(where, a_side + "（" + WallText(a.walls[d]) + "）与 " + b_side + "（" + WallText(b.walls[od]) + "）不一致");
            }
            if (a.wall_attaches[d] != WallAttachType::EMPTY && b.wall_attaches[od] != WallAttachType::EMPTY &&
                a.wall_attaches[d] != b.wall_attaches[od]) {
                Error(where, a_side + " 与 " + b_side + " 的墙面附着不一致");
            }
        };
        for (int row = 0; row < k_size; ++row) {
            for (int col = 0; col < k_size; ++col) {
                if (col + 1 < k_size) check(row, col, Direct::RIGHT, row, col + 1);
                if (row + 1 < k_size) check(row, col, Direct::DOWN, row + 1, col);
            }
        }
    }

    // 传送门：portal_to 为配对的目标格，目标须为传送门且指回本格（可指向自身）
    void CheckPortal(const string& at, const Cells& grid, const int row, const int col)
    {
        const Cell& cell = grid[row][col];
        const Json& ground = *cell.ground;
        const bool is_portal = IsPortal(cell.type);
        if (!ground.contains("portal_to")) {
            if (is_portal) Error(at, "传送门缺少 portal_to");
            return;
        }
        if (!is_portal) {
            Error(at, "不是传送门，不应有 portal_to");
            return;
        }
        ++portal_num;
        CheckKeys(at + " portal_to", ground.at("portal_to"), {"y", "x"});
        const auto target = GetPos(at + " portal_to", ground.at("portal_to"));
        if (!target) return;
        const auto [to_row, to_col] = *target;
        const Cell& to_cell = grid[to_row][to_col];
        if (!to_cell.valid) return;
        if (!IsPortal(to_cell.type)) {
            Error(at, "portal_to 的目标 " + PosText(to_row, to_col) + " 不是传送门");
            return;
        }
        // 目标格的 portal_to 有误时由目标格自身报错，这里只判断是否指回本格
        const Json& to_ground = *to_cell.ground;
        if (!to_ground.contains("portal_to")) return;
        const Json& back = to_ground.at("portal_to");
        if (!back.is_object() || !back.contains("y") || !back.contains("x") ||
            !back.at("y").is_number_integer() || !back.at("x").is_number_integer()) return;
        if (pair{back.at("y").get<int>(), back.at("x").get<int>()} != pair{row, col}) {
            Error(at, "与 " + PosText(to_row, to_col) + " 的传送门未互相指向");
        }
    }

    // 按钮：button_doors 为按下时切换的门，目标墙面须为门；相邻格共用同一扇门，按下时两面一起切换
    void CheckButton(const string& at, const Cells& grid, const int row, const int col)
    {
        const Cell& cell = grid[row][col];
        const Json& ground = *cell.ground;
        const bool is_button = cell.attach == AttachType::BUTTON;
        if (!ground.contains("button_doors")) {
            if (is_button) Error(at, "按钮缺少 button_doors");
            return;
        }
        if (!is_button) {
            Error(at, "不是按钮，不应有 button_doors");
            return;
        }
        ++button_num;
        const string doors_at = at + " button_doors";
        const Json& doors = ground.at("button_doors");
        if (!doors.is_array() || doors.empty()) {
            Error(doors_at, "应为非空数组");
            return;
        }
        std::set<std::tuple<int, int, int>> edges;     // 已列出的门：取坐标较小一侧的格子与墙面，表示同一扇门
        for (size_t i = 0; i < doors.size(); ++i) {
            const Json& door = doors[i];
            const string door_at = doors_at + " 第 " + to_string(i + 1) + " 项";
            CheckKeys(door_at, door, {"y", "x", "side"});
            const auto target = GetPos(door_at, door);
            const auto side = GetString(door_at, door, "side", true);
            if (!target || !side) continue;
            const int d = SideIndex(*side);
            if (d < 0) {
                Error(door_at, "未知墙面「" + *side + "」（应为 top / bottom / left / right）");
                continue;
            }
            ++door_num;
            const auto [to_row, to_col] = *target;
            const Cell& to_cell = grid[to_row][to_col];
            if (!to_cell.valid) continue;
            if (!IsDoor(to_cell.walls[d])) {
                Error(door_at, "目标 " + PosText(to_row, to_col) + " 的 " + *side + " 不是门（为" + WallText(to_cell.walls[d]) + "）");
                continue;
            }
            int edge_row = to_row, edge_col = to_col, edge_dir = d;
            const int next_row = to_row + k_DX_Direct[d], next_col = to_col + k_DY_Direct[d];
            if (0 <= next_row && next_row < k_size && 0 <= next_col && next_col < k_size && pair{next_row, next_col} < pair{to_row, to_col}) {
                edge_row = next_row;
                edge_col = next_col;
                edge_dir = static_cast<int>(opposite(static_cast<Direct>(d)));
            }
            if (!edges.insert({edge_row, edge_col, edge_dir}).second) {
                Error(door_at, "目标 " + PosText(to_row, to_col) + " 的 " + *side + " 与之前列出的是同一扇门（重复切换会相互抵消）");
            }
        }
    }

    // 主要地形 / 附着：热源与小太阳互斥规则按此标记识别区块
    void CheckCatalog(const string& where, const Cells& grid, const optional<GridType> main_type, const optional<AttachType> main_attach)
    {
        bool has_heat = false, has_heatbox = false;
        for (const auto& line : grid) {
            for (const Cell& cell : line) {
                if (cell.type == GridType::HEAT) has_heat = true;
                if (cell.attach == AttachType::HEATBOX) has_heatbox = true;
            }
        }
        if (main_type) {
            if (has_heat && *main_type != GridType::HEAT) Error(where, "含热源，主要地形 type 须为「热源」（热源与小太阳互斥规则依赖此标记）");
            if (!has_heat && *main_type == GridType::HEAT) Warn(where, "主要地形为热源，但区块中没有热源");
        }
        if (main_attach) {
            if (has_heatbox && *main_attach != AttachType::HEATBOX) Error(where, "含小太阳，主要附着 attach 须为「小太阳」（热源与小太阳互斥规则依赖此标记）");
            if (!has_heatbox && *main_attach == AttachType::HEATBOX) Warn(where, "主要附着为小太阳，但区块中没有小太阳");
        }
    }
};

}  // namespace

int main(int argc, char** argv)
{
    const string json_file = Arg(argc, argv, "--json", "");
    string text;
    if (json_file.empty()) {
        text = string(k_block_json);
    } else {
        std::ifstream in(json_file);
        if (!in) {
            cerr << "[错误] 无法读取文件：" << json_file << endl;
            return 1;
        }
        std::stringstream buffer;
        buffer << in.rdbuf();
        text = buffer.str();
    }

    Validator validator;
    if (const auto root = validator.Parse(text, json_file.empty())) {
        validator.CheckBlocks(*root);
        validator.CheckModes(*root);
        validator.CheckLoad(text);
    }

    cout << "[区块数据校验] 来源：" << (json_file.empty() ? "内置区块数据 map/block_data.h" : json_file) << "\n";
    cout << "区块 " << validator.normal_num + validator.exit_num + validator.special_num << " 个（普通 " << validator.normal_num
         << " / 逃生舱 " << validator.exit_num << " / 特殊 " << validator.special_num << "）；传送门 " << validator.portal_num
         << " 格，按钮 " << validator.button_num << " 个（控制 " << validator.door_num << " 扇门），标注 "
         << validator.cell_label_num << " 格 / " << validator.wall_label_num << " 面墙\n";
    for (const auto& error : validator.errors) cout << "  ✗ " << error << "\n";
    for (const auto& warning : validator.warnings) cout << "  △ " << warning << "\n";
    cout << (validator.errors.empty() ? "结果：通过" : "结果：未通过") << "（错误 " << validator.errors.size()
         << " 个，警告 " << validator.warnings.size() << " 个）" << endl;
    return validator.errors.empty() ? 0 : 1;
}
