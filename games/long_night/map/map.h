#include "wall.h"
#include "grid_map.h"
#include "block_loader.h"

// 感谢 BC 提供的初版区块初始化思路
class UnitMaps {
  public:
    const int k_map_num = 12;
    const int k_exit_num = 4;
    const int k_special_num = 2;
    // [拓展] 随机池扩大后的普通区块与逃生舱数量
    const int k_expand_map_num = 16;
    const int k_expand_exit_num = 6;
    // 区块图鉴的格子图标标记
    static constexpr const char* k_grow_mark = "？生长";      // [营养过剩] 可能长出树丛的空地
    static constexpr const char* k_move_mark = "？移动";      // [怠惰的园丁] 将移到随机位置的树丛
    static constexpr const char* k_rain_mark = "？雨天";      // [雨天小故事] 改为发出啪嗒声的陷阱、浆果丛
    static constexpr const char* k_ambush_mark = "！埋伏";    // [十面埋伏] 视为陷阱的浆果丛
    static constexpr const char* k_silence_mark = "✗静音";    // [静音] 不再发出声响的地形
    static constexpr const char* k_chaos_mark = "？紊乱";     // [传送紊乱] 打乱传送目标的传送门
    // 图标标记对应的图片名，不是图标标记时返回空
    static string MarkImage(const string& mark)
    {
        if (mark == k_grow_mark) return "event_grow.png";
        if (mark == k_move_mark) return "event_move.png";
        if (mark == k_rain_mark) return "event_rain.png";
        if (mark == k_ambush_mark) return "event_ambush.png";
        if (mark == k_silence_mark) return "mode_silence.png";
        if (mark == k_chaos_mark) return "mode_chaos.png";
        return "";
    }

    vector<pair<int, int>> pos = {
        {0, 0}, {0, 3}, {0, 6},
        {3, 0}, {3, 3}, {3, 6},
        {6, 0}, {6, 3}, {6, 6},
    };
    vector<pair<int, int>> origin_pos = pos;

    struct Map {
        GridMap block;
        string id;
        string title;
        GridType grid;
        AttachType attach;
        bool is_exit = false;
        bool is_special = false;
    };

    // 区块目录取自区块数据（block_data.h）
    vector<Map> all_maps = LoadMaps('\0');          // 普通区块
    vector<Map> all_exits = LoadMaps('E');          // 逃生舱
    vector<Map> all_special_maps = LoadMaps('S');   // 特殊区块

    // 幻变
    const vector<string> twist_mode_ids = {
        "45", "46", "47", "48",
        "49", "50", "51", "5",
        "2", "6", "13", "14",
        "4", "8", "9", "10",
        "11", "12", "23", "24",
        "E15", "E16", "E1", "E4",
        "E5", "E6", "E7", "E8",
    };
    // 按钮
    const vector<string> button_mode_ids = {
        "2", "6", "3", "7",
        "4", "8", "9", "10",
        "21", "22", "25", "26",
        "27", "37", "33", "34",
        "35", "36", "39", "40",
        "E1", "E2", "E3", "E4",
        "E11", "E12",
    };
    // 陷阱
    const vector<string> trap_mode_ids = {
        "2", "6", "9", "10",
        "19", "20", "21", "22",
        "23", "24", "25", "27",
        "31", "32", "39", "40",
        "16", "S3",
        "E1", "E2", "E3", "E4",
        "E9", "E10",
    };
    // 空旷
    const vector<string> open_mode_ids = {
        "23", "24", "25", "26",
        "27", "28", "43", "44",
        "51",
        "E1", "E2", "E3", "E14",
    };
    // 无声
    const vector<string> silent_mode_ids = {
        "9", "10", "19", "20",
        "21", "22", "27", "28",
        "47", "48",
        "E1", "E2", "E3", "E4",
    };
    vector<Map> pool_maps;
    vector<Map> pool_exits;

    vector<Map> maps;
    vector<Map> exits;
    vector<Map> special_maps;

    // [传送紊乱] 传送门图例增加漩涡
    bool portal_chaos = false;

    UnitMaps() = default;

