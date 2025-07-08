
#include <random>
#include <set>

struct Map {
    int size;
    int treasure;
    int bomb;
    int hp;
    int limit;
};
const Map maps[] = {
    {  6,  6,  4,  1,  4 },     // 小地图
    {  9,  9, 12,  2,  6 },     // 中地图
    { 12, 18, 18,  2,  9 },     // 大地图
    { 15, 33, 24,  2, 12 },     // 特大地图
};


class Player
{
  public:
    Player(const string &name, const string &avatar, const int &hp) : name(name), avatar(avatar), hp(hp) {}

    const string name;
    const string avatar;
    
    int hp;
    int score = 0;
    int change = 0;

    bool has_dug = false;
    pair<int, int> current;
};


const int color_count_ = 6;
const int item_count_ = 2;

enum class GridType {
    R, B, Y, O, G, P,
    TREASURE, BOMB,
};

const vector<string> GridTypeNames = {
    "R", "B", "Y", "O", "G", "P",
    "<font color='#00C8C8'>★</font>", "✹",
};
string toString(GridType type)
{
    int index = static_cast<int>(type);
    if (index >= 0 && index < GridTypeNames.size()) {
        return GridTypeNames[index];
    }
    return "UNKNOWN";
}

class Grid
{
  public:
    string GetClasses(const bool show_all) const
    {
        if (!show && !show_all) return "hidden";

        string base = (dig_count > 0) ? "current" : "show";
        if (IsEmpty()) {
            return base + " color-" + toString(type);
        } else {
            return base;
        }
    }

    string GetContent(const bool show_all) const
    {
        if (!show && !show_all) return "";
        
        if (IsEmpty()) {
            return toString(type) + to_string(num);
        } else {
            return toString(type);
        }
    }

    bool IsEmpty() const { return static_cast<int>(type) < color_count_; }

  private:
	GridType type;
    int num = 0;

    bool show = false;
    int dig_count = 0;  // 每回合清除

    friend class Board;
};


class Board
{
  public:
    Board(string ResourceDir, const int playerNum) : resource_path_(std::move(ResourceDir)), playerNum(playerNum) {}
    // 图片资源文件夹
    const string resource_path_;

    // 地图初始配置
    int size;
	// 玩家
    const int playerNum;
    vector<Player> players;
    // 地图
    vector<vector<Grid>> grid_map;

    // 初始化地图
    void Initialize(const uint32_t map_option, const vector<int> weights, const uint32_t size_option, const uint32_t treasure_option, const uint32_t bomb_option)
    {
        size = size_option ? size_option : maps[map_option].size;
        const int treasure = treasure_option ? treasure_option : maps[map_option].treasure;
        const int bomb = bomb_option ? bomb_option : maps[map_option].bomb;
        grid_map.resize(size);
        for (int i = 0; i < size; i++) {
            grid_map[i].resize(size);
        }
        generateRandomMap(weights, treasure, bomb);
    }

    // 获取玩家和地图的markdown字符串
    string GetMarkdown(const int round, const bool show_all = false) const
    {
        return GetHeadTable(round) + GetBoard(show_all) + GetButtomTable();
    }

