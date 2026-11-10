
struct BoardArea {
    int x, y, rows, cols;
};

struct GetBoardOptions {
    bool with_player = true;
    bool with_content = false;
    bool with_ground = true;
    bool with_frame = true;     // 是否绘制盘面外侧虚线边框
    optional<BoardArea> area;   // 只绘制部分区域
};

class Board
{
  public:
    Board(string image_path, const Texture image_texture, const BlockMode mode, const vector<string>& custom_blocks, const bool expand)
        : image_path_(std::move(image_path)), image_texture_(image_texture), block_mode(mode), g(std::random_device{}()), unitMaps(mode, g, custom_blocks, expand) {}

    // 随机引擎
    std::mt19937 g;

    // 图片资源文件夹
    const string image_path_;
    const Texture image_texture_;

	// 玩家
    uint32_t playerNum;
    vector<Player> players;
    // 地图内的玩家
    vector<vector<vector<lgtbot::PlayerID>>> player_map;
    // 地图大小
    int size = 9;
    // 地图
    GridMap grid_map;
    BlockMode block_mode;
    int exit_num = 0;  // 逃生舱数量
    string init_html_;
    // 区块模板
    UnitMaps unitMaps;
    // BOSS
    vector<std::unique_ptr<Boss>> bosses;
    bool HasBoss() const { return !bosses.empty(); }
    // 完整赛况额外信息
    string all_extra_record;
    // [巨大的心脏] 地图中是否存在心脏
    bool has_heart = false;
    // 特殊区块位置（暂不使用）
    // pair<int, int> special_pos = {12, 12};

    // 初始化地图
    void Initialize()
    {
        grid_map = GridMap(size, true);
        player_map.resize(size);
        for (int i = 0; i < size; i++) {
            player_map[i].resize(size);
        }

        InitializeBlocks();             // 初始化区块（边界墙按优先级合并）
        FixInvalidPortals(grid_map);    // 封闭传送门替换为水洼
        RandomizePlayers();             // 随机生成玩家
        has_heart = TypeCount(GridType::HEART) > 0;     // [巨大的心脏] 存在判定

        // 成就辅助：记录玩家初始位置
        for (lgtbot::PlayerID pid = 0; pid < playerNum; ++pid) {
            players[pid].achievement.MakeStep(players[pid].x, players[pid].y);
        }
    }

    // 保存初始盘面
    void SaveGameStartMap()
    {
        init_html_ = GetBoard(grid_map);
    }

    /* ========== 地图与赛况渲染（实现见 board_render.h） ========== */
    // 获取地图html
    string GetBoard(const GridMap& grid_map, const GetBoardOptions& options = {}) const;
    // 获取终局盘面
    string GetFinalBoard() const;
    // 全部区块信息展示
    string GetAllBlocksInfo(const SpecialEvent event, const bool bomb_mode, const optional<BlockMode> test_block_mode = std::nullopt) const;
    string GetMapTitleContent(const string& title, const string& id, const bool is_exit) const;
    string GetSingleBlock(const string& id, const bool is_exit, const SpecialEvent event) const;
    // 单区块裸图（工具用）
    string GetSingleBlockNoBorder(const string& id, const bool is_exit, const SpecialEvent event) const;
    // 地形图片完整路径（带纹理）
    string GetGridImagePath(const GridType type) const;
    // 指定区块生成地图预览
    string MapPreview(const vector<string>& map_str) const;
    // 获取玩家信息
    string GetPlayerTable(const int round) const;
    string GetPlayerString() const;
    // 玩家状态角标（汇总状态角标）
    vector<string> GetPlayerBadges(const Player& player) const;
    // 玩家头像（状态角标环绕头像四周）
    string GetPlayerAvatar(const Player& player) const;
    // BOSS 图片图标
    string GetBossImageTag(const Boss& boss, const int size) const;
    // 地图玩家序号：按地形上色并描边
    static string GetPlayerMark(const string& marks, const GridType type, const AttachType attach);
    // 完整赛况
    string GetAllRecordHtml(const int query_pid, const int is_public) const;
    string GetAllRecordString(const int query_pid, const int is_public) const;
    string GetAllScore() const;
    // 获取四周墙壁信息
    pair<string, string> GetSurroundingWalls(const lgtbot::PlayerID pid) const;
    // BOSS 周围墙壁信息（center：中心格显示的附着）
    pair<string, string> GetBossSurroundingWalls(const int x, const int y, const AttachType center) const;