    UnitMaps(const BlockMode mode, std::mt19937& gen, const vector<string>& custom_blocks, const bool expand): g(std::make_unique<std::mt19937>(gen))
    {
        // 随机池大小：[拓展] 扩大为 16+6
        const int map_num = expand ? k_expand_map_num : k_map_num;
        const int exit_pair = (expand ? k_expand_exit_num : k_exit_num) / 2;
        if (mode == BlockMode::CLASSIC) {
            // 经典-CLASSIC
            maps.insert(maps.end(), all_maps.begin(), all_maps.begin() + k_map_num);
            exits.insert(exits.end(), all_exits.begin(), all_exits.begin() + k_exit_num);
        } else if (mode == BlockMode::TWIST) {
            // 幻变-TWIST
            SampleBlockPoolsFromIds(twist_mode_ids, map_num, exit_pair);
        } else if (mode == BlockMode::WILD) {
            // 狂野-WILD
            std::sample(all_maps.begin(), all_maps.end(), std::back_inserter(maps), map_num, *g);
            SampleExits(all_exits, exit_pair);
        } else if (mode == BlockMode::CRAZY) {
            // 疯狂-CRAZY
            std::sample(all_special_maps.begin(), all_special_maps.end(), std::back_inserter(special_maps), k_special_num, *g);
            maps.insert(maps.end(), special_maps.begin(), special_maps.end());
            std::sample(all_maps.begin(), all_maps.end(), std::back_inserter(maps), map_num - k_special_num, *g);
            SampleExits(all_exits, exit_pair);
        } else if (mode == BlockMode::BUTTON) {
            // 按钮-BUTTON
            SampleBlockPoolsFromIds(button_mode_ids, map_num, exit_pair);
        } else if (mode == BlockMode::TRAP) {
            // 陷阱-TRAP
            SampleBlockPoolsFromIds(trap_mode_ids, map_num, exit_pair);
        } else if (mode == BlockMode::OPEN) {
            // 空旷-OPEN
            SampleBlockPoolsFromIds(open_mode_ids, map_num, exit_pair);
        } else if (mode == BlockMode::SILENT) {
            // 无声-SILENT
            SampleBlockPoolsFromIds(silent_mode_ids, map_num, exit_pair);
        } else {
            // 自定义-CUSTOM
            for (const auto& block : custom_blocks) {
                if (auto it = std::find_if(all_special_maps.begin(), all_special_maps.end(), [&](const Map& m) { return m.id == block; });
                    it != all_special_maps.end()) {
                    maps.push_back(*it);
                }
                const bool is_exit = !block.empty() && block[0] == 'E';
                string search_id = is_exit ? block.substr(1) : block;
                const auto& list = is_exit ? all_exits : all_maps;
                if (auto it = std::find_if(list.begin(), list.end(), [&](const Map& m) { return m.id == search_id; });
                    it != list.end()) {
                    (is_exit ? exits : maps).push_back(*it);
                }
            }
        }
    }

    void SampleBlockPoolsFromIds(const vector<string>& mode_ids, const int map_num, const int exit_pair)
    {
        maps.clear(); exits.clear();
        pool_maps.clear(); pool_exits.clear();

        for (const auto& map_id : mode_ids) {
            const bool is_exit = !map_id.empty() && map_id[0] == 'E';
            const bool is_special = !map_id.empty() && map_id[0] == 'S';
            const std::string id = is_exit ? map_id.substr(1) : map_id;

            const auto& source = is_exit ? all_exits : (is_special ? all_special_maps : all_maps);
            auto it = std::find_if(source.begin(), source.end(), [&id](const Map& m) { return m.id == id; });

            if (it != source.end()) {
                if (is_exit) pool_exits.push_back(*it);
                else pool_maps.push_back(*it);
            }
        }

        std::sample(pool_maps.begin(), pool_maps.end(), std::back_inserter(maps), map_num, *g);
        SampleExits(pool_exits, exit_pair);

        std::sort(maps.begin(), maps.end(), [](const Map& a, const Map& b) { return CompareMapId(a.id, b.id); });
        std::sort(exits.begin(), exits.end(), [](const Map& a, const Map& b) { return CompareMapId(a.id, b.id); });
    }

    void SampleExits(const vector<Map>& exits_pool, const int k_exit_pair)
    {
        if (exits_pool.size() % 2 != 0) return;

        int pair_num = exits_pool.size() / 2;
        vector<int> pairs(pair_num);
        std::iota(pairs.begin(), pairs.end(), 0);
        std::shuffle(pairs.begin(), pairs.end(), *g);
        pairs.resize(std::min(k_exit_pair, pair_num));     // 逃生舱不足时使用全部
        std::sort(pairs.begin(), pairs.end());
        for (int p : pairs) {
            exits.push_back(exits_pool[p * 2]);
            exits.push_back(exits_pool[p * 2 + 1]);
        }
    }

