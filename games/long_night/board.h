
class Board
{
  public:
    Board(const int32_t mode) : unitMaps(mode) {}

	// 玩家
    uint32_t playerNum;
    vector<Player> players;
    // 地图内的玩家
    vector<vector<vector<PlayerID>>> player_map;
    // 地图大小
    int size = 9;
    // 地图
    vector<vector<Grid>> grid_map;
    int exit_num;  // 剩余逃生舱
    int exited = 0; // 已逃生数量
    string init_html_;
    // 区块模板
    UnitMaps unitMaps;
    // BOSS
    struct Boss {
        int x = -1;
        int y = -1;
        int steps = -1;
        PlayerID target;
        string all_record;
    } boss;
    // 特殊区块位置（暂不使用）
    // pair<int, int> special_pos = {12, 12};

    // 初始化地图
    void Initialize(const bool boss)
    {
        std::random_device rd;
        g = std::mt19937(rd());
        grid_map.resize(size);
        player_map.resize(size);
        for (int i = 0; i < size; i++) {
            grid_map[i].resize(size);
            player_map[i].resize(size);
        }
        InitializeBlock();              // 初始化区块
        FixAdjacentWalls(grid_map);     // 相邻墙面修复
        FixInvalidPortals(grid_map);    // 封闭传送门替换为水洼
        RandomizePlayers();             // 随机生成玩家
        if (boss) BossSpawn();          // 初始化BOSS
        // 保存初始盘面
        init_html_ = GetBoard(grid_map);
    }

    // 获取地图html
    string GetBoard(const vector<vector<Grid>>& grid_map, const bool with_player = true) const
    {
        size_t size = grid_map.size();
        html::Table map(size * 2 + 1, size * 2 + 1);
        map.SetTableStyle("align=\"center\" cellpadding=\"0\" cellspacing=\"0\" style=\"border: 2px dashed black;\"");
        // 方格信息（包括玩家）
        for (int x = 1; x < size * 2; x = x + 2) {
            for (int y = 1; y < size * 2; y = y + 2) {
                int gridX = (x+1)/2-1;
                int gridY = (y+1)/2-1;
                if (size >= 3) {
                    map.Get(x, y).SetStyle("class=\"grid\" " + GetBgStyle(grid_map[gridX][gridY].Type(), false));
                } else {
                    map.Get(x, y).SetStyle("class=\"grid\"");
                }
                if (with_player) {
                    string content = "";
                    if (boss.x == gridX && boss.y == gridY) {
                        content += HTML_COLOR_FONT_HEADER(red) "★" HTML_FONT_TAIL;
                    }
                    for (auto pid: player_map[gridX][gridY]) {
                        content += num[pid];
                        if (content.length() % 2 == 0) {
                            content += "<br>";
                        }
                    }
                    if (!content.empty()) {
                        map.Get(x, y).SetContent(HTML_SIZE_FONT_HEADER(4) + content + HTML_FONT_TAIL);
                    }
                }
            }
        }
        // 纵向围墙
        for (int x = 1; x < size * 2; x = x + 2) {
            for (int y = 0; y < size * 2 - 1; y = y + 2) {
                if (grid_map[(x+1)/2-1][y/2].Wall<Direct::LEFT>()) {
                    map.Get(x, y).SetStyle("class=\"wall-col\"").SetColor("black");
                }
            }
            if (grid_map[(x+1)/2-1][size-1].Wall<Direct::RIGHT>()) {
                map.Get(x, size*2).SetStyle("class=\"wall-col\"").SetColor("black");
            }
        }
        // 横向围墙
        for (int y = 1; y < size * 2; y = y + 2) {
            for (int x = 0; x < size * 2 - 1; x = x + 2) {
                if (grid_map[x/2][(y+1)/2-1].Wall<Direct::UP>()) {
                    map.Get(x, y).SetStyle("class=\"wall-row\"").SetColor("black");
                }
            }
            if (grid_map[size-1][(y+1)/2-1].Wall<Direct::DOWN>()) {
                map.Get(size*2, y).SetStyle("class=\"wall-row\"").SetColor("black");
            }
        }
        // 角落方块
        for (int x = 0; x < size * 2 + 1; x = x + 2) {
            for (int y = 0; y < size * 2 + 1; y = y + 2) {
                map.Get(x, y).SetStyle("class=\"corner\"").SetColor("black");
            }
        }
        return style + map.ToString();
    }

    string GetFinalBoard() const
    {
        html::Table finalTable(2, 2);
        finalTable.SetTableStyle("align=\"center\" cellpadding=\"0\" cellspacing=\"0\"");
        finalTable.Get(0, 0).SetContent(HTML_SIZE_FONT_HEADER(5) "<b>初始地图</b>" HTML_FONT_TAIL);
        finalTable.Get(0, 1).SetContent(HTML_SIZE_FONT_HEADER(5) "<b>终局地图</b>" HTML_FONT_TAIL);
        finalTable.Get(1, 0).SetStyle("style=\"padding: 10px 20px 10px 10px\"").SetContent(init_html_);
        finalTable.Get(1, 1).SetStyle("style=\"padding: 10px 10px 10px 20px\"").SetContent(GetBoard(grid_map));
        return GetPlayerTable(-1) + style + finalTable.ToString();
    }