    // 玩家移动
    // last_direct：本回合上一次成功移动的方向，-1 表示无，用于树篱冲刺判定
    bool MakeMove(const lgtbot::PlayerID pid, const Direct direction, const bool hide, int& last_direct)
    {
        int d = static_cast<int>(direction);
        int cx = players[pid].x;
        int cy = players[pid].y;
        int nx = (cx + k_DX_Direct[d] + size) % size;
        int ny = (cy + k_DY_Direct[d] + size) % size;

        bool hit_wall = false;
        if (players[pid].subspace < 0) {
            // 冲刺：上一步移动在同一回合内且方向一致（撞击类墙壁判定）
            hit_wall = !PassWall(cx, cy, direction, last_direct == d);
            // 非撞墙，尝试移动箱子
            if (!hit_wall && grid_map[nx][ny].HasBox()) {
                bool box_success = BoxMove(nx, ny, k_DX_Direct[d], k_DY_Direct[d]);
                if (box_success) players[pid].achievement.visitAttach(AttachType::BOX); // 成就[乒铃乓啷]辅助
                hit_wall = !box_success;
            }
            // 撞墙
            if (hit_wall) {
                if (!hide) players[pid].NewStepRecord(direction, "撞");
                last_direct = -1;   // 撞墙打断冲刺链
                return false;
            }
        }
        // 轨迹记录
        if (!hide) players[pid].NewStepRecord(direction);
        last_direct = d;

        // 非撞墙，炸弹触发，实际不移动
        if (players[pid].bomb_trigger) {
            return true;
        }

        // 亚空间，实际不移动
        if (players[pid].InSubspace()) {
            players[pid].subspace--;
            return true;
        }

        RemovePlayerFromMap(pid);

        players[pid].x = nx;
        players[pid].y = ny;
        player_map[nx][ny].push_back(pid);

        // 成就记录
        players[pid].achievement.MakeStep(nx, ny);
        RecordExplore(pid, nx, ny);
        return true;
    }

    // 格子所在区块在 unitMaps.pos 中的序号，10x10 地图的空隙格子返回 -1
    int BlockIndex(const int x, const int y) const
    {
        const auto& pos = unitMaps.pos;
        for (int i = 0; i < static_cast<int>(pos.size()); i++) {
            if (x >= pos[i].first && x < pos[i].first + 3 && y >= pos[i].second && y < pos[i].second + 3) return i;
        }
        return -1;
    }

    // 探索分与区块分：记录走过的格子与区块，区块此前无人探索时记为第一个探索
    void RecordExplore(const lgtbot::PlayerID pid, const int x, const int y)
    {
        Score& score = players[pid].score;
        score.explore_map[x][y] = true;
        const int block = BlockIndex(x, y);
        if (block < 0 || score.explore_blocks[block] > 0) return;
        const bool first = std::none_of(players.begin(), players.end(), [&](const Player& p) { return p.score.explore_blocks[block] > 0; });
        score.explore_blocks[block] = first ? 2 : 1;
    }

