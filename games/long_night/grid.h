
enum class Direct { UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3 };

enum class Sound {
    NONE = 0,
    SHASHA = 1,
    PAPA = 2,
    BOSS = 3,
};

enum class GridType {
    SPECIAL = -1,
    EMPTY = 0,
    GRASS = 1,
    WATER = 2,
    PORTAL = 3,
    EXIT = 4,
    TRAP = 5,
    HEAT = 6,
    BOX = 7,
    ONEWAYPORTAL = 8,
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

    bool TrapStatus() const { return trap; }

    bool IsFullyEnclosed() const { return wall[0] && wall[1] && wall[2] && wall[3]; }

    template <Direct direct>
    void SetWall(const bool has_wall) { wall[static_cast<int>(direct)] = has_wall; }

	Grid& SetWall(const bool up, const bool down, const bool left, const bool right)
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

    template <Direct direct>
    bool Wall() const { return wall[static_cast<int>(direct)]; }

    GridType Type() const { return type; }

    pair<int, int> PortalPos() const { return portalRelPos; }

    bool CanGrow() const { return growable; }

  private:
	// 四周墙面（上/下/左/右）
	bool wall[4] = {false, false, false, false};
    // 区块类型
	GridType type = GridType::EMPTY;
    // 传送门相对位置（PORTAL）
    pair<int, int> portalRelPos = {0, 0};
    // 陷阱状态（TRAP）
    bool trap = true;
    // 特殊规则2草丛是否可生长
    bool growable = false;
};