    bool HasHeatConflictPool(const vector<Map>& maps_pool) const
    {
        bool has_heat = false, has_heatbox = false;
        for (const auto& map : maps_pool) {
            if (IsHeatMap(map)) has_heat = true;
            if (IsHeatBoxMap(map)) has_heatbox = true;
        }
        return has_heat && has_heatbox;
    }

    // 自定义模式下，判断“热源/小太阳互斥规则”是否足够抽满普通区块，用【其余区块 + min(热源数, 小太阳数)】做保守检测
    bool CanApplyHeatExclusiveRule(const vector<Map>& maps_pool, const int required_map_num) const
    {
        int heat_count = 0;
        int heatbox_count = 0;
        int other_count = 0;

        for (const auto& map : maps_pool) {
            if (IsHeatMap(map)) ++heat_count;
            else if (IsHeatBoxMap(map)) ++heatbox_count;
            else ++other_count;
        }

        return other_count + std::min(heat_count, heatbox_count) >= required_map_num;
    }

    // 区块抽取随机
    vector<Map> RandomSelectBlocks(const BlockMode block_mode, const int exit_num) const
    {
        vector<Map> maps_pool = maps;
        vector<Map> exits_pool = exits;
        std::shuffle(maps_pool.begin(), maps_pool.end(), *g);
        std::shuffle(exits_pool.begin(), exits_pool.end(), *g);

        const int total_pos = static_cast<int>(pos.size());

        int selected_map_num = 0;
        int selected_exit_num = 0;

        if (block_mode != BlockMode::CUSTOM) {
            // 非自定义模式
            selected_map_num = total_pos - exit_num;
            selected_exit_num = exit_num;
        } else {
            // 自定义模式
            const int exit_count = static_cast<int>(exits_pool.size());

            if (exit_count < exit_num) {
                selected_exit_num = exit_count;
                selected_map_num = total_pos - selected_exit_num;
            } else {
                selected_exit_num = exit_num;
                selected_map_num = total_pos - selected_exit_num;
            }
        }

        // 热源 / 小太阳互斥规则
        bool enable_heat_exclusive_rule = false;
        if (HasHeatConflictPool(maps_pool)) {
            if (block_mode == BlockMode::CUSTOM) {
                enable_heat_exclusive_rule = CanApplyHeatExclusiveRule(maps_pool, selected_map_num);
            } else {
                enable_heat_exclusive_rule = true;
            }
        }

        vector<Map> selected_maps;
        if (enable_heat_exclusive_rule) {
            vector<Map> normal_maps;
            vector<Map> heat_maps;
            vector<Map> heatbox_maps;

            for (const auto& map : maps_pool) {
                if (IsHeatMap(map)) heat_maps.push_back(map);
                else if (IsHeatBoxMap(map)) heatbox_maps.push_back(map);
                else normal_maps.push_back(map);
            }

            // 二选一，只保留一种热系区块
            const bool choose_heat =
                heatbox_maps.empty() ? true :
                heat_maps.empty() ? false :
                (std::uniform_int_distribution<int>(0, 1)(*g) == 0);

            vector<Map> candidate_maps = normal_maps;
            if (choose_heat) {
                candidate_maps.insert(candidate_maps.end(), heat_maps.begin(), heat_maps.end());
            } else {
                candidate_maps.insert(candidate_maps.end(), heatbox_maps.begin(), heatbox_maps.end());
            }

            std::shuffle(candidate_maps.begin(), candidate_maps.end(), *g);

            // 双保险，避免极端情况下越界
            selected_map_num = std::min(selected_map_num, static_cast<int>(candidate_maps.size()));
            for (int i = 0; i < selected_map_num; ++i) {
                selected_maps.push_back(candidate_maps[i]);
            }
        } else {
            // 规则关闭，直接全池随机，防止自定义区块不足时越界
            selected_map_num = std::min(selected_map_num, static_cast<int>(maps_pool.size()));
            for (int i = 0; i < selected_map_num; ++i) {
                selected_maps.push_back(maps_pool[i]);
            }
        }

        vector<Map> selected;
        selected.reserve(selected_maps.size() + selected_exit_num);

        for (auto& map : selected_maps) {
            selected.push_back(map);
        }

        selected_exit_num = std::min(selected_exit_num, static_cast<int>(exits_pool.size()));
        for (int i = 0; i < selected_exit_num; ++i) {
            selected.push_back(exits_pool[i]);
        }

        std::shuffle(selected.begin(), selected.end(), *g);
        return selected;
    }