    // 箱子移动
    bool BoxMove(const int b_cx, const int b_cy, const int dx, const int dy)
    {
        int b_nx = (b_cx + dx + size) % size;
        int b_ny = (b_cy + dy + size) % size;

        // 起点必须有箱子
        if (!grid_map[b_cx][b_cy].HasBox()) return false;
        // 仅存活玩家阻挡箱子，出局玩家可被覆盖
        for (auto pid : player_map[b_nx][b_ny]) {
            if (players[pid].out == 0) {
                return false;
            }
        }
        // 后方有任何附着物，则不可推动
        if (grid_map[b_nx][b_ny].Attach() != AttachType::EMPTY) return false;

        bool wall = false;
        if (dx == -1 && dy == 0)        wall = !grid_map.WallAt(b_cx, b_cy, Direct::UP).CanPass();
        else if (dx == 1 && dy == 0)    wall = !grid_map.WallAt(b_cx, b_cy, Direct::DOWN).CanPass();
        else if (dx == 0 && dy == -1)   wall = !grid_map.WallAt(b_cx, b_cy, Direct::LEFT).CanPass();
        else if (dx == 0 && dy == 1)    wall = !grid_map.WallAt(b_cx, b_cy, Direct::RIGHT).CanPass();
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

        // 支持全部箱子类型
        AttachType box_type = grid_map[b_cx][b_cy].Attach();
        grid_map[b_cx][b_cy].SetAttach(AttachType::EMPTY);
        grid_map[b_nx][b_ny].SetAttach(box_type);
        return true;
    }

    // 玩家从地图中移除（辅助功能）
    void RemovePlayerFromMap(const lgtbot::PlayerID pid)
    {
        int cx = players[pid].x;
        int cy = players[pid].y;
        auto &oldCell = player_map[cx][cy];
        auto it = std::find(oldCell.begin(), oldCell.end(), pid);
        if (it != oldCell.end()) oldCell.erase(it);
    }

    // [按钮] 按下 (x, y) 处的按钮：切换关联的门
    void PressButton(const int x, const int y)
    {
        const vector<Grid::ButtonTarget> targets = grid_map[x][y].ButtonTargetPos();
        for (const auto& target : targets) {
            if (!target.dir.has_value()) continue;
            // 两侧格子共用同一面墙，仅需切换一边
            grid_map.WallAt(x + target.dx, y + target.dy, target.dir.value()).SwitchDoor();
        }
    }

    // [单向传送门] 进入 (x, y) 处的传送门时，若出口为单向传送门则入口与出口交换
    void SwapOneWayPortal(const int x, const int y)
    {
        const pair<int, int> rel_pos = grid_map[x][y].PortalPos();
        Grid& target_grid = grid_map[x + rel_pos.first][y + rel_pos.second];
        if (target_grid.Type() == GridType::ONEWAYPORTAL) {
            grid_map[x][y].SetType(GridType::ONEWAYPORTAL);
            target_grid.SetType(GridType::PORTAL);
        }
    }

    // [传送紊乱] 全部传送门随机组成一个大环，每个传送门传送至环中的下一个，单向传送门不参与
    void ShufflePortals()
    {
        vector<pair<int, int>> portals;
        for (int x = 0; x < size; x++) {
            for (int y = 0; y < size; y++) {
                if (grid_map[x][y].Type() == GridType::PORTAL) portals.push_back({x, y});
            }
        }
        std::shuffle(portals.begin(), portals.end(), g);
        for (size_t i = 0; i < portals.size(); i++) {
            const auto [x, y] = portals[i];
            const auto [nx, ny] = portals[(i + 1) % portals.size()];
            grid_map[x][y].SetPortal(nx - x, ny - y);
        }
    }