    // 获取顶部表格
    string GetHeadTable(const int round) const
    {
        html::Table headTable(playerNum + 1, 7);
        headTable.SetTableStyle("align=\"center\" cellpadding=\"2\"");
        headTable.SetRowStyle(0, "bgcolor=\"E7E7E7\" style=\"text-align:center; font-size:12px; font-weight:bold;\"");
        headTable.MergeRight(0, 0, 2);
        headTable.Get(0, 0).SetStyle("style=\"width:280px;\"").SetContent("玩家");
        headTable.Get(0, 2).SetStyle("style=\"width:80px;\"").SetContent(HTML_COLOR_FONT_HEADER(#BD8900) "【总得分】" HTML_FONT_TAIL);
        headTable.MergeRight(0, 3, 2);
        headTable.Get(0, 3).SetStyle("style=\"width:32px;\"").SetContent("生命值");
        headTable.Get(0, 5).SetStyle("style=\"width:40px;\"").SetContent("选择");
        headTable.Get(0, 6).SetStyle("style=\"width:50px;\"").SetContent("状态变化");
        for (int pid = 0; pid < playerNum; pid++) {
            const Player& p = players[pid];
            headTable.Get(pid + 1, 0).SetStyle("style=\"width:40px;\"").SetContent(p.avatar);
            headTable.Get(pid + 1, 1).SetStyle("style=\"width:240px; text-align:left;\"").SetContent(HTML_SIZE_FONT_HEADER(2) + p.name + HTML_FONT_TAIL);
            headTable.Get(pid + 1, 2).SetStyle("style=\"width:80px;\"").SetContent(HTML_COLOR_FONT_HEADER(#BD8900) "<b>" + to_string(p.score) + "</b>" HTML_FONT_TAIL);
            if (p.hp > 0) {     // 生命值
                headTable.Get(pid + 1, 3).SetStyle("style=\"width:20px; text-align:right;\"").SetContent("<img src='file:///" + resource_path_ + "hp.png'>");
                headTable.Get(pid + 1, 4).SetStyle("style=\"width:12px; text-align:left;\"").SetContent(HTML_COLOR_FONT_HEADER(#EC4646) + to_string(p.hp) + HTML_FONT_TAIL);
            } else {
                headTable.MergeRight(pid + 1, 3, 2);
                headTable.Get(pid + 1, 3).SetStyle("style=\"width:32px;\"").SetColor("#E5E5E5").SetContent("淘汰");
            }
            headTable.Get(pid + 1, 5).SetStyle("style=\"width:40px;\"").SetContent(p.has_dug ? string(1, 'A' + p.current.first) + to_string(p.current.second + 1) : "--");
            if (p.change > 0) {     // 状态变化
                string html = HTML_COLOR_FONT_HEADER(#2E7D32) "+" + to_string(p.change) + HTML_FONT_TAIL;
                headTable.Get(pid + 1, 6).SetStyle("style=\"width:50px;\"").SetColor("#D6F5D6").SetContent(html);
            } else if (p.change < 0) {
                string html = HTML_COLOR_FONT_HEADER(#C62828) + to_string(p.change) + "<img src='file:///" + resource_path_ + "hp.png' width='12'>" HTML_FONT_TAIL;
                headTable.Get(pid + 1, 6).SetStyle("style=\"width:50px;\"").SetColor("#FAD6D6").SetContent(html);
            } else {
                headTable.Get(pid + 1, 6).SetStyle("style=\"width:50px;\"");
            }
        }
        return "### 第 " + to_string(round) + " 回合" + headTable.ToString();
    }

    // 获取地图html
    string GetBoard(bool show_all = false) const
    {
        html::Table map(size + 2, size + 2);
        map.SetTableStyle("align=\"center\" style=\"border-collapse: collapse;\"");
        // 坐标
        for (int i = 1; i < size + 1; i++) {
            map.Get(i, 0).SetStyle("class=\"pos\"").SetContent(char('A' + i - 1));
            map.Get(i, size + 1).SetStyle("class=\"pos\"").SetContent(char('A' + i - 1));
            map.Get(0, i).SetStyle("class=\"pos\"").SetContent(to_string(i));
            map.Get(size + 1, i).SetStyle("class=\"pos\"").SetContent(to_string(i));
        }
        // 方格
        for (int x = 0; x < size; x++) {
            for (int y = 0; y < size; y++) {
                const Grid grid = grid_map[x][y];
                map.Get(x + 1, y + 1).SetStyle("class=\"grid " + grid.GetClasses(show_all) + "\"").SetContent(grid.GetContent(show_all));
            }
        }
        return style + map.ToString();
    }

    // 获取底部图例
    string GetButtomTable() const
    {
        html::Table buttomTable(max(color_count_, item_count_), 4);
        buttomTable.SetTableStyle("align=\"center\" style=\"font-size:12px;\"");

        struct InfoColor {
            string fontColor;
            string bgColor;
            string label;
            string description;
        };
        const InfoColor infoColor[] = {
            { "#C91D32", "#FADADE", "R 红色", "统计周围 8 格的炸弹总数" },
            { "#0070C0", "#D9E1F4", "B 蓝色", "统计本行本列的炸弹总数" },
            { "#BD8900", "#FFF3CA", "Y 黄色", "统计周围 8 格的宝藏总数" },
            { "#C55C10", "#F9CBAA", "O 橙色", "统计周围 8 格的宝藏和炸弹总数" },
            { "#588E31", "#E3F2D9", "G 绿色", "统计周围 8 格以及本行本列的宝藏总数（不重复计算）" },
            { "#7030A0", "#F6CCFF", "P 紫色", "统计周围 8 格以及本行本列的炸弹总数（不重复计算）" },
        };
        for (int i = 0; i < color_count_; i++) {
            string html = "<font color='" + infoColor[i].fontColor + "'>" +
                            "<span style='background:" + infoColor[i].bgColor + "'>" + infoColor[i].label + "</span>：" +
                            infoColor[i].description + "</font>";
            buttomTable.Get(i, 0).SetStyle("style=\"width:350px; text-align:left\"").SetContent(html);
        }

        buttomTable.Get(0, 1).SetStyle("style=\"width:30px;\"").SetContent("<b>图例</b>");
        buttomTable.Get(0, 2).SetStyle("style=\"width:80px;\"").SetContent("<b>名称</b>");
        buttomTable.Get(0, 3).SetStyle("style=\"width:60px;\"").SetContent("<b>剩余数量</b>");
        struct InfoItem {
            string name;
            GridType type;
        };
        const InfoItem infoItem[] = {
            { "宝藏", GridType::TREASURE },
            { "炸弹", GridType::BOMB}, 
        };
        for (int i = 0; i < item_count_; i++) {
            buttomTable.Get(i + 1, 1).SetStyle("style=\"width:30px;\"").SetContent(toString(infoItem[i].type));
            buttomTable.Get(i + 1, 2).SetStyle("style=\"width:80px;\"").SetContent(infoItem[i].name);
            buttomTable.Get(i + 1, 3).SetStyle("style=\"width:60px;\"").SetContent(to_string(countLeftGridType(infoItem[i].type)));
        }
        buttomTable.Get(item_count_ + 1, 1).SetStyle("style=\"width:30px;\"").SetContent("<div style=\"width:11px; height:11px; border:2px solid red;\"></div>");
        buttomTable.MergeRight(item_count_ + 1, 2, 2);
        buttomTable.Get(item_count_ + 1, 2).SetStyle("style=\"width:140px;\"").SetContent("本回合挖掘位置");
        
        return buttomTable.ToString();
    }

    // 统计某一个类型的剩余数量
    int countLeftGridType(const GridType type) const
    {
        int count = 0;
        for (int x = 0; x < size; x++)
            for (int y = 0; y < size; y++)
                if (grid_map[x][y].type == type && !grid_map[x][y].show)
                    count++;
        return count;
    }

    // 检查坐标是否合法
	static string CheckCoordinate(string &s)
	{
		// 长度必须为2
		if (s.length() != 2 && s.length() != 3) {
			return "[错误] 输入的坐标长度只能为 2 或 3，如：A1";
		}
		// 大小写不敏感 
		if (s[0] <= 'z' && s[0] >= 'a') {
			s[0] = s[0] - 'a' + 'A';
		}
		// 检查是否为合法输入 
		if (s[0] > 'Z' || s[0] < 'A' || s[1] > '9' || s[1] < '0') {
			return "[错误] 请输入合法的坐标（字母+数字），如：A1";
		}
		if (s.length() == 3 && (s[2] > '9' || s[2] < '0')) {
			return "[错误] 请输入合法的坐标（字母+数字），如：A1";
		}
		return "OK";
	}

	// 将字符串转为一个位置pair。必须确保字符串是合法的再执行这个操作。 
	static pair<int, int> TranString(string s)
	{
		int nowX = s[0] - 'A', nowY = s[1] - '0' - 1; 
		if (s.length() == 3)
		{
			nowY = (s[1] - '0') * 10 + s[2] - '0' - 1;
		}
		pair<int, int> ret;
		ret.first = nowX;
		ret.second = nowY;
		return ret;
	}

    // 玩家行动
    pair<bool, string> PlayerAction(const PlayerID pid, string s, const int round)
    {
        string result = CheckCoordinate(s);
		if (result != "OK") return make_pair(false, result);
		const auto [X, Y] = TranString(s);
        
        if (X < 0 || X > size - 1 || Y < 0 || Y > size - 1) {
			return make_pair(false, "[错误] 挖掘位置超出了地图的范围");
		}

        return Action(pid, X, Y, round);
    }

    pair<bool, string> Action(const PlayerID pid, const int X, const int Y, const int round) {
        Player& p = players[pid];
        Grid& grid = grid_map[X][Y];
        if (grid.show) {
            switch (grid.type) {
                case GridType::TREASURE: return {false, "[错误] 想再挖一次就能多一份宝藏？可没那么简单。试试其他位置吧~"};
                case GridType::BOMB:     return {false, "[错误] 这里已经炸成渣了，再挖也只剩焦土。试试其他位置吧~"};
                default:                 return {false, "[错误] 这片空地已经被别人翻过啦。试试其他位置吧~"};
            }
        }

        string reply;
        switch (grid.type) {
            case GridType::TREASURE: reply = "你挖到到了一个【宝藏】！"; break;
            case GridType::BOMB:     reply = "糟糕！你挖到了【炸弹】！" + string(round <= 2 ? "（前 2 回合炸弹不生效）" : "");
                if (round > 2) { p.hp -= 1; p.change = -1; } break;
            default:                 reply = "你什么也没有挖到，通过探测器看到的结果为【" + grid.GetContent(true) + "】"; break;
        }
        grid.dig_count++;
        p.has_dug = true;
        p.current = {X, Y};
        return {true, reply};
    }

    void ClearRoundStatus()
    {
        for (int x = 0; x < size; x++)
            for (int y = 0; y < size; y++)
                grid_map[x][y].dig_count = 0;
        for (auto& player: players) {
            player.has_dug = false;
            player.change = 0;
        }
    }

    void UpdatePlayerStatus()
    {
        for (int pid = 0; pid < playerNum; ++pid) {
            Player& player = players[pid];
            if (!player.has_dug) continue;

            auto [X, Y] = player.current;
            Grid& grid = grid_map[X][Y];
            grid.show = true;
            
            if (grid.type == GridType::TREASURE) {
                int score = 60 / grid.dig_count;
                player.score += score;
                player.change = score;
            }
        }
    }

    int AliveCount() const
    {
        return std::count_if(players.begin(), players.end(), [](const Player& p) {
            return p.hp > 0;
        });
    }

  private:
    // 随机生成地图
    void generateRandomMap(const vector<int> weights,const int treasure, const int bomb)
    {
        int placed = 0;
        while (placed < treasure) {
            int x = rand() % size;
            int y = rand() % size;
            if (grid_map[x][y].IsEmpty()) {
                grid_map[x][y].type = GridType::TREASURE;
                placed++;
            }
        }
        placed = 0;
        while (placed < bomb) {
            int x = rand() % size;
            int y = rand() % size;
            if (grid_map[x][y].IsEmpty()) {
                grid_map[x][y].type = GridType::BOMB;
                placed++;
            }
        }

        vector<GridType> types = {
            GridType::R, GridType::B, GridType::Y, GridType::O, GridType::G, GridType::P,
        };
        for (int x = 0; x < size; x++) {
            for (int y = 0; y < size; y++) {
                Grid& grid = grid_map[x][y];
                if (!grid.IsEmpty()) continue;

                std::random_device rd;
                std::mt19937 gen(rd());
                std::discrete_distribution<> dist(weights.begin(), weights.end());
                GridType type = types[dist(gen)];

                grid.type = type;
                int num = 0;

                switch (type) {
                    case GridType::R: num = countSurroundingBombs(x, y); break;
                    case GridType::B: num = countRowColBombs(x, y); break;
                    case GridType::Y: num = countSurroundingTreasures(x, y); break;
                    case GridType::O: num = countSurroundingBombs(x, y) + countSurroundingTreasures(x, y); break;
                    case GridType::G: num = countRowColSurroundingTreasures(x, y); break;
                    case GridType::P: num = countRowColSurroundingBombs(x, y); break;
                    default: num = -1;
                }
                grid.num = num;
            }
        }
    }

    int countSurroundingBombs(const int r, const int c) const
    {
        int count = 0;
        for (int i = max(0, r - 1); i <= min(size - 1, r + 1); i++) {
            for (int j = max(0, c - 1); j <= min(size - 1, c + 1); j++) {
                if (i == r && j == c) continue;
                if (grid_map[i][j].type == GridType::BOMB) count++;
            }
        }
        return count;
    }

    int countRowColBombs(const int r, const int c) const
    {
        int count = 0;
        for (int j = 0; j < size; j++)
            if (grid_map[r][j].type == GridType::BOMB) count++;
        for (int i = 0; i < size; i++)
            if (grid_map[i][c].type == GridType::BOMB) count++;
        if (grid_map[r][c].type == GridType::BOMB) count--;
        return count;
    }

    int countSurroundingTreasures(const int r, const int c) const
    {
        int count = 0;
        for (int i = max(0, r - 1); i <= min(size - 1, r + 1); i++) {
            for (int j = max(0, c - 1); j <= min(size - 1, c + 1); j++) {
                if (i == r && j == c) continue;
                if (grid_map[i][j].type == GridType::TREASURE) count++;
            }
        }
        return count;
    }

    int countRowColSurroundingBombs(const int r, const int c) const
    {
        set<string> s;
        for (int j = 0; j < size; j++)
            if (grid_map[r][j].type == GridType::BOMB)
                s.insert(to_string(r) + "," + to_string(j));
        for (int i = 0; i < size; i++)
            if (grid_map[i][c].type == GridType::BOMB)
                s.insert(to_string(i) + "," + to_string(c));
        for (int i = max(0, r - 1); i <= min(size - 1, r + 1); i++) {
            for (int j = max(0, c - 1); j <= min(size - 1, c + 1); j++) {
                if (i == r && j == c) continue;
                if (grid_map[i][j].type == GridType::BOMB)
                    s.insert(to_string(i) + "," + to_string(j));
            }
        }
        return s.size();
    }

    int countRowColSurroundingTreasures(const int r, const int c) const
    {
        set<string> s;
        for (int j = 0; j < size; j++)
            if (grid_map[r][j].type == GridType::TREASURE)
                s.insert(to_string(r) + "," + to_string(j));
        for (int i = 0; i < size; i++)
            if (grid_map[i][c].type == GridType::TREASURE)
                s.insert(to_string(i) + "," + to_string(c));
        for (int i = max(0, r - 1); i <= min(size - 1, r + 1); i++) {
            for (int j = max(0, c - 1); j <= min(size - 1, c + 1); j++) {
                if (i == r && j == c) continue;
                if (grid_map[i][j].type == GridType::TREASURE)
                    s.insert(to_string(i) + "," + to_string(j));
            }
        }
        return s.size();
    }

    // 地图style
    const string style = R"(
<style>
    @font-face{
        font-family: 'qisinanati';
        src: url("file:///)" + resource_path_ + R"(qisinanati.ttf");
    }
    .grid {
        font-family: 'qisinanati';
        font-size: 20px;
        width: 40px;
        height: 40px;
        box-sizing: border-box;
    }
    .hidden {
        background: #000000;
        box-shadow: inset 0 0 0 1px #C2C2C2;
    }
    .show {
        box-shadow: inset 0 0 0 1px #000000;
    }
    .current {
        box-shadow: inset 0 0 0 2px red;
    }
    .pos {
        font-size: 18px;
        font-weight: bold;
        width: 40px;
        height: 40px;
        box-sizing: border-box;
    }
    .color-R { color: #C91D32; background: #FADADE; }
    .color-B { color: #0070C0; background: #D9E1F4; }
    .color-Y { color: #BD8900; background: #FFF3CA; }
    .color-O { color: #C55C10; background: #F9CBAA; }
    .color-G { color: #588E31; background: #E3F2D9; }
    .color-P { color: #7030A0; background: #F6CCFF; }
</style>
)";

};