    GridMap FindBlockById(const string& id, bool is_exit, SpecialEvent event) const
    {
        const bool has_event = event != SpecialEvent::NONE;

        const auto& special_list = has_event ? special_maps : all_special_maps;
        if (const Map* map = FindMapById(special_list, id))
            return map->block;

        const auto& normal_list = is_exit
            ? (has_event ? exits : all_exits)
            : (has_event ? maps : all_maps);
        if (const Map* map = FindMapById(normal_list, id))
            return map->block;

        return InitializeMapTemplate();
    }

    bool IsBlockExist(const string& id, bool is_exit) const
    {
        if (FindMapById(all_special_maps, id))
            return true;

        const auto& list = is_exit ? all_exits : all_maps;
        return FindMapById(list, id) != nullptr;
    }

    const Map* FindMapById(const vector<Map>& maps, const string& id) const
    {
        auto it = std::find_if(maps.begin(), maps.end(),
            [&id](const Map& map) { return map.id == id; });

        return it != maps.end() ? &(*it) : nullptr;
    }

    static bool MapContainGridType(const vector<Map>& maps, const GridType& type)
    {
        for (const auto& map: maps) {
            for (int k = 0; k < 9; ++k) {
                if (map.block[k / 3][k % 3].Type() == type) {
                    return true;
                }
            }
        }
        return false;
    }

    static bool MapContainAttachType(const vector<Map>& maps, const AttachType& type)
    {
        for (const auto& map: maps) {
            for (int k = 0; k < 9; ++k) {
                if (map.block[k / 3][k % 3].Attach() == type) {
                    return true;
                }
            }
        }
        return false;
    }

    static bool MapContainWallType(const vector<Map>& maps, const WallType type)
    {
        for (const auto& map: maps) {
            if (map.block.ContainsWallType(type)) {
                return true;
            }
        }
        return false;
    }

    // [区块变化] 每个按钮都同时切换区块内全部的门且门至少 2 扇时，返回按下按钮后的区块
    static optional<GridMap> SwitchedBlock(const GridMap& block)
    {
        const int n = static_cast<int>(block.size());
        std::unordered_set<const Wall*> doors;
        vector<pair<int, int>> buttons;
        for (int x = 0; x < n; ++x) {
            for (int y = 0; y < n; ++y) {
                for (int d = 0; d < 4; ++d) {
                    const Wall& wall = block.WallAt(x, y, static_cast<Direct>(d));
                    if (wall.Type() == WallType::DOOR || wall.Type() == WallType::DOOROPEN) doors.insert(&wall);
                }
                if (block[x][y].Attach() == AttachType::BUTTON) buttons.emplace_back(x, y);
            }
        }
        if (buttons.empty() || doors.size() < 2) return std::nullopt;
        for (const auto& [x, y] : buttons) {
            std::unordered_set<const Wall*> targets;
            for (const auto& target : block[x][y].ButtonTargetPos()) {
                if (target.dir.has_value()) targets.insert(&block.WallAt(x + target.dx, y + target.dy, *target.dir));
            }
            if (targets != doors) return std::nullopt;
        }
        GridMap switched = block;
        const auto [bx, by] = buttons.front();
        for (const auto& target : block[bx][by].ButtonTargetPos()) {
            if (target.dir.has_value()) switched.WallAt(bx + target.dx, by + target.dy, *target.dir).SwitchDoor();
        }
        return switched;
    }

    static SpecialEvent GetRandomSpecialEvent()
    {
        static constexpr std::array events = {
            SpecialEvent::LAZYGARDENER,
            SpecialEvent::OVERGROWTH,
            SpecialEvent::RAINSTORY,
            SpecialEvent::AMBUSH,
        };
        return events[rand() % events.size()];
    }

