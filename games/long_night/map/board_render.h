// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).
//
// Board 类渲染相关成员实现

/* ========== 盘面与区块 ========== */
// 获取地图html
inline string Board::GetBoard(const GridMap& grid_map, const GetBoardOptions& options) const
{
    size_t size = grid_map.size();
    html::Table map(size * 2 + 1, size * 2 + 1);
    map.SetTableStyle(string("align=\"center\" cellpadding=\"0\" cellspacing=\"0\"")
            + (options.with_frame ? " style=\"border: 2px dashed black;\"" : ""));
    // 方格信息（包括玩家）
    for (int x = 1; x < size * 2; x = x + 2) {
        for (int y = 1; y < size * 2; y = y + 2) {
            int gridX = (x+1)/2-1, gridY = (y+1)/2-1;
            const Grid& grid = grid_map[gridX][gridY];
            if (options.with_ground) {
                map.Get(x, y).SetStyle("class=\"grid\" " + GetGridStyle(grid.Type(), grid.Attach(), true));
            } else {
                map.Get(x, y).SetStyle("class=\"grid\" " + GetGridStyle(GridType::EMPTY, AttachType::EMPTY, true));
            }
            if (options.with_content) {
                const string& content = grid.GetContent();
                if (!content.empty()) {
                    map.Get(x, y).SetContent(GetColoredMark(content, grid.Type(), grid.Attach()));
                }
            } else if (options.with_player) {
                string player_marks;
                for (const auto& boss : bosses) {
                    if (boss->x == gridX && boss->y == gridY) player_marks += boss->GetBossIcon();
                }
                for (auto pid: player_map[gridX][gridY]) {
                    player_marks += num[pid];
                    if (player_marks.length() % 2 == 0) {
                        player_marks += "<br>";
                    }
                }
                // 根据地形改变玩家标记颜色
                if (!player_marks.empty()) {
                    map.Get(x, y).SetContent(GetColoredMark(player_marks, grid.Type(), grid.Attach()));
                }
            }
            // 调试：测试地形文字颜色
            // map.Get(x, y).SetContent(GetColoredMark(string(num[0]), grid.Type(), grid.Attach()));
        }
    }
    // 绘制一面墙（含附着与标注）
    const auto DrawWall = [&](const int x, const int y, const Wall& wall, const string& direction) {
        map.Get(x, y).SetStyle("class=\"wall-" + direction + "\" style=\"" + GetWallStyle(wall, direction) + "\"");
        if (options.with_content && !wall.Label().empty()) map.Get(x, y).SetContent(wall.Label());
    };
    // 纵向围墙
    for (int x = 1; x < size * 2; x = x + 2) {
        for (int y = 0; y < size * 2 - 1; y = y + 2) {
            int gridX = (x+1)/2-1, gridY = y/2;
            DrawWall(x, y, grid_map.WallAt(gridX, gridY, Direct::LEFT), "col");
        }
        DrawWall(x, size*2, grid_map.WallAt((x+1)/2-1, size-1, Direct::RIGHT), "col");
    }
    // 横向围墙
    for (int y = 1; y < size * 2; y = y + 2) {
        for (int x = 0; x < size * 2 - 1; x = x + 2) {
            int gridX = x/2, gridY = (y+1)/2-1;
            DrawWall(x, y, grid_map.WallAt(gridX, gridY, Direct::UP), "row");
        }
        DrawWall(size*2, y, grid_map.WallAt(size-1, (y+1)/2-1, Direct::DOWN), "row");
    }
    // 角落方块
    for (int x = 0; x < size * 2 + 1; x = x + 2) {
        for (int y = 0; y < size * 2 + 1; y = y + 2) {
            map.Get(x, y).SetStyle("class=\"corner\" style=\"background-image: url('" + ToFileUrl(image_path_ + GetImageTypeFolder() + "walls/corner.png") + "');\"");
        }
    }
    return GetBoardStyle() + map.ToString();
}

// 获取终局盘面
inline string Board::GetFinalBoard() const
{
    html::Table finalTable(2, 2);
    finalTable.SetTableStyle("align=\"center\" cellpadding=\"0\" cellspacing=\"0\"");
    finalTable.Get(0, 0).SetContent(HTML_SIZE_FONT_HEADER(5) "<b>初始地图</b>" HTML_FONT_TAIL);
    finalTable.Get(0, 1).SetContent(HTML_SIZE_FONT_HEADER(5) "<b>终局地图</b>" HTML_FONT_TAIL);
    finalTable.Get(1, 0).SetStyle("style=\"padding: 10px 20px 10px 10px\"").SetContent(init_html_);
    finalTable.Get(1, 1).SetStyle("style=\"padding: 10px 10px 10px 20px\"").SetContent(GetBoard(grid_map));
    return GetPlayerTable(-1) + GetBoardStyle() + finalTable.ToString();
}

