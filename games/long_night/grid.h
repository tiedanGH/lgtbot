
enum class Direct {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
};

enum class Sound {
    NONE,
    SHASHA,
    PAPA,
    BOSS,
};

enum class Wall {
    EMPTY,
    NORMAL,
    DOOR,
};

enum class GridType {
    SPECIAL,
    EMPTY,
    GRASS,
    WATER,
    PORTAL,
    EXIT,
    TRAP,
    HEAT,
    BOX,
    BUTTON,
    ONEWAYPORTAL,
};

const map<string, Direct> direction_map = {
    {"上", Direct::UP}, {"U", Direct::UP}, {"s", Direct::UP},
	{"下", Direct::DOWN}, {"D", Direct::DOWN}, {"x", Direct::DOWN},
	{"左", Direct::LEFT}, {"L", Direct::LEFT}, {"z", Direct::LEFT},
	{"右", Direct::RIGHT}, {"R", Direct::RIGHT}, {"y", Direct::RIGHT},
};

const string num[10] = {"⓪", "①", "②", "③", "④", "⑤", "⑥", "⑦", "⑧", "⑨"};

const int hide_limit = 4;

const char* score_rule = R"(
　【抓人分】<br>
抓人+100，被抓-100<br>
　【逃生分】<br>
第1/2/3/4个逃生+150/200/250/250<br>
　【探索分】<br>
每探索一个自己未探索的格子+1<br>
每探索一个所有玩家未探索的格子额外+1<br>
【热源惩罚】因热源出局-150分<br>
【退出惩罚】强制退出-300分<br>)";

Direct opposite(Direct dir)
{
    switch (dir) {
        case Direct::UP: return Direct::DOWN;
        case Direct::DOWN: return Direct::UP;
        case Direct::LEFT: return Direct::RIGHT;
        case Direct::RIGHT: return Direct::LEFT;
    }
    assert(false);  // Invalid direction
}


class Score
{
  public:
    Score(const int size)
    {
        explore_map.resize(size);
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                explore_map[i].push_back(0);
            }
        }
    }
    // 抓人分
    int catch_score = 0;
    // 逃生分
    static constexpr const int exit_order[4] = {150, 200, 250, 250};
    int exit_score = 0;
    // 探索分   
    vector<vector<int>> explore_map;
    // 退出惩罚
    int quit_score = 0;

    pair<int, int> ExploreCount() const
    {
        int count1 = 0, count2 = 0;
        for (const auto &row : explore_map) {
            count1 += count(row.begin(), row.end(), 1);
            count2 += count(row.begin(), row.end(), 2);
        }
        return make_pair(count1, count2);
    }

    static string ScoreInfo() { return score_rule; }

    int FinalScore() const { return catch_score + exit_score + ExploreScore() + quit_score; }

  private:
    int ExploreScore() const
    {
        auto [c1, c2] = ExploreCount();
        return c2 * 2 + c1 * 1;
    }
};


class Player
{
  public:
    Player(const PlayerID pid, const string &name, const string &avatar, const int size)
        : pid(pid), name(name), avatar(avatar), score(size) {}

    // 玩家信息
    const PlayerID pid;     // 玩家ID
    const string name;      // 玩家名字
    const string avatar;    // 玩家头像
    // 出局（1被抓 2出口）
    int out = 0;
    // 当前坐标
    int x, y;
    // 抓捕目标
    PlayerID target;
    // 移动相关
    int subspace = -1;      // 亚空间剩余步数
    string move_record;     // 当前回合行动轨迹
    string all_record;      // 历史回合行动轨迹
    string private_record;  // 当前回合私信记录
    int hide_remaining = 0; // 隐匿剩余次数
    bool inHeatZone = false;// 在热源区块内
    bool heated = false;    // 两次烫伤出局
    // 挂机状态（等待时间缩减）
    bool hook_status = false;
    // 玩家分数
    Score score;
};


class Grid
{
  public:
    void PortalTeleport(Player& player) const
    {
        player.x += portalRelPos.first;
        player.y += portalRelPos.second;
        player.subspace = -1;
    }

    void TrapTrigger() { trap = !trap; }

    void switchDoor(const Direct dir)
    {
        Wall& target = wall[static_cast<int>(dir)];
        target = (target == Wall::DOOR) ? Wall::EMPTY : Wall::DOOR;
    }

    bool TrapStatus() const { return trap; }

    bool IsFullyEnclosed() const
    {
        return wall[0] == Wall::NORMAL && wall[1] == Wall::NORMAL && wall[2] == Wall::NORMAL && wall[3] == Wall::NORMAL;
    }

    void HideSpecialWalls()
    {
        for (int i = 0; i < 4; i++)
            if (wall[i] != Wall::EMPTY) wall[i] = Wall::NORMAL;
    }

    template <Direct direct>
    void SetWall(const Wall new_wall) { wall[static_cast<int>(direct)] = new_wall; }

	Grid& SetWall(const Wall up, const Wall down, const Wall left, const Wall right)
    {
        wall[0] = up;
        wall[1] = down;
        wall[2] = left;
        wall[3] = right;
        return *this;
    }

    Grid& SetType(const GridType type)
    {
        this->type = type;
        return *this;
    }

    void SetGrowable(const bool growable) { this->growable = growable; }

    void SetPortal(const int relPosX, const int relPosY) { this->portalRelPos = {relPosX, relPosY}; }

    void SetButton(const int relPosX, const int relPosY, const optional<Direct> dir = nullopt)
    {
        this->buttonRelPos = {relPosX, relPosY, dir};
    }

    template <Direct direct>
    Wall GetWall() const { return wall[static_cast<int>(direct)]; }

    GridType Type() const { return type; }

    pair<int, int> PortalPos() const { return portalRelPos; }

    bool CanGrow() const { return growable; }


    // 按钮触发位置关联
    struct ButtonTarget {
        int dx;
        int dy;
        std::optional<Direct> dir;
    };

    ButtonTarget ButtonTargetPos() const { return buttonRelPos; }

    bool ContainWallType(const Wall w) const
    {
        return wall[0] == w || wall[1] == w || wall[2] == w || wall[3] == w;
    }

  private:
	// 四周墙面（上/下/左/右）
	Wall wall[4] = { Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY };
    // 区块类型
	GridType type = GridType::EMPTY;
    // 特殊规则2草丛是否可生长
    bool growable = false;

    // 传送门相对位置（PORTAL）
    pair<int, int> portalRelPos = {0, 0};
    // 按钮触发位置（BUTTON）
    ButtonTarget buttonRelPos = {0, 0, nullopt};
    // 陷阱状态（TRAP）
    bool trap = true;
};