    // 特殊事件详情
    static string ShowSpecialEvent(const SpecialEvent event)
    {
        switch (event) {
            case SpecialEvent::LAZYGARDENER: return "[特殊事件]【怠惰的园丁】树丛（浆果丛）将在其区块内随机位置生成（有可能生成在中间）";
            case SpecialEvent::OVERGROWTH:   return "[特殊事件]【营养过剩】树丛（浆果丛）和陷阱将额外向随机1个方向再次生成1个树丛（不可隔墙生长）";
            case SpecialEvent::RAINSTORY:    return "[特殊事件]【雨天小故事】地图中所有树丛变成水洼，陷阱和浆果丛会发出" PAPA_STR "声";
            case SpecialEvent::AMBUSH:       return "[特殊事件]【十面埋伏】地图中所有树丛变成陷阱，浆果丛也视为陷阱";
            default:                         return "[特殊事件]【无】";
        }
    }

    // 特殊事件1——怠惰的园丁：树丛（浆果丛）将在其区块内随机位置生成
    void SpecialEvent1()
    {
        auto MarkMaps = [](auto& maps) {
            for (auto& map : maps) {
                for (int k = 0; k < 9; ++k) {
                    int i = k / 3, j = k % 3;
                    if (map.block[i][j].Type() == GridType::GRASS || map.block[i][j].Type() == GridType::BERRY)
                        map.block[i][j].SetMark(k_move_mark);
                }
            }
        };
        auto ProcessMaps = [](auto& maps) {
            for (auto& map: maps) {
                for (int k = 0; k < 9; ++k) {
                    int i = k / 3, j = k % 3;
                    const GridType type = map.block[i][j].Type();
                    if (type == GridType::GRASS || type == GridType::BERRY) {
                        map.block[i][j].SetType(GridType::EMPTY);
                        int m;
                        do {
                            m = rand() % 9;
                        } while (map.block[m / 3][m % 3].Type() != GridType::EMPTY);
                        map.block[m / 3][m % 3].SetType(type);
                        break;
                    }
                }
            }
        };

        MarkMaps(all_maps);
        MarkMaps(all_exits);
        MarkMaps(all_special_maps);
        ProcessMaps(maps);
        ProcessMaps(exits);
        ProcessMaps(special_maps);
    }

    // [营养过剩] 区块内可生长树丛的位置：生长源（树丛/浆果丛/陷阱）周围 8 格中的空地，且两步内可达
    // 上下左右需直接相通，斜向需经任一侧相邻格转折相通；相通指两格之间的墙可直接通行（不能隔墙生长）
    static vector<pair<int, int>> GrowablePositions(const GridMap& block)
    {
        const int n = static_cast<int>(block.size());
        // (x, y) 与 d 方向相邻格是否相通
        const auto linked = [&](const int x, const int y, const int d) {
            const int nx = x + k_DX_Direct[d], ny = y + k_DY_Direct[d];
            return 0 <= nx && nx < n && 0 <= ny && ny < n && block.WallAt(x, y, static_cast<Direct>(d)).CanPass();
        };
        vector<vector<bool>> reachable(n, vector<bool>(n, false));
        for (int sx = 0; sx < n; ++sx) {
            for (int sy = 0; sy < n; ++sy) {
                const GridType type = block[sx][sy].Type();
                if (type != GridType::GRASS && type != GridType::BERRY && type != GridType::TRAP) continue;
                for (int d1 = 0; d1 < 4; ++d1) {
                    if (!linked(sx, sy, d1)) continue;
                    const int mx = sx + k_DX_Direct[d1], my = sy + k_DY_Direct[d1];
                    reachable[mx][my] = true;       // 上下左右
                    for (int d2 = 0; d2 < 4; ++d2) {
                        // 仅转折一次到达斜向格（同向两步与原路返回均不在周围 8 格内）
                        if (d2 == d1 || d2 == static_cast<int>(opposite(static_cast<Direct>(d1)))) continue;
                        if (linked(mx, my, d2)) reachable[mx + k_DX_Direct[d2]][my + k_DY_Direct[d2]] = true;
                    }
                }
            }
        }
        vector<pair<int, int>> positions;
        for (int x = 0; x < n; ++x) {
            for (int y = 0; y < n; ++y) {
                if (reachable[x][y] && block[x][y].Type() == GridType::EMPTY) positions.emplace_back(x, y);
            }
        }
        return positions;
    }