// 全部区块信息展示
inline string Board::GetAllBlocksInfo(const SpecialEvent event, const bool bomb_mode, const optional<BlockMode> test_block_mode) const
{
    const BlockMode test_mode = test_block_mode.value_or(BlockMode::CLASSIC);

    string title = "";
    int col_num = (test_mode == BlockMode::CRAZY) ? 8 : 4;
    const auto& maps = [&]() -> const std::vector<UnitMaps::Map>& {
        if (test_mode == BlockMode::CLASSIC) return unitMaps.maps;
        if (test_mode == BlockMode::CRAZY)   return unitMaps.all_maps;
        return unitMaps.pool_maps;
    }();
    const auto& exits = [&]() -> const std::vector<UnitMaps::Map>& {
        if (test_mode == BlockMode::CLASSIC) return unitMaps.exits;
        if (test_mode == BlockMode::CRAZY)   return unitMaps.all_exits;
        return unitMaps.pool_exits;
    }();
    const auto& all_special_maps = unitMaps.all_special_maps;

    // 区块池预览标题：仅在指定 test_block_mode 时展示，供 tool_pool_exporter 导出完整区块池
    if (test_block_mode.has_value()) {
        switch (*test_block_mode) {
            case BlockMode::CLASSIC: title = HTML_SIZE_FONT_HEADER(6) "<b>《漫漫长夜》「经典」模式区块</b>" HTML_FONT_TAIL; break;
            case BlockMode::TWIST:   title = HTML_SIZE_FONT_HEADER(6) "<b>《漫漫长夜》「幻变」模式轮换区块</b>" HTML_FONT_TAIL; break;
            case BlockMode::CRAZY:   title = HTML_SIZE_FONT_HEADER(6) "<b>《漫漫长夜》「狂野」+「疯狂」模式全部区块</b>" HTML_FONT_TAIL; break;
            case BlockMode::BUTTON:  title = HTML_SIZE_FONT_HEADER(6) "<b>《漫漫长夜》「按钮」模式主题区块</b>" HTML_FONT_TAIL; break;
            case BlockMode::TRAP:    title = HTML_SIZE_FONT_HEADER(6) "<b>《漫漫长夜》「陷阱」模式主题区块</b>" HTML_FONT_TAIL; break;
            case BlockMode::OPEN:    title = HTML_SIZE_FONT_HEADER(6) "<b>《漫漫长夜》「空旷」模式主题区块</b>" HTML_FONT_TAIL; break;
            case BlockMode::SILENT:  title = HTML_SIZE_FONT_HEADER(6) "<b>《漫漫长夜》「无声」模式主题区块</b>" HTML_FONT_TAIL; break;
            default:;
        }
    }

    bool has_special = (test_mode == BlockMode::CRAZY);
    for (const auto& map : maps) {
        if (map.is_special) {
            has_special = true;
        }
    }
    SpecialEvent rain_event = event == SpecialEvent::RAINSTORY ? SpecialEvent::RAINSTORY : SpecialEvent::NONE;

    int line_num = (ceil(maps.size() / (double) col_num) + ceil(exits.size() / (double) col_num)) * 2 +  + has_special * (ceil(all_special_maps.size() / (double) col_num) * 2 + 1);
    html::Table blocks(line_num, col_num);
    blocks.SetTableStyle("align=\"center\" cellpadding=\"0\" cellspacing=\"0\" style=\"font-size: 25px; line-height: 1.1;\"");
    int row = 0;
    for (int i = 0; i < maps.size(); i++) {
        blocks.Get(row, i % col_num).SetStyle("style=\"padding: 25px 25px 5px 25px\"").SetContent(GetSingleBlock(maps[i].id, false, rain_event));
        // [疯狂模式] 特殊地图不显示id
        if (maps[i].is_special && block_mode == BlockMode::CRAZY) {
            blocks.Get(row + 1, i % col_num).SetContent(HTML_COLOR_FONT_HEADER(red) HTML_SIZE_FONT_HEADER(6) "<b>？？？</b>" HTML_FONT_TAIL HTML_FONT_TAIL);
        } else {
            blocks.Get(row + 1, i % col_num).SetContent(GetMapTitleContent(maps[i].title, maps[i].id, false));
        }
        if ((i + 1) % col_num == 0 || i == maps.size() - 1) row += 2;
    }
    for (int i = 0; i < exits.size(); i++) {
        blocks.Get(row, i % col_num).SetStyle("style=\"padding: 20px 25px 5px 25px\"").SetContent(GetSingleBlock(exits[i].id, true, rain_event));
        blocks.Get(row + 1, i % col_num).SetContent(GetMapTitleContent(exits[i].title, exits[i].id, true));
        if ((i + 1) % col_num == 0 || i == exits.size() - 1) row += 2;
    }
    if (has_special && block_mode == BlockMode::CRAZY) {
        blocks.MergeRight(row, 0, col_num);
        blocks.Get(row++, 0).SetStyle("style=\"padding: 20px 25px 5px 25px\"")
            .SetContent(HTML_SIZE_FONT_HEADER(6) "<b>特殊区块列表<br>" HTML_FONT_TAIL HTML_SIZE_FONT_HEADER(5) "（多传送门按照图中字母代号传送）" HTML_FONT_TAIL "</b>");
        for (int i = 0; i < all_special_maps.size(); i++) {
            blocks.Get(row, i % col_num).SetStyle("style=\"padding: 20px 25px 5px 25px\"")
                .SetContent(GetBoard(unitMaps.FindBlockById(all_special_maps[i].id, false, SpecialEvent::NONE), GetBoardOptions{.with_content = true}));
            blocks.Get(row + 1, i % col_num).SetContent(GetMapTitleContent(all_special_maps[i].title, all_special_maps[i].id, false));
            if ((i + 1) % col_num == 0 || i == all_special_maps.size() - 1) row += 2;
        }
    }

    const vector<pair<WallType, string>> all_walls_info = {
        { WallType::DOOR, "【门】初始关闭状态的门，关闭时视为墙壁。当关联的按钮被按下时会切换开关状态<br><b>如果门发生过变化，在回合结束会进行提示。</b>关闭的门在私信墙壁信息会显示为**普通墙壁**" },
        { WallType::DOOROPEN, "【门 (开)】初始打开状态的门，玩家可移动穿过打开的门。在私信墙壁信息会显示为**无墙壁**" },
        { WallType::HEDGE, "【树篱】若**上一步移动与本步在同一回合内且方向一致**，则冲刺穿过树篱；否则视为**撞墙**。<br>树篱在私信墙壁信息会显示为**无墙壁**。**箱子不可穿过树篱**。<br>玩家出生或被传送在树篱旁，不能立即穿过树篱（亚空间传送后行动方向需一致）" },
    };
    const vector<pair<AttachType, string>> all_attachs_info = {
        { AttachType::BUTTON, "【按钮】玩家进入时会触发区块内按钮相关事件。（出生不算）<br>进入按钮格**没有任何额外信息提示**，且仅在进入时才会触发按钮" },
        { AttachType::BOMB, "【炸弹】玩家**经过并尝试离开**会引爆炸弹，使玩家**立即出局并-100分**<br>在炸弹上**结束行动**可拆除炸弹，放置后直接停止不会拆除。<font color=red>**炸弹不能放置在其他附着类型上**</font>" },
        { AttachType::BOX, "【箱子】玩家相邻箱子且向箱子移动时，箱子可被推动。（不会出生在箱子内）<br>**箱子不可移动到本区块外，其后方有玩家、墙壁或其他附着时，均不可被推动**。<br>若箱子不可被推动，则视为**撞墙**，且**箱子本身不会显示为墙壁**。" },
        { AttachType::HEATBOX, "【小太阳】有 5x5 **热浪范围**的箱子，基础推动规则和**箱子**一致。（不会出生在小太阳内）<br>玩家进入小太阳周围 5x5 范围时，将**私信**收到热浪提示。<br><font color=red>小太阳和其他热源区块**不会同时出现在地图中**</font>（自定义模式区块不足时除外）" },
        { AttachType::JAMMERBOX, "【屏蔽器】有 3x3 **寂静范围**的箱子，基础推动规则和**箱子**一致。（不会出生在屏蔽器内）<br>玩家位于屏蔽器周围 3x3 范围内时，<font color=teal>**将无法获知所有声响的方向来源**</font>。" },
        { AttachType::COIN, "【金币】玩家经过时**静默拾取**该枚金币（金币移除），每枚金币 <font color=#e0b400>**+30分**</font>。<br>本回合有玩家获得金币时，将在**回合结束时指名公开**获得数量。<br><font color=red>**捕捉成功可抢夺目标持有的全部金币**</font>（首轮捕捉也可抢夺）" },
    };
    const vector<pair<GridType, string>> all_grids_info = {
        { GridType::GRASS, "【树丛】玩家进入时会发出让其他人听见的<font color=#00af50>**" SHASHA_STR "声**</font>。（出生不算）" },
        { GridType::WATER, "【水洼】玩家进入时会发出让其他人听见的<font color=#01b0f1>**" PAPA_STR "声**</font>。（出生不算）" },
        { GridType::PORTAL, "【传送门】玩家进入时会发出其他人听见的<font color=#01b0f1>**" PAPA_STR "声**</font>。（出生不算）<br>进入后，再任意 <font color=purple>**2次**</font> 移动后就会传送至同区块另一个传送门。<br>进入后，玩家视作进入亚空间，上述 <font color=purple>**2次**</font> 移动都在亚空间内。" },
        { GridType::ONEWAYPORTAL, "【传送门出口】玩家进入时会发出其他人听见的<font color=#01b0f1>**" PAPA_STR "声**</font>。（出生不算）<br>传送门的单向出口，进入时不会触发传送（必须从入口进入才会传送至此处）<br>**玩家在进入同一区块的传送门入口时，传送门会转换方向，入口和出口交换位置**" },
        { GridType::TRAP, "【陷阱】陷阱隐藏在树丛中：被奇数次进入时，会发出让其他人听见的<font color=#00af50>**" SHASHA_STR "声**</font>（出生不算）<br>被偶数次进入时，不发出声响，并**强制玩家停止**（出生不算）" },
        { GridType::HEAT, "【热源】进入热源周围 8 格时，将**私信**收到热浪提示。（只有移动时才能感受到热浪）<br>当进入热源时，将**私信**收到高温烫伤提示（不会出生在热源内）<br><font color=red>在整局游戏中，**当第 2 次或更多次进入热源时，会被强制停止行动**</font>" },
        { GridType::BERRY, "【浆果丛】玩家进入时会发出让其他人听见的<font color=#00af50>**" SHASHA_STR "声**</font>，公开为树丛。（出生不算）<br>只有进入的玩家会**私信**得知这里是浆果丛。" },
        { GridType::HEART, "【巨大的心脏】地图中存在心脏时，每位玩家行动的<font color=#c00000>**第 1/4/7/10/13/16/19 步**</font>会发出全图无方向的" PENGPENG_STR "声（每回合行动最多 7 次）。<br><font color=#c00000>**" PENGPENG_STR "声会完全掩盖同一步发出的其他声响**</font>（他人不会收到方向私信，不显示声响内容）。" },
        { GridType::EXIT, "【逃生舱】逃生者使用后，**会消失**。" + (!test_block_mode.has_value() ? ("本局逃生舱数量为 **" + to_string(exit_num) + "** 个。") : "") },
    };

    auto GenerateWallStyle = [&](WallType type, const string& direction) {
        return "<div class='wall-row' style=\""
            + GetWallStyle(Wall(type), direction)
            + "\"></div>";
    };
    vector<UnitMaps::Map> all_maps_in_game;
    all_maps_in_game.insert(all_maps_in_game.end(), maps.begin(), maps.end());
    all_maps_in_game.insert(all_maps_in_game.end(), exits.begin(), exits.end());
    if (has_special && block_mode == BlockMode::CRAZY) all_maps_in_game.insert(all_maps_in_game.end(), all_special_maps.begin(), all_special_maps.end());

    int legend_max_size = all_walls_info.size() + all_attachs_info.size() + all_grids_info.size() + 1;
    html::Table legend(legend_max_size, 2);
    legend.SetTableStyle("cellpadding=\"5\" cellspacing=\"0\" style=\"font-size: 22px;\"");
    for (int i = 0; i < legend_max_size; i++) {
        legend.Get(i, 1).SetStyle("style=\"text-align: left;\"");
    }
    const string wall_html =
        "<div style=\"width:" + to_string(GRID_SIZE) + "px;\">" +
            GenerateWallStyle(WallType::NORMAL, "row") +
            GetGridStyle(GridType::EMPTY, AttachType::EMPTY, false) +
            GenerateWallStyle(WallType::NORMAL, "row") +
        "</div>";
    legend.Get(0, 0).SetContent(wall_html);
    legend.Get(0, 1).SetContent("【墙壁】如图例，上下为墙壁，玩家不可从上下通过，可以从左右通过");
    row = 1;
    for (const auto& wall_info: all_walls_info) {
        if (UnitMaps::MapContainWallType(all_maps_in_game, wall_info.first)) {
            legend.Get(row, 0).SetContent(GenerateWallStyle(wall_info.first, "row"));
            legend.Get(row, 1).SetContent(wall_info.second);
            row++;
        }
    }
    for (const auto& attach_info: all_attachs_info) {
        if (UnitMaps::MapContainAttachType(all_maps_in_game, attach_info.first) || (attach_info.first == AttachType::BOMB && bomb_mode)) {
            legend.Get(row, 0).SetContent(GetGridStyle(GridType::EMPTY, attach_info.first, false));
            legend.Get(row, 1).SetContent(attach_info.second);
            row++;
        }
    }
    for (const auto& grid_info: all_grids_info) {
        if (UnitMaps::MapContainGridType(all_maps_in_game, grid_info.first)) {
            if (grid_info.first == GridType::GRASS && event == SpecialEvent::RAINSTORY) continue;
            legend.Get(row, 0).SetContent(GetGridStyle(grid_info.first, AttachType::EMPTY, false));
            legend.Get(row, 1).SetContent(grid_info.second);
            row++;
        }
    }
    legend.ResizeRow(row);
    return title + blocks.ToString() + legend.ToString();
}

