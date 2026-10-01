// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).
//
// 区块数据加载：将 block_data.h 中的 JSON 直接解析为区块，首次取用时解析并缓存
// 数据在实装前由 tools/tool_block_validator 校验，加载出错时抛出异常并注明区块编号

#include "nlohmann/json.hpp"

#include "block_data.h"

class BlockLoader
{
  public:
    using Json = nlohmann::ordered_json;    // 保留区块在数据中的先后顺序

    // 区块数据中的一个区块
    struct Entry {
        string key;                         // 编号：普通区块 "1"、逃生舱 "E1"、特殊区块 "S1"
        string name;                        // 名称
        GridType type;                      // 主要地形
        AttachType attach;                  // 主要附着
        GridMap block;                      // 3×3 方格与墙壁
    };

    // 全部区块，顺序与 block_data.h 一致
    static const vector<Entry>& All()
    {
        static const vector<Entry> entries = Parse(k_block_json);
        return entries;
    }

    // 墙面字段名，顺序与 Direct 一致（上 / 下 / 左 / 右）
    static constexpr const char* k_side_keys[4] = {"top", "bottom", "left", "right"};

    static constexpr std::pair<const char*, GridType> k_grid_names[] = {
        {"空地", GridType::EMPTY}, {"树丛", GridType::GRASS}, {"水洼", GridType::WATER},
        {"传送门", GridType::PORTAL}, {"逃生舱", GridType::EXIT}, {"陷阱", GridType::TRAP},
        {"热源", GridType::HEAT}, {"单向传送门", GridType::ONEWAYPORTAL}, {"浆果丛", GridType::BERRY},
        {"心脏", GridType::HEART},
    };
    static constexpr std::pair<const char*, AttachType> k_attach_names[] = {
        {"无", AttachType::EMPTY}, {"按钮", AttachType::BUTTON}, {"炸弹", AttachType::BOMB},
        {"箱子", AttachType::BOX}, {"小太阳", AttachType::HEATBOX}, {"屏蔽器", AttachType::JAMMERBOX},
        {"金币", AttachType::COIN},
    };
    // 墙面类型（无墙为「空」）
    static constexpr std::pair<const char*, WallType> k_wall_names[] = {
        {"空", WallType::EMPTY}, {"普通", WallType::NORMAL}, {"树篱", WallType::HEDGE}, {"门", WallType::DOOR}, {"门 (开)", WallType::DOOROPEN},
    };
    // 墙面附着（可选字段）
    static constexpr std::pair<const char*, WallAttachType> k_wall_attach_names[] = {
        {"无", WallAttachType::EMPTY},
    };

    // 解析全部区块；出错时抛出异常并注明出错的区块编号
    static vector<Entry> Parse(const std::string_view text)
    {
        string current = "JSON";
        try {
            const Json root = Json::parse(text);
            vector<Entry> entries;
            for (const auto& [key, value] : root.items()) {
                current = "区块 " + key;
                entries.push_back({key, value.at("name").get<string>(), FindByName(k_grid_names, value.at("type").get<string>()),
                                   value.contains("attach") ? FindByName(k_attach_names, value.at("attach").get<string>()) : AttachType::EMPTY,
                                   ParseCells(value.at("cells"))});
            }
            return entries;
        } catch (const std::exception& e) {
            throw std::runtime_error("[漫漫长夜区块数据] " + current + " 加载失败：" + e.what());
        }
    }

  private:
    template <typename T, size_t N>
    static T FindByName(const std::pair<const char*, T> (&names)[N], const string& name)
    {
        for (const auto& [n, value] : names) {
            if (name == n) return value;
        }
        throw std::runtime_error("未知名称「" + name + "」");
    }

    static int SideIndex(const string& side)
    {
        for (int d = 0; d < 4; ++d) {
            if (side == k_side_keys[d]) return d;
        }
        throw std::runtime_error("未知墙面「" + side + "」");
    }

    // 9 个格子 → 3×3 方格；相邻两格共用的墙在数据中两侧各写一次，合并到同一个墙壁对象
    static GridMap ParseCells(const Json& cells)
    {
        GridMap block(3, false);
        for (const Json& cell : cells) {
            const int row = cell.at("pos").at("y").get<int>();
            const int col = cell.at("pos").at("x").get<int>();
            Grid& grid = block.at(row).at(col);

            const Json& ground = cell.at("ground");
            grid.SetType(FindByName(k_grid_names, ground.at("type").get<string>()));
            if (ground.contains("attach")) grid.SetAttach(FindByName(k_attach_names, ground.at("attach").get<string>()));
            if (ground.contains("label")) grid.SetContent(ground.at("label").get<string>());
            if (ground.contains("portal_to")) {
                const Json& to = ground.at("portal_to");
                grid.SetPortal(to.at("y").get<int>() - row, to.at("x").get<int>() - col);
            }
            if (ground.contains("button_doors")) {
                vector<Grid::ButtonTarget> targets;
                for (const Json& door : ground.at("button_doors")) {
                    targets.push_back({door.at("y").get<int>() - row, door.at("x").get<int>() - col,
                                       static_cast<Direct>(SideIndex(door.at("side").get<string>()))});
                }
                grid.SetButton(targets);
            }

            for (int d = 0; d < 4; ++d) {
                const Json& side = cell.at(k_side_keys[d]);
                Wall& wall = block.WallAt(row, col, static_cast<Direct>(d));
                wall.SetType(FindByName(k_wall_names, side.at("type").get<string>()));
                if (side.contains("label")) {
                    const string label = side.at("label").get<string>();
                    if (!label.empty()) wall.SetLabel(label);
                }
                if (side.contains("attach")) {
                    const WallAttachType attach = FindByName(k_wall_attach_names, side.at("attach").get<string>());
                    if (attach != WallAttachType::EMPTY) wall.SetAttach(attach);
                }
            }
        }
        return block;
    }
};