    // 获取玩家信息
    string GetPlayerTable(const int round) const
    {
        html::Table playerTable(playerNum + (boss.steps >= 0), 6);
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
                playerTable.Get(pid, 3).SetStyle("style=\"width:150px;\"").SetContent("【单机模式】<br>寻找唯一的逃生舱！");
            } else if (players[pid].out == 0) {
                playerTable.Get(pid, 3).SetStyle("style=\"width:40px;\"").SetContent("捕捉<br>目标");
                playerTable.Get(pid, 4).SetStyle("style=\"width:40px;\"").SetContent("[" + to_string(players[pid].target) + "号]");
                playerTable.Get(pid, 5).SetStyle("style=\"width:40px;\"").SetContent(players[players[pid].target].avatar);
            } else {
                playerTable.MergeRight(pid, 3, 3);
                playerTable.Get(pid, 3).SetStyle("style=\"width:120px;\"").SetColor("#FFA07A").SetContent("[玩家状态错误]");
            }
        }
        if (boss.steps >= 0) {
            playerTable.Get(playerNum, 0).SetStyle("style=\"width:60px;\"").SetContent("<font color=\"red\" size=\"5\">★</font>");
            playerTable.MergeRight(playerNum, 1, 2);
            playerTable.Get(playerNum, 1).SetStyle("style=\"width:290px;\"").SetColor("lavender").SetContent("[BOSS] 米诺陶斯");
            playerTable.Get(playerNum, 3).SetStyle("style=\"width:40px;\"").SetContent("捕捉<br>目标");
            playerTable.Get(playerNum, 4).SetStyle("style=\"width:40px;\"").SetContent("[" + to_string(boss.target) + "号]");
            playerTable.Get(playerNum, 5).SetStyle("style=\"width:40px;\"").SetContent(players[boss.target].avatar);
        }
        return (round > 0 ? "### 第 " + to_string(round) + " 回合" : "") + playerTable.ToString();
    }

    // 全部区块信息展示
    string GetAllBlocksInfo(const int special, const int32_t test_mode = 0) const
    {
        const vector<UnitMaps::Map>& maps = test_mode == 0 ? unitMaps.maps : (test_mode == 2 ? unitMaps.rotation_maps : unitMaps.all_maps);
        const vector<UnitMaps::Map>& exits = test_mode == 0 ? unitMaps.exits : (test_mode == 2 ? unitMaps.rotation_exits : unitMaps.all_exits);
        const vector<UnitMaps::Map>& special_maps = unitMaps.special_maps;
        const string title = 
            (test_mode == 0) ? "" 
            : (test_mode == 2) ? 
                (HTML_SIZE_FONT_HEADER(6) "<b>《漫漫长夜》幻变模式轮换区块</b>" HTML_FONT_TAIL) 
                : (HTML_SIZE_FONT_HEADER(6) "<b>《漫漫长夜》狂野+疯狂模式全部区块</b>" HTML_FONT_TAIL);
        bool has_special = (test_mode == 3);
        for (const auto& map : maps) {
            if (map.type == GridType::SPECIAL) {
                has_special = true;
            }
        }

        int line_num = (ceil(maps.size() / 4.0) + ceil(exits.size() / 4.0)) * 2 +  + has_special * (ceil(special_maps.size() / 4.0) * 2 + 1);
        html::Table blocks(line_num, 4);
        blocks.SetTableStyle("align=\"center\" cellpadding=\"0\" cellspacing=\"0\" style=\"font-size: 25px;\"");
        int row = 0;
        for (int i = 0; i < maps.size(); i++) {
            blocks.Get(row, i % 4).SetStyle("style=\"padding: 25px 25px 0px 25px\"").SetContent(GetSingleBlock(0, maps[i].id, special));
            // 特殊地图不显示id
            blocks.Get(row + 1, i % 4).SetContent(HTML_COLOR_FONT_HEADER(red) "<b>" + (maps[i].id[0] == 'S' ? "？？？" : maps[i].id) + "</b>" HTML_FONT_TAIL);
            if ((i + 1) % 4 == 0 || i == maps.size() - 1) row += 2;
        }
        for (int i = 0; i < exits.size(); i++) {
            blocks.Get(row, i % 4).SetStyle("style=\"padding: 20px 25px 5px 25px\"").SetContent(GetSingleBlock(1, exits[i].id, special));
            blocks.Get(row + 1, i % 4).SetContent(HTML_COLOR_FONT_HEADER(red) "<b>EXIT " + exits[i].id + "</b>" HTML_FONT_TAIL);
            if ((i + 1) % 4 == 0 || i == exits.size() - 1) row += 2;
        }
        if (has_special) {
            blocks.MergeRight(row, 0, 4);
            blocks.Get(row++, 0).SetStyle("style=\"padding: 20px 25px 5px 25px\"")
                .SetContent(HTML_SIZE_FONT_HEADER(6) "<b>特殊区块列表<br>" HTML_FONT_TAIL HTML_SIZE_FONT_HEADER(5) "（多传送门均为中心对称传送）</b>");
            for (int i = 0; i < special_maps.size(); i++) {
                blocks.Get(row, i % 4).SetStyle("style=\"padding: 20px 25px 5px 25px\"")
                    .SetContent(GetBoard(unitMaps.FindBlockById(special_maps[i].id, false, special == 3), false));
                blocks.Get(row + 1, i % 4).SetContent(HTML_COLOR_FONT_HEADER(red) "<b>" + special_maps[i].id + "</b>" HTML_FONT_TAIL);
                if ((i + 1) % 4 == 0 || i == special_maps.size() - 1) row += 2;
            }
        }
        auto generateSVG = [](const std::string& style) -> std::string {
            return "<svg width=\"50\" height=\"50\">"
                   "<rect width=\"50\" height=\"50\" " + style + "/>"
                   "</svg>";
        };
        const string wall_svg = "<svg width=\"50\" height=\"70\">"
                "<rect x=\"0\" y=\"0\" width=\"50\" height=\"10\" fill=\"black\"/>"
                "<rect x=\"0\" y=\"10\" width=\"50\" height=\"50\" fill=\"white\" stroke=\"black\"/>"
                "<rect x=\"0\" y=\"60\" width=\"50\" height=\"10\" fill=\"black\"/>"
                "</svg>";
        html::Table legend(9, 2);
        legend.SetTableStyle("cellpadding=\"5\" cellspacing=\"0\" style=\"font-size: 22px;\"");
        for (int i = 0; i < 9; i++) {
            legend.Get(i, 1).SetStyle("style=\"text-align: left;\"");
        }
        legend.Get(0, 0).SetContent(wall_svg);
        legend.Get(0, 1).SetContent("【墙壁】黑色为墙壁，如图例，玩家不可从上下通过，可以从左右通过");
        legend.Get(1, 0).SetContent(generateSVG(GetBgStyle(GridType::GRASS, true)));
        legend.Get(1, 1).SetContent("【树丛】玩家进入时会发出让其他人听见的沙沙声。（出生不算）");
        legend.Get(2, 0).SetContent(generateSVG(GetBgStyle(GridType::WATER, true)));
        legend.Get(2, 1).SetContent("【水洼】玩家进入时会发出让其他人听见的啪啪声。（出生不算）");
        legend.Get(3, 0).SetContent(generateSVG(GetBgStyle(GridType::PORTAL, true)));
        legend.Get(3, 1).SetContent("【传送门】玩家进入时会发出其他人听见的啪啪声。（出生不算）<br>进入后，再任意2次移动后就会传送至同区块另1个传送门。<br>进入后，玩家视作进入亚空间，上述2次移动都在亚空间内。");
        legend.Get(4, 0).SetContent(generateSVG(GetBgStyle(GridType::ONEWAYPORTAL, true)));
        legend.Get(4, 1).SetContent("【传送门出口】玩家进入时会发出其他人听见的啪啪声。（出生不算）<br>传送门的单向出口，进入时不会触发传送（必须从入口进入才会传送至此处）<br><b>玩家在进入同一区块的传送门入口时，传送门会转换方向，入口和出口交换位置</b>");
        legend.Get(5, 0).SetContent(generateSVG(GetBgStyle(GridType::TRAP, true)));
        legend.Get(5, 1).SetContent("【陷阱】陷阱隐藏在树丛中：被奇数次进入时，会发出让其他人听见的沙沙声（出生不算）<br>被偶数次进入时，不发出声响，并强制玩家停止（出生不算）");
        legend.Get(6, 0).SetContent(generateSVG(GetBgStyle(GridType::HEAT, true)));
        legend.Get(6, 1).SetContent("【热源】进入热源周围8格时，将私信受到热浪提示。（出生不算）<br>当进入热源时，将私信收到高温烫伤提示（不会出生在热源内）<br><b>在整局游戏中，如果第二次进入热源，则直接出局并-150分</b>");
        legend.Get(7, 0).SetContent(generateSVG(GetBgStyle(GridType::BOX, true)));
        legend.Get(7, 1).SetContent("【箱子】玩家相邻箱子且向箱子移动时，箱子可被推动。（不会出生在箱子内）<br>箱子不可移动到本区块外。若箱子不可推动，则撞墙，箱子本身不会显示为墙。");
        legend.Get(8, 0).SetContent(generateSVG(GetBgStyle(GridType::EXIT, true)));
        legend.Get(8, 1).SetContent("【逃生舱】逃生者使用后，会消失。默认逃生舱数=人数的一半");
        return title + blocks.ToString() + legend.ToString();
    }

    string GetSingleBlock(const int type, const string id, const int special) const
    {
        // 特殊地图不显示预览
        if (id[0] == 'S') return "";
        vector<vector<Grid>> grid;
        if (type == 0) {
            if (special == 3) {
                grid = unitMaps.FindBlockById(id, false, true);
            } else {
                grid = unitMaps.FindBlockById(id, false);
            }
        } else if (type == 1) {
            grid = unitMaps.FindBlockById(id, true);
        }
        return GetBoard(grid, false);
    }

    string GetAllRecord() const
    {
        string all_record;
        for (int pid = 0; pid < playerNum; pid++) {
            all_record += "<b>[" + to_string(pid) + "号]" + players[pid].name + "</b>";
            all_record += players[pid].all_record + "<br>";
            if (pid < playerNum - 1) all_record += "<br>";
        }
        if (boss.steps >= 0) {
            all_record += "<br><b>[BOSS] 米诺陶斯</b>";
            all_record += boss.all_record + "<br>";
        }
        return regex_replace(all_record, regex(R"(\]\()"), "] (");
    }

    string GetAllScore() const
    {
        html::Table scoreTable(playerNum + 1, 7);
        scoreTable.SetTableStyle("cellpadding=\"2\" cellspacing=\"0\" border=\"1\"");
        scoreTable.Get(0, 0).SetStyle("style=\"width:50px;\"").SetContent("序号");
        scoreTable.Get(0, 1).SetStyle("style=\"width:250px;\"").SetContent("玩家");
        scoreTable.Get(0, 2).SetStyle("style=\"width:70px;\"").SetContent("抓人分");
        scoreTable.Get(0, 3).SetStyle("style=\"width:70px;\"").SetContent("逃生分");
        scoreTable.Get(0, 4).SetStyle("style=\"width:70px;\"").SetContent("探索分");
        scoreTable.Get(0, 5).SetStyle("style=\"width:70px;\"").SetContent("额外<br>探索分");
        scoreTable.Get(0, 6).SetStyle("style=\"width:100px;\"").SetContent("【最终总分】");
        for (int pid = 0; pid < playerNum; pid++) {
            Score s = players[pid].score;
            scoreTable.Get(pid + 1, 0).SetContent(to_string(pid) + "号");
            scoreTable.Get(pid + 1, 1).SetContent(players[pid].name);
            if (s.catch_score > 0) scoreTable.Get(pid + 1, 2).SetColor("#BAFFA8");
            if (s.catch_score < 0) scoreTable.Get(pid + 1, 2).SetColor("#FFA07A");
            scoreTable.Get(pid + 1, 2).SetContent(to_string(s.catch_score));
            if (s.exit_score > 0) scoreTable.Get(pid + 1, 3).SetColor(GetBgColor(GridType::EXIT));
            scoreTable.Get(pid + 1, 3).SetContent(to_string(s.exit_score));
            auto [c1, c2] = s.ExploreCount();
            scoreTable.Get(pid + 1, 4).SetContent(to_string(c1 + c2));
            scoreTable.Get(pid + 1, 5).SetContent(to_string(c2));
            scoreTable.Get(pid + 1, 6).SetContent((s.quit_score < 0 ? HTML_COLOR_FONT_HEADER("red") : HTML_COLOR_FONT_HEADER("black")) + to_string(s.FinalScore()) + HTML_FONT_TAIL);
        }
        return Score::ScoreInfo() + scoreTable.ToString();
    }

    // 玩家移动
    bool MakeMove(const PlayerID pid, const Direct direction, const bool hide)
    {
        static const int dx[4] = {-1, 1, 0, 0};
        static const int dy[4] = {0, 0, -1, 1};
        static const char* arrow[4] = {"↑", "↓", "←", "→"};
        static const char* hit[4] = {"(↑撞)", "(↓撞)", "(←撞)", "(→撞)"};

        int d = static_cast<int>(direction);
        int cx = players[pid].x;
        int cy = players[pid].y;
        int nx = (cx + dx[d] + size) % size;
        int ny = (cy + dy[d] + size) % size;

        bool wall = false;
        switch (direction) {
            case Direct::UP:    wall = grid_map[cx][cy].Wall<Direct::UP>(); break;
            case Direct::DOWN:  wall = grid_map[cx][cy].Wall<Direct::DOWN>(); break;
            case Direct::LEFT:  wall = grid_map[cx][cy].Wall<Direct::LEFT>(); break;
            case Direct::RIGHT: wall = grid_map[cx][cy].Wall<Direct::RIGHT>(); break;
            default: return false;
        }
        // 非撞墙尝试移动箱子
        if (!wall && grid_map[nx][ny].Type() == GridType::BOX && players[pid].subspace < 0) {
            wall = !BoxMove(nx, ny, dx[d], dy[d]);
        }
        // 轨迹记录
        if (wall && players[pid].subspace < 0) {
            if (!hide) players[pid].move_record += hit[d];
            return false;
        }
        if (!hide) players[pid].move_record += arrow[d];

        if (players[pid].subspace > 0) {
            players[pid].subspace--;
            return true;
        }

        RemovePlayerFromMap(pid);

        players[pid].x = nx;
        players[pid].y = ny;
        player_map[nx][ny].push_back(pid);

        // 探索分
        auto& explore = players[pid].score.explore_map[nx][ny];
        if (explore == 0) {
            bool first = true;
            for (auto& player: players) {
                if (player.score.explore_map[nx][ny] > 0) {
                    first = false;
                }
            }
            explore = first ? 2 : 1;
        }
    
        return true;
    }

    // 箱子移动
    bool BoxMove(const int b_cx, const int b_cy, const int dx, const int dy)
    {
        int b_nx = (b_cx + dx + size) % size;
        int b_ny = (b_cy + dy + size) % size;
        if (!player_map[b_nx][b_ny].empty()) return false;
        if (grid_map[b_nx][b_ny].Type() != GridType::EMPTY) return false;

        bool wall = false;
        if (dx == -1 && dy == 0)        wall = grid_map[b_cx][b_cy].Wall<Direct::UP>();
        else if (dx == 1 && dy == 0)    wall = grid_map[b_cx][b_cy].Wall<Direct::DOWN>();
        else if (dx == 0 && dy == -1)   wall = grid_map[b_cx][b_cy].Wall<Direct::LEFT>();
        else if (dx == 0 && dy == 1)    wall = grid_map[b_cx][b_cy].Wall<Direct::RIGHT>();
        if (wall) return false;

        bool sameBlock = false;
        for (auto pos : unitMaps.pos) {
            int bx = pos.first, by = pos.second;
            if (b_cx >= bx && b_cx < bx + 3 && b_cy >= by && b_cy < by + 3 &&
                b_nx >= bx && b_nx < bx + 3 && b_ny >= by && b_ny < by + 3) {
                sameBlock = true;
                break;
            }
        }
        if (!sameBlock) return false;

        grid_map[b_cx][b_cy].SetType(GridType::EMPTY);
        grid_map[b_nx][b_ny].SetType(GridType::BOX);
        return true;
    }

    // 玩家从地图中移除（辅助功能）
    void RemovePlayerFromMap(const PlayerID pid)
    {
        int cx = players[pid].x;
        int cy = players[pid].y;
        auto &oldCell = player_map[cx][cy];
        auto it = std::find(oldCell.begin(), oldCell.end(), pid);
        if (it != oldCell.end()) oldCell.erase(it);
    }

    // 地区声响（0无 1沙沙 2啪啪）
    Sound GetSound(const Grid& grid, const bool water_mode) const
    {
        switch(grid.Type()) {
            case GridType::GRASS:   return Sound::SHASHA;
            case GridType::WATER:   return Sound::PAPA;
            case GridType::ONEWAYPORTAL:
            case GridType::PORTAL:  return Sound::PAPA;
            case GridType::TRAP:    return grid.TrapStatus() ? Sound::NONE : (water_mode ? Sound::PAPA : Sound::SHASHA);
            default: return Sound::NONE;
        }
    }

    // 获取声响方向（用于发送声响）
    string GetSoundDirection(const int fromX, const int fromY, const Player& to) const
    {
        int toX = to.x;
        int toY = to.y;

        int dx = toX - fromX;
        int dy = toY - fromY;

        if (abs(dx) > size / 2) {
            if (dx > 0)
                dx -= size;
            else
                dx += size;
        }
        if (abs(dy) > size / 2) {
            if (dy > 0)
                dy -= size;
            else
                dy += size;
        }
    
        if (size % 2 == 0 && abs(dx) == size / 2) {
            dx = (rand() % 2 == 0) ? size / 2 : -size / 2;
        }
        if (size % 2 == 0 && abs(dy) == size / 2) {
            dy = (rand() % 2 == 0) ? size / 2 : -size / 2;
        }

        // 两玩家在同一位置
        if (dx == 0 && dy == 0) {
            return "";
        }

        if (dx == 0) {
            return (dy < 0) ? "正右" : "正左";
        }
        if (dy == 0) {
            return (dx < 0) ? "正下" : "正上";
        }
        string vertical = (dx < 0) ? "下" : "上";
        string horizontal = (dy < 0) ? "右" : "左";
        return horizontal + vertical;
    }

    Grid& PlayerLocationGrid(const PlayerID pid)
    {
        return grid_map[players[pid].x][players[pid].y];
    }

    // 热浪提示
    bool HeatNotice(const PlayerID pid)
    {
        int cx = players[pid].x;
        int cy = players[pid].y;
        if (grid_map[cx][cy].Type() == GridType::HEAT) {
            return false;
        }
        bool inZone = false;
        for (int dx = -1; dx <= 1 && !inZone; dx++) {
            for (int dy = -1; dy <= 1 && !inZone; dy++) {
                int nx = (cx + dx + size) % size;
                int ny = (cy + dy + size) % size;
                if (grid_map[nx][ny].Type() == GridType::HEAT) {
                    inZone = true;
                }
            }
        }
        if (inZone && !players[pid].inHeatZone) {
            players[pid].inHeatZone = true;
            return true;
        }
        if (!inZone) {
            players[pid].inHeatZone = false;
        }
        return false;
    }

    // 获取四周墙壁信息
    pair<string, string> GetSurroundingWalls(const PlayerID pid) const
    {
        Grid grid = grid_map[players[pid].x][players[pid].y];
        if (players[pid].subspace > 0) {
            grid.SetWall(false, false, false, false);
        }
        string info;
        if (grid.Wall<Direct::UP>()) info += "墙"; else info += "空";
        if (grid.Wall<Direct::DOWN>()) info += "墙"; else info += "空";
        if (grid.Wall<Direct::LEFT>()) info += "墙"; else info += "空";
        if (grid.Wall<Direct::RIGHT>()) info += "墙"; else info += "空";
        return make_pair(info, GetBoard({{grid}}, false));
    }

    // 玩家随机传送
    void TeleportPlayer(const PlayerID pid)
    {
        players[pid].subspace = -1;
        players[pid].inHeatZone = false;
        vector<pair<int, int>> candidates = FindLargestConnectedArea();
    
        // 禁止坐标
        vector<pair<int, int>> forbiddenSources;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (grid_map[i][j].Type() == GridType::EXIT || grid_map[i][j].Type() == GridType::HEAT) {
                    forbiddenSources.push_back({i, j});
                }
            }
        }
        if (boss.steps >= 0) {
            forbiddenSources.push_back({boss.x, boss.y});
        }
        for (const auto& player: players) {
            if (player.pid == pid || player.out > 0) continue;
            forbiddenSources.push_back({player.x, player.y});
        }
        // 禁止区域（3*3）
        vector<vector<bool>> forbidden(size, vector<bool>(size, false));
        for (const auto& src : forbiddenSources) {
            int sx = src.first, sy = src.second;
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    int ni = (sx + di + size) % size;
                    int nj = (sy + dj + size) % size;
                    forbidden[ni][nj] = true;
                }
            }
        }
        // 过滤候选区域（同时去除BOSS可能到达的区域）
        vector<pair<int, int>> finalCandidates;
        std::copy_if(
            candidates.begin(), candidates.end(),
            std::back_inserter(finalCandidates),
            [&](auto const& pos) {
                auto [x, y] = pos;
                return !forbidden[x][y] && (boss.steps <= 0 || ManhattanDistance(x, y, boss.x, boss.y) > boss.steps);
            }
        );
    
        // 无有效区域，不传送
        if (finalCandidates.empty()) {
            cout << "[debug warning] 传送失败，无有效候选位置" << endl;
            return;
        }
    
        std::shuffle(finalCandidates.begin(), finalCandidates.end(), g);
        pair<int, int> newPos = finalCandidates.front();
    
        RemovePlayerFromMap(pid);
    
        players[pid].x = newPos.first;
        players[pid].y = newPos.second;
        player_map[newPos.first][newPos.second].push_back(pid);
    }

    // 捕捉顺位变更
    void UpdatePlayerTarget(const bool next)
    {
        for (PlayerID pid = 0; pid < playerNum; ++pid) {
            if (players[pid].out > 0) continue;
            PlayerID target = next ? (pid + 1) % playerNum : (playerNum + pid - 1) % playerNum;
            while (players[target].out > 0 && target != pid) {
                players[target].target = -1;
                target = next ? (target + 1) % playerNum : (playerNum + target - 1) % playerNum;
            }
            players[pid].target = target;
        }
    }

    string MapGenerate(const vector<string>& map_str) const
    {
        vector<vector<Grid>> grid_map;
        grid_map.resize(size);
        for (int i = 0; i < size; i++) {
            grid_map[i].resize(size);
        }
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
                unitMaps.SetExitBlock(unitMaps.origin_pos[i].first, unitMaps.origin_pos[i].second, grid_map, map_id, false);
            } else {
                unitMaps.SetMapBlock(unitMaps.origin_pos[i].first, unitMaps.origin_pos[i].second, grid_map, map_id, false);
            }
        }
        FixAdjacentWalls(grid_map);
        FixInvalidPortals(grid_map);
        return GetBoard(grid_map, false);
    }

    // BOSS生成和锁定目标
    void BossSpawn()
    {
        for (int i = 0; i < 1000; i++) {
            int x = rand() % size;
            int y = rand() % size;
            bool nearPlayer = false;
            for (int i = 0; i < playerNum && !nearPlayer; i++) {
                for (int dx = -1; dx <= 1 && !nearPlayer; dx++) {
                    for (int dy = -1; dy <= 1 && !nearPlayer; dy++) {
                        int nx = (players[i].x + dx + size) % size;
                        int ny = (players[i].y + dy + size) % size;
                        if (nx == x && ny == y)
                            nearPlayer = true;
                    }
                }
            }
            boss.x = x;
            boss.y = y;
            if (!nearPlayer) {
                boss.steps = 0;
                break;
            }
        }
        // 初始锁定最近的玩家作为目标
        int bestDist = INT_MAX;
        vector<int> candidates;
        for (int i = 0; i < playerNum; i++) {
            int d = ManhattanDistance(boss.x, boss.y, players[i].x, players[i].y);
            if (d < bestDist) {
                bestDist = d;
                candidates.clear();
                candidates.push_back(i);
            } else if (d == bestDist) {
                candidates.push_back(i);
            }
        }
        boss.target = candidates[rand() % candidates.size()];
    }

    // BOSS更换目标（未更换返回true）
    bool BossChangeTarget(const bool reset)
    {
        int curTarget = boss.target;
        int curDist = ManhattanDistance(boss.x, boss.y, players[boss.target].x, players[boss.target].y);
        if (reset || players[boss.target].out > 0) curDist = INT_MAX;
        for (auto& player : players) {
            if (player.out > 0) continue;
            int d = ManhattanDistance(boss.x, boss.y, player.x, player.y);
            if (d < curDist) {
                boss.target = player.pid;
                boss.steps = 0;
                curDist = d;
            }
        }
        return curTarget == boss.target;
    }

    // BOSS移动和更换目标（抓到人返回true）
    bool BossMove()
    {
        int targetDist = ManhattanDistance(boss.x, boss.y, players[boss.target].x, players[boss.target].y);
        boss.steps++;
        // 步数足够直接走到目标位置
        if (boss.steps >= targetDist) {
            boss.x = players[boss.target].x;
            boss.y = players[boss.target].y;
            boss.steps = 0;
            return true;
        }
        // 执行移动
        int stepsRemaining = boss.steps;
        while (stepsRemaining > 0) {
            targetDist = ManhattanDistance(boss.x, boss.y, players[boss.target].x, players[boss.target].y);
            int tx = players[boss.target].x, ty = players[boss.target].y;
            int dx = tx - boss.x, dy = ty - boss.y;
            if (abs(dx) > size / 2) { dx = (dx > 0) ? dx - size : dx + size; }
            if (abs(dy) > size / 2) { dy = (dy > 0) ? dy - size : dy + size; }

            // 如果|dx|≠|dy|则沿较大差值轴走一步
            if (abs(dx) != abs(dy)) {
                if (abs(dx) > abs(dy)) {
                    boss.x = (boss.x + ((dx > 0) ? 1 : -1) + size) % size;
                } else {
                    boss.y = (boss.y + ((dy > 0) ? 1 : -1) + size) % size;
                }
            } else {
                // 如果已经正方形，随机选择在 x 或 y 方向上移动一步
                if (rand() % 2 == 0)
                    boss.x = (boss.x + ((dx > 0) ? 1 : -1) + size) % size;
                else
                    boss.y = (boss.y + ((dy > 0) ? 1 : -1) + size) % size;
            }
            stepsRemaining--;
        }
        return false;
    }

    bool IsBossNearby(const Player& player) const
    {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = (boss.x + dx + size) % size;
                int ny = (boss.y + dy + size) % size;
                if (player.x == nx && player.y == ny) {
                    return true;
                }
            }
        }
        return false;
    }

  private:
    // 初始化区块
    void InitializeBlock()
    {
        vector<UnitMaps::Map> maps = unitMaps.maps;
        vector<UnitMaps::Map> exits = unitMaps.exits;
        std::shuffle(maps.begin(), maps.end(), g);
        std::shuffle(exits.begin(), exits.end(), g);

        vector<UnitMaps::Map> selected;
        for (int i = 0; i < unitMaps.pos.size() - exit_num; i++) {
            selected.push_back(maps[i]);
        }
        for (int i = 0; i < exit_num; i++) {
            selected.push_back(exits[i]);
        }
        std::shuffle(selected.begin(), selected.end(), g);

        for (int i = 0; i < unitMaps.pos.size(); i++) {
            if (selected[i].type == GridType::EXIT) {
                unitMaps.SetExitBlock(unitMaps.pos[i].first, unitMaps.pos[i].second, grid_map, selected[i].id, true);
            } else {
                unitMaps.SetMapBlock(unitMaps.pos[i].first, unitMaps.pos[i].second, grid_map, selected[i].id, true);
            }
            // 记录特殊地图坐标（暂不使用）
            // if (selected[i].type == GridType::SPECIAL) {
            //     special_pos = unitMaps.pos[i];
            // }
        }
    }

    // 相邻墙面修复
    void FixAdjacentWalls(vector<vector<Grid>>& grid_map) const
    {
        for (int x = 0; x < size; x++) {
            for (int y = 0; y < size; y++) {
                if (grid_map[x][y].Wall<Direct::LEFT>()) {
                    grid_map[x][(y - 1 + size) % size].SetWall<Direct::RIGHT>(true);
                }
                if (grid_map[x][y].Wall<Direct::RIGHT>()) {
                    grid_map[x][(y + 1 + size) % size].SetWall<Direct::LEFT>(true);
                }
                if (grid_map[x][y].Wall<Direct::UP>()) {
                    grid_map[(x - 1 + size) % size][y].SetWall<Direct::DOWN>(true);
                }
                if (grid_map[x][y].Wall<Direct::DOWN>()) {
                    grid_map[(x + 1 + size) % size][y].SetWall<Direct::UP>(true);
                }
            }
        }
    }

    // 封闭传送门替换为水洼
    void FixInvalidPortals(vector<vector<Grid>>& grid_map) const
    {
        for (int x = 0; x < size; x++) {
            for (int y = 0; y < size; y++) {
                if (grid_map[x][y].Type() == GridType::PORTAL && grid_map[x][y].IsFullyEnclosed()) {
                    pair<int, int> pRelPos = grid_map[x][y].PortalPos();
                    grid_map[x + pRelPos.first][y + pRelPos.second].SetType(GridType::ONEWAYPORTAL);
                }
            }
        }
    }

    // 判断(x, y)格子沿(dx, dy)方向是否连通（考虑环绕）
    bool IsConnected(int x, int y, int dx, int dy) const
    {
        int nx = (x + dx + size) % size;
        int ny = (y + dy + size) % size;
        if (grid_map[x][y].Type() == GridType::HEAT || grid_map[x][y].Type() == GridType::BOX) {
            return false;
        }
        if (grid_map[nx][ny].Type() == GridType::HEAT || grid_map[nx][ny].Type() == GridType::BOX) {
            return false;
        }
        if (dx == -1 && dy == 0)
            return !grid_map[x][y].Wall<Direct::UP>() && !grid_map[nx][ny].Wall<Direct::DOWN>();
        else if (dx == 1 && dy == 0)
            return !grid_map[x][y].Wall<Direct::DOWN>() && !grid_map[nx][ny].Wall<Direct::UP>();
        else if (dx == 0 && dy == -1)
            return !grid_map[x][y].Wall<Direct::LEFT>() && !grid_map[nx][ny].Wall<Direct::RIGHT>();
        else if (dx == 0 && dy == 1)
            return !grid_map[x][y].Wall<Direct::RIGHT>() && !grid_map[nx][ny].Wall<Direct::LEFT>();
        return false;
    }

    int BFSRouteDistance(int sx, int sy, int ex, int ey) const
    {
        vector<vector<bool>> visited(size, vector<bool>(size, false));
        queue<tuple<int, int, int>> q; // (x, y, distance)
        q.push({sx, sy, 0});
        visited[sx][sy] = true;
        while (!q.empty()) {
            auto [x, y, d] = q.front();
            q.pop();
            if (x == ex && y == ey)
                return d;
            int dx[4] = {-1, 1, 0, 0};
            int dy[4] = {0, 0, -1, 1};
            for (int i = 0; i < 4; i++) {
                int nx = (x + dx[i] + size) % size;
                int ny = (y + dy[i] + size) % size;
                if (!visited[nx][ny] && IsConnected(x, y, dx[i], dy[i])) {
                    visited[nx][ny] = true;
                    q.push({nx, ny, d + 1});
                }
            }
        }
        return INT_MAX;
    }
    
    // * 随机生成所有玩家的位置，所有位置均在最大联通区域内 *
    // 按照顺序依次尝试：
    //   方案1：使用非逃生舱区块，相邻玩家之间的 BFS 路径距离≥5；每个玩家落点到所有逃生舱的 BFS 路径距离≥5
    //   方案2：使用非逃生舱区块，仅要求相邻玩家之间的 BFS 路径距离≥5（不检查逃生舱距离）
    //   方案3：直接使用非逃生舱区块随机分配（不检查距离）
    //   方案4：允许使用逃生舱区块
    //   保险方案：直接从最大连通区域中随机选取位置
    void RandomizePlayers()
    {
        // 1. 获取最大连通区域
        vector<pair<int, int>> largest = FindLargestConnectedArea();
        vector<vector<bool>> inLargest(size, vector<bool>(size, false));
        for (auto coord : largest)
            inLargest[coord.first][coord.second] = true;

        // 1.1 收集所有逃生舱位置
        vector<pair<int, int>> exitPositions;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (grid_map[i][j].Type() == GridType::EXIT)
                    exitPositions.push_back({i, j});
            }
        }

        // Lambda：计算 (x,y) 的连通度（即周围可连通方向数量）
        auto connectivityDegree = [this](int x, int y) -> int {
            int degree = 0;
            int dx[4] = {-1, 1, 0, 0};
            int dy[4] = {0, 0, -1, 1};
            for (int k = 0; k < 4; k++) {
                if (IsConnected(x, y, dx[k], dy[k]))
                    degree++;
            }
            return degree;
        };

        // 2. 构造候选区块：遍历 UnitMaps::pos 中所有区块
        //      - blockPos：区块左上角坐标
        //      - validCells：区块内所有满足在最大联通区域且连通度 ≥ 2 的候选落点
        //      - isExit：若区块内含有逃生舱，则为 true
        struct BlockCandidate {
            pair<int, int> blockPos;
            vector<pair<int, int>> validCells;
            bool isExit;
        };
        vector<BlockCandidate> candidateBlocks;
        for (auto pos : unitMaps.pos) {
            int bx = pos.first, by = pos.second;
            bool containsExit = false;
            vector<pair<int, int>> validCells;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (grid_map[bx + i][by + j].Type() == GridType::EXIT) {
                        containsExit = true;
                        break;
                    }
                    if (!inLargest[bx + i][by + j])
                        continue;
                    if (connectivityDegree(bx + i, by + j) >= 2)
                        validCells.push_back({bx + i, by + j});
                }
                if (containsExit)
                    break;
            }
            candidateBlocks.push_back({{bx, by}, validCells, containsExit});
        }

        // 3. 筛选非逃生舱区块（用于方案1、2、3）
        vector<BlockCandidate> candidateNonExit;
        for (auto &b : candidateBlocks) {
            if (!b.isExit && !b.validCells.empty())
                candidateNonExit.push_back(b);
        }
        // 为使得候选区块分布随机，先随机打乱候选非逃生舱区块顺序
        std::shuffle(candidateNonExit.begin(), candidateNonExit.end(), g);

        // 4. 方案1：使用非逃生舱区块，相邻玩家之间的 BFS 路径距离≥5；每个玩家落点到所有逃生舱的 BFS 路径距离≥5
        vector<pair<int, int>> assigned(playerNum, {-1, -1});
        vector<bool> used(candidateNonExit.size(), false);
        // 回溯函数，checkExit 表示是否需要额外检查落点到逃生舱的距离
        function<bool(int, bool)> bt = [&](int idx, bool checkExit) -> bool {
            if (idx == playerNum) {
                int d = BFSRouteDistance(assigned[playerNum - 1].first, assigned[playerNum - 1].second,
                                        assigned[0].first, assigned[0].second);
                return d >= 5 || playerNum == 1;
            }
            for (int i = 0; i < candidateNonExit.size(); i++) {
                if (used[i])
                    continue;
                // 对当前区块随机遍历其候选落点
                vector<pair<int, int>> cellCandidates = candidateNonExit[i].validCells;
                std::shuffle(cellCandidates.begin(), cellCandidates.end(), g);
                for (auto candidate : cellCandidates) {
                    if (idx > 0) {
                        int d = BFSRouteDistance(assigned[idx - 1].first, assigned[idx - 1].second,
                                                candidate.first, candidate.second);
                        if (d < 5)
                            continue;
                    }
                    if (checkExit) {
                        bool ok = true;
                        for (auto &ex : exitPositions) {
                            int d = BFSRouteDistance(candidate.first, candidate.second, ex.first, ex.second);
                            if (d < 5) { ok = false; break; }
                        }
                        if (!ok)
                            continue;
                    }
                    assigned[idx] = candidate;
                    used[i] = true;
                    if (bt(idx + 1, checkExit))
                        return true;
                    used[i] = false;
                }
            }
            return false;
        };

        if (candidateNonExit.size() >= (unsigned)playerNum && bt(0, true)) {
            // 方案1成功
            for (int i = 0; i < playerNum; i++) {
                int x = assigned[i].first, y = assigned[i].second;
                players[i].x = x;
                players[i].y = y;
                player_map[x][y].push_back(i);
            }
            return;
        }

        // 5. 方案2：使用非逃生舱区块，仅要求相邻玩家之间的 BFS 路径距离≥5（不检查逃生舱距离）
        std::fill(used.begin(), used.end(), false);
        if (candidateNonExit.size() >= (unsigned)playerNum && bt(0, false)) {
            for (int i = 0; i < playerNum; i++) {
                int x = assigned[i].first, y = assigned[i].second;
                players[i].x = x;
                players[i].y = y;
                player_map[x][y].push_back(i);
            }
            return;
        }

        // 6. 方案3：直接使用非逃生舱区块随机分配（不检查距离）
        if (candidateNonExit.size() >= (unsigned)playerNum) {
            vector<int> indices(candidateNonExit.size());
            for (int i = 0; i < candidateNonExit.size(); i++)
                indices[i] = i;
            std::shuffle(indices.begin(), indices.end(), g);
            for (int i = 0; i < playerNum; i++) {
                auto &block = candidateNonExit[indices[i]];
                vector<pair<int, int>> cells = block.validCells;
                std::shuffle(cells.begin(), cells.end(), g);
                pair<int, int> chosen = cells.front();
                players[i].x = chosen.first;
                players[i].y = chosen.second;
                player_map[chosen.first][chosen.second].push_back(i);
            }
            return;
        }

        // 7. 方案4：允许使用逃生舱区块
        vector<BlockCandidate> candidateAll;
        for (auto &b : candidateBlocks) {
            if (!b.validCells.empty())
                candidateAll.push_back(b);
        }
        if (candidateAll.size() >= (unsigned)playerNum) {
            vector<int> indices(candidateAll.size());
            for (int i = 0; i < candidateAll.size(); i++)
                indices[i] = i;
            std::shuffle(indices.begin(), indices.end(), g);
            for (int i = 0; i < playerNum; i++) {
                auto &block = candidateAll[indices[i]];
                vector<pair<int, int>> cells = block.validCells;
                std::shuffle(cells.begin(), cells.end(), g);
                pair<int, int> chosen = cells.front();
                players[i].x = chosen.first;
                players[i].y = chosen.second;
                player_map[chosen.first][chosen.second].push_back(i);
            }
            return;
        }

        // 8. 保险方案：直接从最大连通区域随机选取位置
        std::shuffle(largest.begin(), largest.end(), g);
        for (int i = 0; i < playerNum; i++) {
            players[i].x = largest[i].first;
            players[i].y = largest[i].second;
            player_map[largest[i].first][largest[i].second].push_back(i);
        }
    }

    // 查找最大联通区域
    vector<pair<int, int>> FindLargestConnectedArea()
    {
        vector<vector<bool>> visited(size, vector<bool>(size, false));
        vector<vector<pair<int, int>>> components;

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (visited[i][j]) continue;

                vector<pair<int, int>> comp;
                queue<pair<int, int>> q;
                q.push({i, j});
                visited[i][j] = true;
                while (!q.empty()) {
                    auto [cx, cy] = q.front();
                    q.pop();
                    comp.push_back({cx, cy});
                    int dx[4] = {-1, 1, 0, 0};
                    int dy[4] = {0, 0, -1, 1};
                    for (int k = 0; k < 4; k++) {
                        int nx = (cx + dx[k] + size) % size;
                        int ny = (cy + dy[k] + size) % size;
                        if (!visited[nx][ny] && IsConnected(cx, cy, dx[k], dy[k])) {
                            visited[nx][ny] = true;
                            q.push({nx, ny});
                        }
                    }
                }
                components.push_back(comp);
            }
        }

        vector<pair<int, int>> largest;
        for (auto &comp : components) {
            if (comp.size() > largest.size())
                largest = comp;
        }
        if (largest.empty()) assert(false);
        return largest;
    }

    // 计算两点间曼哈顿距离
    int ManhattanDistance(int x1, int y1, int x2, int y2) const
    {
        int dx = abs(x1 - x2);
        int dy = abs(y1 - y2);
        dx = std::min(dx, size - dx);
        dy = std::min(dy, size - dy);
        return dx + dy;
    }

    string GetBgStyle(const GridType type, const bool is_fill) const
    {
        string style;
        if (is_fill) {
            style += "fill: " + GetBgColor(type) + "; ";
        } else {
            style += "background-color: " + GetBgColor(type) + "; ";
        }
        return "style=\"" + style + "\"";
    }

    string GetBgColor(const GridType type) const
    {
        switch (type) {
            case GridType::GRASS:   return "#00AF50";
            case GridType::WATER:   return "#01B0F1";
            case GridType::ONEWAYPORTAL: return "#8563FF";
            case GridType::PORTAL:  return "#73309A";
            case GridType::EXIT:    return "#FFDB60";
            case GridType::TRAP:    return "#808080";
            case GridType::HEAT:    return "#FF0000";
            case GridType::BOX:     return "#C55C10";
            default:                return "#FFFFFF";
        }
    }

    std::mt19937 g;
    inline static constexpr const char* style = R"(
<style>
    .grid {
        width: 50px;
        height: 50px;
    }
    .wall-row {
        width: 50px;
        height: 10px;
    }
    .wall-col {
        width: 10px;
        height: 50px;
    }
    .corner {
        width: 10px;
        height: 10px;
    }
</style>
)";
};