    // 解析多步行动：将输入字符串解析为方向数组
    static string parseDirections(const string &input, vector<Direct> &out)
    {
        // 按 key 长度降序，优先匹配中文
        vector<string> keys;
        for (auto& p : direction_map) keys.push_back(p.first);
        sort(keys.begin(), keys.end(),[](const string& a, const string& b) {
            return a.size() > b.size();
        });
        vector<Direct> tmp;
        size_t i = 0;
        while (i < input.size()) {
            bool matched = false;
            for (const auto& k : keys) {
                if (i + k.size() <= input.size() &&
                    input.compare(i, k.size(), k) == 0) {
                    tmp.push_back(direction_map.at(k));
                    i += k.size();
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                size_t len = utf8CharLen(static_cast<unsigned char>(input[i]));
                return input.substr(i, len);    // 返回非法字符
            }
        }
        out = std::move(tmp);
        return "";
    }

    // 地区声响
    Sound GetSound(const Grid& grid, const SpecialEvent event) const
    {
        switch(grid.Type()) {
            case GridType::GRASS:   return Sound::SHASHA;
            case GridType::BERRY:   return event == SpecialEvent::RAINSTORY ? Sound::PAPA : Sound::SHASHA;
            case GridType::WATER:   return Sound::PAPA;
            case GridType::ONEWAYPORTAL:
            case GridType::PORTAL:  return Sound::PAPA;
            case GridType::TRAP:    return grid.TrapStatus() ? Sound::NONE : (event == SpecialEvent::RAINSTORY ? Sound::PAPA : Sound::SHASHA);
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
            return "同格";
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

    // 热浪提示（返回true提示热浪）
    bool HeatWaveNotice(const lgtbot::PlayerID pid)
    {
        int cx = players[pid].x;
        int cy = players[pid].y;
        if (grid_map[cx][cy].Type() == GridType::HEAT) {
            return false;
        }

        bool in_zone = false;

        // [热源-HEAT]: 3*3 范围
        for (int dx = -1; dx <= 1 && !in_zone; dx++) {
            for (int dy = -1; dy <= 1 && !in_zone; dy++) {
                int nx = (cx + dx + size) % size;
                int ny = (cy + dy + size) % size;
                if (grid_map[nx][ny].Type() == GridType::HEAT) {
                    in_zone = true;
                }
            }
        }
        // [小太阳-HEATBOX]: 5*5 范围
        for (int dx = -2; dx <= 2 && !in_zone; dx++) {
            for (int dy = -2; dy <= 2 && !in_zone; dy++) {
                int nx = (cx + dx + size) % size;
                int ny = (cy + dy + size) % size;
                if (grid_map[nx][ny].Attach() == AttachType::HEATBOX) {
                    in_zone = true;
                }
            }
        }

        if (in_zone && !players[pid].in_heat_zone) {
            players[pid].in_heat_zone = true;
            return true;
        }
        if (!in_zone) {
            players[pid].in_heat_zone = false;
        }
        return false;
    }

    // 玩家是否在屏蔽器附近
    bool IsNearJammer(const lgtbot::PlayerID pid)
    {
        int cx = players[pid].x;
        int cy = players[pid].y;

        bool in_zone = false;

        // [屏蔽器-JAMMERBOX]: 3*3 范围
        for (int dx = -1; dx <= 1 && !in_zone; dx++) {
            for (int dy = -1; dy <= 1 && !in_zone; dy++) {
                int nx = (cx + dx + size) % size;
                int ny = (cy + dy + size) % size;
                if (grid_map[nx][ny].Attach() == AttachType::JAMMERBOX) {
                    in_zone = true;
                }
            }
        }

        if (in_zone) return true;
        return false;
    }

    // 玩家随机传送
    void TeleportPlayer(const lgtbot::PlayerID pid)
    {
        players[pid].subspace = -1;         // 移除亚空间状态
        players[pid].in_heat_zone = false;  // 移除热浪区域状态
        players[pid].bomb_trigger = false;  // 移除炸弹触发状态
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
        for (const auto& boss : bosses) {
            forbiddenSources.push_back({boss->x, boss->y});
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
        // 过滤候选区域（同时去除 1 步内可达[金币]的格子与所有BOSS可能到达的区域）
        vector<pair<int, int>> finalCandidates;
        std::copy_if(
            candidates.begin(), candidates.end(),
            std::back_inserter(finalCandidates),
            [&](auto const& pos) {
                auto [x, y] = pos;
                if (forbidden[x][y] || CoinWithinOneStep(x, y)) return false;
                for (const auto& boss : bosses) {
                    if (ManhattanDistance(x, y, boss->x, boss->y, size) <= boss->TeleportAvoidRange()) return false;
                }
                return true;
            }
        );
    
        // 保险方案：无有效区域，直接在最大联通区域内随机
        if (finalCandidates.empty()) {
            finalCandidates = candidates;
        }
    
        std::shuffle(finalCandidates.begin(), finalCandidates.end(), g);
        pair<int, int> newPos = finalCandidates.front();
    
        RemovePlayerFromMap(pid);
    
        players[pid].x = newPos.first;
        players[pid].y = newPos.second;
        player_map[newPos.first][newPos.second].push_back(pid);
    }

    // 捕捉顺位变更
    void UpdatePlayerTarget(const Target type)
    {
        for (lgtbot::PlayerID pid = 0; pid < playerNum; ++pid) {
            if (players[pid].out > 0) continue;
            lgtbot::PlayerID target = type == Target::NEXT ? (pid + 1) % playerNum : (playerNum + pid - 1) % playerNum;
            while (players[target].out > 0 && target != pid) {
                players[target].target = -1;
                target = type == Target::NEXT ? (target + 1) % playerNum : (playerNum + target - 1) % playerNum;
            }
            players[pid].target = target;
        }
    }

    int TypeCount(const GridType type) const
    {
        int count = 0;
        for (int x = 0; x < size; x++) {
            for (int y = 0; y < size; y++) {
                if (grid_map[x][y].Type() == type) {
                    count++;
                }
            }
        }
        return count;
    }

  private:
    // 初始化区块
    void InitializeBlocks()
    {
        vector<UnitMaps::Map> selected = unitMaps.RandomSelectBlocks(block_mode, exit_num);
        // [11x11] 地图中心为逃生舱
        if (size == 11) {
            vector<int> exit_indices;
            for (int i = 0; i < static_cast<int>(selected.size()); i++) {
                if (selected[i].is_exit) exit_indices.push_back(i);
            }
            if (!exit_indices.empty() && selected.size() > 4) {
                std::swap(selected[4], selected[exit_indices[std::uniform_int_distribution<int>(0, static_cast<int>(exit_indices.size()) - 1)(g)]]);
            }
        }

        for (int i = 0; i < static_cast<int>(selected.size()) && i < static_cast<int>(unitMaps.pos.size()); i++) {
            if (selected[i].is_exit) {
                unitMaps.SetExitBlock(unitMaps.pos[i].first, unitMaps.pos[i].second, grid_map, selected[i].id, SpecialEvent::RANDOM);   // 此处特殊事件仅用于调用原始地图
            } else {
                unitMaps.SetMapBlock(unitMaps.pos[i].first, unitMaps.pos[i].second, grid_map, selected[i].id, SpecialEvent::RANDOM);    // 此处特殊事件仅用于调用原始地图
            }
            // 记录特殊地图坐标（暂不使用）
            // if (selected[i].is_special) {
            //     special_pos = unitMaps.pos[i];
            // }
        }
    }

    // 玩家从 (x, y) 向 direction 穿过墙壁：返回能否通过（撞击类墙壁结算）
    // dash（冲刺）：上一步移动与本步在同一回合内且方向一致
    bool PassWall(const int x, const int y, const Direct direction, const bool dash)
    {
        Wall& wall = grid_map.WallAt(x, y, direction);
        switch (wall.Type()) {
            case WallType::EMPTY:
            case WallType::NORMAL:
            case WallType::DOOR:
            case WallType::DOOROPEN: return wall.CanPass();
            case WallType::HEDGE:    return dash;   // [树篱] 冲刺穿过
        }
        return false;
    }

    // [传送门] 封闭判定：四周墙壁均封闭时传送门失效
    static bool IsPortalSealed(const GridMap& grid_map, const int x, const int y)
    {
        for (int d = 0; d < 4; ++d) {
            if (!grid_map.WallAt(x, y, static_cast<Direct>(d)).SealsPortal()) return false;
        }
        return true;
    }

    // 封闭传送门自动更新传送门样式
    void FixInvalidPortals(GridMap& grid_map) const
    {
        for (int x = 0; x < size; x++) {
            for (int y = 0; y < size; y++) {
                if (grid_map[x][y].Type() == GridType::PORTAL && IsPortalSealed(grid_map, x, y)) {
                    pair<int, int> pRelPos = grid_map[x][y].PortalPos();
                    grid_map[x][y].SetType(GridType::ONEWAYPORTAL);
                    grid_map[x + pRelPos.first][y + pRelPos.second].SetType(GridType::ONEWAYPORTAL);
                }
            }
        }
    }

    bool IsBlocked(const Grid& g) const
    {
        return g.Type() == GridType::HEAT ||
            g.Type() == GridType::HEART ||
            g.Attach() == AttachType::BOX ||
            g.Attach() == AttachType::HEATBOX ||
            g.Attach() == AttachType::JAMMERBOX;
    }

    // 判断(x, y)格子沿(dx, dy)方向是否连通（考虑环绕）
    bool IsConnected(int x, int y, int dx, int dy) const
    {
        int nx = (x + dx + size) % size;
        int ny = (y + dy + size) % size;
        // 当前格或目标格无法通行
        if (IsBlocked(grid_map[x][y]) || IsBlocked(grid_map[nx][ny])) {
            return false;
        }
        if (dx == -1 && dy == 0)
            return grid_map.WallAt(x, y, Direct::UP).CanPass();
        else if (dx == 1 && dy == 0)
            return grid_map.WallAt(x, y, Direct::DOWN).CanPass();
        else if (dx == 0 && dy == -1)
            return grid_map.WallAt(x, y, Direct::LEFT).CanPass();
        else if (dx == 0 && dy == 1)
            return grid_map.WallAt(x, y, Direct::RIGHT).CanPass();
        return false;
    }

    // [金币] (x, y) 在金币上或 1 步内可达金币，连通判断同 BFS
    bool CoinWithinOneStep(const int x, const int y) const
    {
        if (grid_map[x][y].Attach() == AttachType::COIN) return true;
        for (int d = 0; d < 4; ++d) {
            const int nx = (x + k_DX_Direct[d] + size) % size;
            const int ny = (y + k_DY_Direct[d] + size) % size;
            if (IsConnected(x, y, k_DX_Direct[d], k_DY_Direct[d]) && grid_map[nx][ny].Attach() == AttachType::COIN) return true;
        }
        return false;
    }

    // 每一格到最近起点的 BFS 路径距离，不可达为 INT_MAX
    vector<vector<int>> BFSRouteDistance(const vector<pair<int, int>>& sources) const
    {
        vector<vector<int>> dist(size, vector<int>(size, INT_MAX));
        queue<pair<int, int>> q;
        for (auto [x, y] : sources) {
            dist[x][y] = 0;
            q.push({x, y});
        }
        while (!q.empty()) {
            auto [x, y] = q.front();
            q.pop();
            int dx[4] = {-1, 1, 0, 0};
            int dy[4] = {0, 0, -1, 1};
            for (int i = 0; i < 4; i++) {
                int nx = (x + dx[i] + size) % size;
                int ny = (y + dy[i] + size) % size;
                if (dist[nx][ny] == INT_MAX && IsConnected(x, y, dx[i], dy[i])) {
                    dist[nx][ny] = dist[x][y] + 1;
                    q.push({nx, ny});
                }
            }
        }
        return dist;
    }

    // * 随机生成所有玩家的位置，所有位置均在最大联通区域内，每名玩家位于不同区块 *
    // 每名玩家在全部候选落点中等概率随机，按照顺序依次尝试：
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
        //      - validCells：区块内所有满足在最大联通区域、连通度 ≥ 2、1 步内可达[金币]的非逃生舱格，即候选落点
        //      - isExit：若区块内含有逃生舱，则为 true
        struct BlockCandidate {
            vector<pair<int, int>> validCells;
            bool isExit;
        };
        vector<BlockCandidate> candidateBlocks;
        for (auto pos : unitMaps.pos) {
            int bx = pos.first, by = pos.second;
            BlockCandidate block{{}, false};
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (grid_map[bx + i][by + j].Type() == GridType::EXIT)
                        block.isExit = true;
                    else if (inLargest[bx + i][by + j] && connectivityDegree(bx + i, by + j) >= 2 && !CoinWithinOneStep(bx + i, by + j))
                        block.validCells.push_back({bx + i, by + j});
                }
            }
            if (!block.validCells.empty())
                candidateBlocks.push_back(block);
        }

        // 3. 按玩家顺序回溯分配：每名玩家列出未占用区块中与相邻玩家距离≥minDist 的全部候选落点，打乱后依次尝试
        vector<pair<int, int>> assigned(playerNum);
        auto assignPlayers = [&](const vector<BlockCandidate>& blocks, const int minDist) -> bool {
            if (blocks.size() < playerNum)
                return false;
            vector<bool> used(blocks.size(), false);
            function<bool(int)> bt = [&](const int idx) -> bool {
                if (idx == playerNum)
                    return true;
                // 已确定位置的相邻玩家：上家，最后一名玩家还有下家 0 号
                vector<pair<int, int>> neighbors;
                if (idx > 0)
                    neighbors.push_back(assigned[idx - 1]);
                if (idx > 0 && idx == playerNum - 1)
                    neighbors.push_back(assigned[0]);
                const auto dist = BFSRouteDistance(neighbors);
                vector<pair<int, pair<int, int>>> options;  // (区块序号, 落点)
                for (int b = 0; b < blocks.size(); b++) {
                    if (used[b])
                        continue;
                    for (auto cell : blocks[b].validCells) {
                        if (dist[cell.first][cell.second] >= minDist)
                            options.push_back({b, cell});
                    }
                }
                std::shuffle(options.begin(), options.end(), g);
                for (auto [b, cell] : options) {
                    assigned[idx] = cell;
                    used[b] = true;
                    if (bt(idx + 1))
                        return true;
                    used[b] = false;
                }
                return false;
            };
            if (!bt(0))
                return false;
            for (int i = 0; i < playerNum; i++) {
                int x = assigned[i].first, y = assigned[i].second;
                players[i].x = x;
                players[i].y = y;
                player_map[x][y].push_back(i);
            }
            return true;
        };

        // 4. 方案1：使用非逃生舱区块，相邻玩家之间的 BFS 路径距离≥5；每个玩家落点到所有逃生舱的 BFS 路径距离≥5
        vector<BlockCandidate> candidateNonExit;
        for (auto &b : candidateBlocks) {
            if (!b.isExit)
                candidateNonExit.push_back(b);
        }
        const auto exitDist = BFSRouteDistance(exitPositions);
        vector<BlockCandidate> candidateFarFromExit;
        for (auto &b : candidateNonExit) {
            BlockCandidate far{{}, false};
            for (auto cell : b.validCells) {
                if (exitDist[cell.first][cell.second] >= 5)
                    far.validCells.push_back(cell);
            }
            if (!far.validCells.empty())
                candidateFarFromExit.push_back(far);
        }
        if (assignPlayers(candidateFarFromExit, 5))
            return;

        // 5. 方案2：使用非逃生舱区块，仅要求相邻玩家之间的 BFS 路径距离≥5（不检查逃生舱距离）
        if (assignPlayers(candidateNonExit, 5))
            return;

        // 6. 方案3：直接使用非逃生舱区块随机分配（不检查距离）
        if (assignPlayers(candidateNonExit, 0))
            return;

        // 7. 方案4：允许使用逃生舱区块
        if (assignPlayers(candidateBlocks, 0))
            return;

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

    /* ========== 样式与图片资源（实现见 board_render.h） ========== */
    string GetImageTypeFolder() const;
    string GetGridStyle(const GridType type, const AttachType attach, const bool is_bg) const;
    string GetWallStyle(const Wall& wall, const string& direction) const;
    static string GetColoredMark(const string& content, const GridType type, const AttachType attach);
    static string GetBoardStyle();
};

#include "board_render.h"      // Board 类渲染相关成员实现