inline string Board::GetMapTitleContent(const string& title, const string& id, const bool is_exit) const
{
    return "<b>" HTML_COLOR_FONT_HEADER(#990000) HTML_SIZE_FONT_HEADER(4) + title + HTML_FONT_TAIL HTML_FONT_TAIL
        "<br>" HTML_COLOR_FONT_HEADER(red) + (is_exit ? "EXIT " : "") + id + HTML_FONT_TAIL "</b>";
}

inline string Board::GetSingleBlock(const string& id, const bool is_exit, const SpecialEvent event) const
{
    // [疯狂模式] 特殊地图不显示预览
    if (id[0] == 'S' && block_mode == BlockMode::CRAZY) {
        string size = to_string(GRID_SIZE * 3 + WALL_SIZE * 4) + "px";
        return "<div style=\"width:" + size + "; height:" + size + "; background:#e0e0e0;"
            "border:2px dashed black; font-size:30px; color:#333;"
            "display:flex; align-items:center; justify-content:center;\">"
            "<b>特殊区块</b></div>";
    }
    return GetBoard(unitMaps.FindBlockById(id, is_exit, event), GetBoardOptions{.with_content = true});
}

// 单区块裸图（工具用）
inline string Board::GetSingleBlockNoBorder(const string& id, const bool is_exit, const SpecialEvent event) const
{
    return GetBoard(unitMaps.FindBlockById(id, is_exit, event),
            GetBoardOptions{.with_player = false, .with_content = true, .with_frame = false});
}

// 地形图片完整路径（带纹理）
inline string Board::GetGridImagePath(const GridType type) const
{
    return image_path_ + GetImageTypeFolder() + GetGridImage(type);
}

// 指定区块生成地图预览
inline string Board::MapPreview(const vector<string>& map_str) const
{
    GridMap grid_map(size, true);
    for (int i = 0; i < unitMaps.origin_pos.size(); i++) {
        string map_id;
        bool is_exit = false;
        string str = map_str[i];
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
        if (!str.empty() && str[0] == 'E') {
            is_exit = true;
            map_id = str.substr(1);
        } else {
            map_id = str;
        }
        if (is_exit) {
            unitMaps.SetExitBlock(unitMaps.origin_pos[i].first, unitMaps.origin_pos[i].second, grid_map, map_id, SpecialEvent::NONE);   // 预览时需取用原始地图
        } else {
            unitMaps.SetMapBlock(unitMaps.origin_pos[i].first, unitMaps.origin_pos[i].second, grid_map, map_id, SpecialEvent::NONE);    // 预览时需取用原始地图
        }
    }
    FixInvalidPortals(grid_map);
    return GetBoard(grid_map, GetBoardOptions{.with_content = true});
}


/* ========== 玩家与赛况 ========== */
// 获取玩家信息
inline string Board::GetPlayerTable(const int round) const
{
    html::Table playerTable(playerNum + bosses.size(), 6);
    playerTable.SetTableStyle("align=\"center\" cellpadding=\"2\"");
    for (int pid = 0; pid < playerNum; pid++) {
        playerTable.Get(pid, 0).SetStyle("style=\"width:60px; text-align:right;\"").SetContent(to_string(pid) + "号：");
        playerTable.Get(pid, 1).SetStyle("style=\"width:40px;\"").SetContent(players[pid].avatar);
        playerTable.Get(pid, 2).SetStyle("style=\"width:250px; text-align:left;\"").SetContent(players[pid].name);
        if (players[pid].out == 2) {
            playerTable.MergeRight(pid, 3, 3);
            playerTable.Get(pid, 3).SetStyle("style=\"width:120px;\"").SetColor("#FFEBA3").SetContent("【逃生舱撤离】");
        } else if (players[pid].out == 1) {
            playerTable.MergeRight(pid, 3, 3);
            playerTable.Get(pid, 3).SetStyle("style=\"width:120px;\"").SetColor("#E5E5E5").SetContent("【已出局】");
        } else if (players[pid].target == 100) {
            playerTable.MergeRight(pid, 3, 3);
            playerTable.Get(pid, 3).SetStyle("style=\"width:130px;\"").SetContent("【单机模式】<br>寻找唯一逃生舱！");
        } else if (players[pid].out == 0) {
            playerTable.Get(pid, 3).SetStyle("style=\"width:40px;\"").SetContent("捕捉<br>目标");
            playerTable.Get(pid, 4).SetStyle("style=\"width:40px;\"").SetContent("[" + to_string(players[pid].target) + "号]");
            playerTable.Get(pid, 5).SetStyle("style=\"width:40px;\"").SetContent(players[players[pid].target].avatar);
        } else {
            playerTable.MergeRight(pid, 3, 3);
            playerTable.Get(pid, 3).SetStyle("style=\"width:120px;\"").SetColor("#FFA07A").SetContent("[玩家状态错误]");
        }
    }
    for (size_t i = 0; i < bosses.size(); i++) {
        const Boss& boss = *bosses[i];
        const int row = playerNum + i;
        playerTable.Get(row, 0).SetStyle("style=\"width:60px;\"").SetContent("<font size=\"5\">" + boss.GetBossIcon() + "</font>");
        playerTable.MergeRight(row, 1, 2);
        playerTable.Get(row, 1).SetStyle("style=\"width:290px;\"").SetColor("lavender").SetContent("[BOSS] " + boss.GetBossName());
        playerTable.Get(row, 3).SetStyle("style=\"width:40px;\"").SetContent("追击<br>目标");
        playerTable.Get(row, 4).SetStyle("style=\"width:40px;\"").SetContent("[" + to_string(boss.target) + "号]");
        playerTable.Get(row, 5).SetStyle("style=\"width:40px;\"").SetContent(players[boss.target].avatar);
    }
    return (round > 0 ? "### 第 " + to_string(round) + " 回合" : "") + playerTable.ToString();
}

inline string Board::GetPlayerString() const
{
    string players_string;
    for (int pid = 0; pid < playerNum; pid++) {
        if (pid > 0) players_string += "\n";
        players_string += "[" + to_string(pid) + "号]" + players[pid].name;
        if (players[pid].out == 2) players_string += "【逃生舱撤离】";
        else if (players[pid].out == 1) players_string += "【已出局】";
        else if (players[pid].target == 100) players_string += "【单机模式】";
        else if (players[pid].out == 0) players_string += "\n目标→ [" + to_string(players[pid].target) + "号]" + players[players[pid].target].name;
        else players_string += "[玩家状态错误]";
    }
    for (const auto& boss : bosses) {
        players_string += "\n[BOSS] " + boss->GetBossName() + "\n";
        players_string += "目标→ [" + to_string(boss->target) + "号]" + players[boss->target].name;
    }
    return players_string;
}

// 完整赛况
inline string Board::GetAllRecordHtml(const int query_pid, const int is_public) const
{
    const char* style = R"(
<style>
    .record {}
    .player {
        padding: 8px;
        margin-bottom: 8px;
        border-left: 4px solid #4CAF50;
        background: #F9F9F9;
    }
    .player.out {
        opacity: 0.6;
        border-left-color: #E5E5E5;
        background: #F9F9F9;
    }
    .player.exit {
        opacity: 0.6;
        border-left-color: #FFE06F;
        background: #FFF3E0;
    }
    .player-title {
        font-size: 16px;
    }
    .coin-badge {
        display: inline-block;
        margin-left: 6px;
        padding: 1px 7px;
        font-size: 14px;
        font-weight: bold;
        color: #7a5c00;
        border: 1px solid #e0b400;
        border-radius: 9px;
        background: linear-gradient(135deg, #fff7b0, #ffe75e 45%, #f0d000);
        box-shadow: 0 1px 2px rgba(224, 180, 0, 0.5);
    }
    .player-record {}
    .boss {
        margin-top: 12px;
        padding: 8px;
        background: #FFF3E0;
        border-left: 4px solid #FF9800;
    }
    .extra {
        margin-top: 12px;
        padding: 8px;
        background: #E3F2FD;
        border-left: 4px solid #2196F3;
    }
</style>
)";
    string record_html;
    record_html += "<div class='record'>";
    // 玩家记录
    for (int pid = 0; pid < playerNum; pid++) {
        record_html += "<div class='player";
        if (players[pid].out == 1) record_html += " out";
        if (players[pid].out == 2) record_html += " exit";
        record_html += "'>";

        record_html += "<div class='player-title'>";
        record_html += "<b>[" + to_string(pid) + "号]" + esc_html(players[pid].name) + "</b>";
        // [金币] 在玩家名称右侧显示金色徽章（仅显示已公示数量，本回合新获得需等回合结束公示）
        if (players[pid].coins_public > 0) {
            record_html += "<span class='coin-badge'>🪙 " + to_string(players[pid].coins_public) + "</span>";
        }
        record_html += "</div>";

        record_html += "<div class='player-record'>";
        record_html += players[pid].GetAllMoveRecord(query_pid, is_public);
        record_html += "</div>";

        record_html += "</div>";
    }
    // BOSS 记录
    for (const auto& boss : bosses) {
        record_html += "<div class='boss'>";
        record_html += "<div class='boss-title'><b>[BOSS] " + boss->GetBossName() + " " + boss->GetBossIcon() + "</b></div>";
        record_html += "<div class='boss-record'>";
        record_html += boss->GetBossRecord(query_pid, is_public);
        record_html += "</div>";
        record_html += "</div>";
    }
    // 额外信息
    if (!all_extra_record.empty()) {
        record_html += "<div class='extra'>";
        record_html += "<div class='extra-title'><b>[额外信息]</b></div>";
        record_html += "<div class='extra-record'>";
        record_html += all_extra_record;
        record_html += "</div>";
        record_html += "</div>";
    }
    record_html += "</div>";
    return style + record_html;
}

inline string Board::GetAllRecordString(const int query_pid, const int is_public) const
{
    string record_string;
    // 玩家记录
    for (int pid = 0; pid < playerNum; pid++) {
        record_string += "[" + to_string(pid) + "号]" + players[pid].name;
        if (players[pid].coins_public > 0) record_string += "[🪙" + to_string(players[pid].coins_public) + "]";
        if (players[pid].out == 1) record_string += "【已出局】";
        if (players[pid].out == 2) record_string += "【逃生舱撤离】";
        record_string += "\n" + players[pid].GetAllMoveRecord(query_pid, is_public, false) + "\n";
    }
    // BOSS 记录
    for (const auto& boss : bosses) {
        record_string += "[BOSS] " + boss->GetBossName() + " " + boss->GetBossIcon();
        record_string += boss->GetBossRecord(query_pid, is_public, false) + "\n";
    }
    // 额外信息
    if (!all_extra_record.empty()) {
        record_string += "[额外信息]";
        record_string += replace_br_with_line(all_extra_record) + "\n";
    }
    return record_string;
}

inline string Board::GetAllScore() const
{
    html::Table scoreTable(playerNum + 1, 8);
    scoreTable.SetTableStyle("cellpadding=\"2\" cellspacing=\"0\" border=\"1\"");
    scoreTable.Get(0, 0).SetStyle("style=\"width:50px;\"").SetContent("序号");
    scoreTable.Get(0, 1).SetStyle("style=\"width:250px;\"").SetContent("玩家");
    scoreTable.Get(0, 2).SetStyle("style=\"width:70px;\"").SetContent("抓人分");
    scoreTable.Get(0, 3).SetStyle("style=\"width:70px;\"").SetContent("逃生分");
    scoreTable.Get(0, 4).SetStyle("style=\"width:70px;\"").SetContent("金币分");
    scoreTable.Get(0, 5).SetStyle("style=\"width:70px;\"").SetContent("探索分");
    scoreTable.Get(0, 6).SetStyle("style=\"width:70px;\"").SetContent("额外<br>探索分");
    scoreTable.Get(0, 7).SetStyle("style=\"width:100px;\"").SetContent("【最终总分】");
    for (int pid = 0; pid < playerNum; pid++) {
        Score s = players[pid].score;
        scoreTable.Get(pid + 1, 0).SetContent(to_string(pid) + "号");
        scoreTable.Get(pid + 1, 1).SetContent(players[pid].name);
        if (s.catch_score > 0) scoreTable.Get(pid + 1, 2).SetColor("#BAFFA8");
        if (s.catch_score < 0) scoreTable.Get(pid + 1, 2).SetColor("#FFA07A");
        scoreTable.Get(pid + 1, 2).SetContent(to_string(s.catch_score));
        if (s.exit_score > 0) scoreTable.Get(pid + 1, 3).SetColor("#FFDB60");
        scoreTable.Get(pid + 1, 3).SetContent(to_string(s.exit_score));
        if (s.coin_score > 0) scoreTable.Get(pid + 1, 4).SetColor("#FFF3B0");
        scoreTable.Get(pid + 1, 4).SetContent(to_string(s.coin_score));
        auto [c1, c2] = s.ExploreCount();
        scoreTable.Get(pid + 1, 5).SetContent(to_string(c1 + c2));
        scoreTable.Get(pid + 1, 6).SetContent(to_string(c2));
        scoreTable.Get(pid + 1, 7).SetContent((s.quit_score < 0 ? HTML_COLOR_FONT_HEADER("red") : HTML_COLOR_FONT_HEADER("black")) + to_string(s.FinalScore()) + HTML_FONT_TAIL);
    }
    return Score::ScoreInfo() + scoreTable.ToString();
}


/* ========== 墙壁信息 ========== */
// 获取四周墙壁信息
inline pair<string, string> Board::GetSurroundingWalls(const lgtbot::PlayerID pid) const
{
    const int x = players[pid].x, y = players[pid].y;
    GridMap view(1, false);
    view[0][0] = grid_map[x][y];
    string info;
    for (int d = 0; d < 4; ++d) {
        const Direct dir = static_cast<Direct>(d);
        // 亚空间中四周均为空墙；特殊墙壁按私信规则伪装
        Wall& wall = view.WallAt(0, 0, dir);
        wall = players[pid].InSubspace() ? Wall() : grid_map.WallAt(x, y, dir).Hidden();
        info += wall.CanPass() ? "空" : "墙";
    }
    return make_pair(info, GetBoard(view, GetBoardOptions{.with_player = false, .with_ground = false}));
}

// [BOSS-邦邦] 周围墙壁信息
inline pair<string, string> Board::GetBangBangSurroundingWalls(const int x, const int y) const
{
    GridMap view(1, false);
    view[0][0] = grid_map[x][y];
    view[0][0].SetType(GridType::EMPTY).SetAttach(AttachType::BOMB);
    string info;
    for (int d = 0; d < 4; ++d) {
        const Direct dir = static_cast<Direct>(d);
        Wall& wall = view.WallAt(0, 0, dir);
        wall = grid_map.WallAt(x, y, dir).Hidden();
        info += wall.CanPass() ? "空" : "墙";
    }
    return make_pair(info, GetBoard(view, GetBoardOptions{.with_player = false}));
}


/* ========== 样式与图片资源 ========== */

inline string Board::GetImageTypeFolder() const
{
    switch (image_texture_) {
        case Texture::CLASSIC:  return "classic/";
        case Texture::RETRO:    return "retro/";
        case Texture::FRESH:    return "fresh/";
        default: return "";
    }
}

inline string Board::GetGridStyle(const GridType type, const AttachType attach, const bool is_bg) const
{
    string bg_image = ToFileUrl(image_path_ + GetImageTypeFolder() + GetGridImage(type));
    const string attach_file = GetAttachImage(attach);
    const string attach_image = attach_file.empty() ? "" : ToFileUrl(image_path_ + GetImageTypeFolder() + attach_file);

    if (is_bg) {
        if (attach_image.empty()) {
            return "style=\""
                "background-image: url('" + bg_image + "');"
                "background-repeat: no-repeat;"
                "background-position: center;"
                "background-size: cover;"
                "\"";
        }
        return "style=\""
            "background-image: url('" + attach_image + "'), url('" + bg_image + "');"
            "background-repeat: no-repeat, no-repeat;"
            "background-position: center, center;"
            "background-size: contain, cover;"
            "\"";
    } else {
        string html = "<div style=\"position: relative; width: 50px; height: 50px;\">"
            "<img src='" + bg_image + "' style='width:100%; height:100%;'>";
        if (!attach_image.empty()) {
            html += "<img src='" + attach_image + "' style='position:absolute; top:0; left:0; width:100%; height:100%;'>";
        }
        return html + "</div>";
    }
}

inline string Board::GetWallStyle(const Wall& wall, const string& direction) const
{
    const string folder = GetImageTypeFolder() + "walls/";
    string wall_image = ToFileUrl(image_path_ + folder + GetWallImage(wall.Type(), direction));
    const string attach_image = GetWallAttachImage(wall.Attach(), direction);
    if (attach_image.empty()) {
        return "background-image: url('" + wall_image + "');";
    }
    return "background-image: url('" + ToFileUrl(image_path_ + folder + attach_image) + "'), url('" + wall_image + "');";
}

inline string Board::GetColoredMark(const string& content, const GridType type, const AttachType attach)
{
    string color;

    switch (type) {
        case GridType::PORTAL:  color = "#FFFF00"; break;
        case GridType::ONEWAYPORTAL:  color = "#FFF8E7"; break;
        // case GridType::HEAT:    color = "#00FFFF"; break;
        default:;
    }
    switch (attach) {
        case AttachType::BOMB:      color = "#FFFF00"; break;
        case AttachType::BOX:       color = "#FFFF00"; break;
        case AttachType::HEATBOX:   color = "#FFFF00"; break;
        case AttachType::JAMMERBOX: color = "#FFFF00"; break;
        case AttachType::COIN:      color = "#FF6EC7"; break;
        default:;
    }

    if (color.empty()) return "<b>" + content + "</b>";
    return "<font color=\"" + color + "\"><b>" + content + "</b></font>";
}

inline string Board::GetBoardStyle()
{
    return R"(
<style>
    .grid {
        width: )" + to_string(GRID_SIZE) + R"(px;
        height: )" + to_string(GRID_SIZE) + R"(px;
        font-size: 19px;
        line-height: 1;
        background-size: cover;
        background-repeat: no-repeat;
    }
    .wall-row {
        width: )" + to_string(GRID_SIZE) + R"(px;
        height: )" + to_string(WALL_SIZE) + R"(px;
        font-size: 8px;
        line-height: 8px;
        background-size: cover;
        background-repeat: no-repeat;
    }
    .wall-col {
        width: )" + to_string(WALL_SIZE) + R"(px;
        height: )" + to_string(GRID_SIZE) + R"(px;
        font-size: 8px;
        line-height: 8px;
        background-size: cover;
        background-repeat: no-repeat;
    }
    .corner {
        width: )" + to_string(WALL_SIZE) + R"(px;
        height: )" + to_string(WALL_SIZE) + R"(px;
    }
</style>
)";
}