    // 特殊事件2——营养过剩：树丛（浆果丛）和陷阱将额外向随机1个方向再次生成1个树丛
    void SpecialEvent2()
    {
        auto MarkMaps = [](auto& maps) {
            for (auto& map : maps) {
                for (const auto& [i, j] : GrowablePositions(map.block)) {
                    map.block[i][j].SetMark(k_grow_mark);
                }
            }
        };
        auto ProcessMaps = [this](auto& maps) {
            for (auto& map : maps) {
                vector<pair<int, int>> growablePositions = GrowablePositions(map.block);
                int grassCount = 0;
                for (int i = 0; i < 3; ++i) {
                    for (int j = 0; j < 3; ++j) {
                        const GridType type = map.block[i][j].Type();
                        if (type == GridType::GRASS || type == GridType::TRAP || type == GridType::BERRY) {
                            grassCount++;
                        }
                    }
                }
                int targetGrowth = min(grassCount, static_cast<int>(growablePositions.size()));
                if (targetGrowth == 0) continue;    // 无法生长则跳过

                std::shuffle(growablePositions.begin(), growablePositions.end(), *g);
                for (int i = 0; i < targetGrowth; i++) {
                    int x = growablePositions[i].first;
                    int y = growablePositions[i].second;
                    map.block[x][y].SetType(GridType::GRASS);
                }
            }
        };

        MarkMaps(all_maps);
        MarkMaps(all_exits);
        MarkMaps(all_special_maps);
        ProcessMaps(maps);
        ProcessMaps(exits);
        ProcessMaps(special_maps);
    }

    // 特殊事件3——雨天小故事：地图中所有树丛变成水洼，陷阱和浆果丛会发出啪嗒声
    void SpecialEvent3()
    {
        auto ProcessMaps = [](auto& maps) {
            for (auto& map: maps) {
                for (int k = 0; k < 9; ++k) {
                    int i = k / 3, j = k % 3;
                    auto& grid = map.block[i][j];
                    if (grid.Type() == GridType::GRASS) {
                        grid.SetType(GridType::WATER);
                    } else if (grid.Type() == GridType::TRAP || grid.Type() == GridType::BERRY) {
                        grid.SetMark(k_rain_mark);
                    }
                }
            }
        };

        ProcessMaps(maps);
        ProcessMaps(exits);
        ProcessMaps(special_maps);
        ProcessMaps(all_special_maps);
    }

    // 特殊事件4——十面埋伏：地图中所有树丛变成陷阱，浆果丛也视为陷阱
    void SpecialEvent4()
    {
        auto ProcessMaps = [](auto& maps) {
            for (auto& map: maps) {
                for (int k = 0; k < 9; ++k) {
                    auto& grid = map.block[k / 3][k % 3];
                    if (grid.Type() == GridType::GRASS) {
                        grid.SetType(GridType::TRAP);
                    } else if (grid.Type() == GridType::BERRY) {
                        grid.SetMark(k_ambush_mark);
                    }
                }
            }
        };

        ProcessMaps(maps);
        ProcessMaps(exits);
        ProcessMaps(special_maps);
        ProcessMaps(all_special_maps);
    }

    // 静音模式：为所有会发出声响的地形标记静音图标
    void MarkSilenceMode()
    {
        auto IsSoundGrid = [](const GridType type) {
            return type == GridType::GRASS || type == GridType::BERRY || type == GridType::WATER ||
                   type == GridType::PORTAL || type == GridType::ONEWAYPORTAL ||
                   type == GridType::TRAP || type == GridType::HEART;
        };
        auto MarkMaps = [&](auto& maps) {
            for (auto& map : maps) {
                for (int k = 0; k < 9; ++k) {
                    int i = k / 3, j = k % 3;
                    // 静音图标覆盖特殊事件图标
                    if (IsSoundGrid(map.block[i][j].Type())) map.block[i][j].SetMark(k_silence_mark);
                }
            }
        };
        MarkMaps(all_maps);
        MarkMaps(all_exits);
        MarkMaps(all_special_maps);
        MarkMaps(maps);
        MarkMaps(exits);
        MarkMaps(special_maps);
    }

    // 传送紊乱模式：为所有传送门标记漩涡图标，去掉不再对应的字母
    void MarkPortalChaos()
    {
        auto MarkMaps = [](auto& maps) {
            for (auto& map : maps) {
                for (int k = 0; k < 9; ++k) {
                    Grid& grid = map.block[k / 3][k % 3];
                    if (grid.Type() != GridType::PORTAL) continue;
                    grid.SetMark(k_chaos_mark);
                    grid.SetContent("");
                }
            }
        };
        MarkMaps(all_maps);
        MarkMaps(all_exits);
        MarkMaps(all_special_maps);
        MarkMaps(maps);
        MarkMaps(exits);
        MarkMaps(special_maps);
        portal_chaos = true;
    }

    // 大地图区块位置随机：枚举区块互不重叠的全部布局，等概率抽取
    void RandomizeBlockPosition(const int size)
    {
        vector<vector<pair<int, int>>> layouts;
        vector<pair<int, int>> chosen;
        vector<vector<bool>> covered(size, vector<bool>(size, false));
        EnumerateLayouts(0, size * size - 81, covered, chosen, layouts);
        const auto& layout = layouts[std::uniform_int_distribution<int>(0, static_cast<int>(layouts.size()) - 1)(*g)];
        for (int i = 0; i < 9; i++) {
            pos[i] = layout[i];
        }
    }

    // 边长11：9 个区块之间隔着横竖各 2 条大走廊
    void SetMapPosition11()
    {
        pos = {
            {0, 0}, {0, 4}, {0, 8},
            {4, 0}, {4, 4}, {4, 8},
            {8, 0}, {8, 4}, {8, 8},
        };
        origin_pos = pos;
    }

    // 边长12：16 个区块
    void SetMapPosition12()
    {
        pos.insert(pos.begin() + 3, {0, 9});
        pos.insert(pos.begin() + 6, {3, 9});
        pos.push_back({6, 9});
        pos.push_back({9, 0});
        pos.push_back({9, 3});
        pos.push_back({9, 6});
        pos.push_back({9, 9});
        origin_pos = pos;
    }

    void SetMapBlock(const int x, const int y, GridMap& grid_map, const string& map_id, const SpecialEvent event) const
    {
        grid_map.PlaceBlock(x, y, FindBlockById(map_id, false, event));
    }

    void SetExitBlock(const int x, const int y, GridMap& grid_map, const string& exit_id, const SpecialEvent event) const
    {
        grid_map.PlaceBlock(x, y, FindBlockById(exit_id, true, event));
    }

  private:
    std::unique_ptr<std::mt19937> g;

    // 读取区块数据中编号前缀为 prefix 的区块（'\0' 为无前缀的普通区块）；逃生舱的 id 去掉 E 前缀，特殊区块保留 S 前缀
    static vector<Map> LoadMaps(const char prefix)
    {
        vector<Map> maps;
        for (const auto& entry : BlockLoader::All()) {
            const char key_prefix = (entry.key[0] == 'E' || entry.key[0] == 'S') ? entry.key[0] : '\0';
            if (key_prefix != prefix) continue;
            maps.push_back({entry.block, prefix == 'E' ? entry.key.substr(1) : entry.key, entry.name, entry.type, entry.attach,
                            prefix == 'E', prefix == 'S'});
        }
        return maps;
    }

    bool IsHeatMap(const Map& map) const { return map.grid == GridType::HEAT; }

    bool IsHeatBoxMap(const Map& map) const { return map.attach == AttachType::HEATBOX; }

    // 枚举布局：按行优先取第一个未确定的格子，留作空隙（共 gaps 个）或作为区块左上角
    static void EnumerateLayouts(const int cell, const int gaps, vector<vector<bool>>& covered,
                                 vector<pair<int, int>>& chosen, vector<vector<pair<int, int>>>& layouts)
    {
        if (chosen.size() == 9) {
            layouts.push_back(chosen);
            return;
        }
        const int size = covered.size();
        int c = cell;
        while (c < size * size && covered[c / size][c % size])
            c++;
        if (c == size * size)
            return;
        const int x = c / size, y = c % size;
        if (gaps > 0) {
            covered[x][y] = true;
            EnumerateLayouts(c + 1, gaps - 1, covered, chosen, layouts);
            covered[x][y] = false;
        }
        if (x + 3 > size || y + 3 > size)
            return;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (covered[x + i][y + j])
                    return;
            }
        }
        auto fill = [&](const bool value) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++)
                    covered[x + i][y + j] = value;
            }
        };
        fill(true);
        chosen.push_back({x, y});
        EnumerateLayouts(c + 1, gaps, covered, chosen, layouts);
        chosen.pop_back();
        fill(false);
    }

    static GridMap InitializeMapTemplate() { return GridMap(3, false); }
};
