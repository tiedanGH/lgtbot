
#pragma once

#include <regex>
#include <span>

/* ========== 构建期开关 ========== */
// 受限内容开关（项目级通用）：0 = 仅使用合规内容；1 = 启用完整内容（包含部分敏感内容）
// 关闭时，受限条目在编译期被剔除，相应 hint 数组的大小自动缩减
#ifndef INCLUDE_RESTRICTED_CONTENT
#define INCLUDE_RESTRICTED_CONTENT 0
#endif

// 声响名称宏定义
#if INCLUDE_RESTRICTED_CONTENT
#define PAPA_STR   "啪啪"
#else
#define PAPA_STR   "啪嗒"
#endif
#define SHASHA_STR "沙沙"
#define PENGPENG_STR "砰砰"


/* ========== constants ========== */
inline constexpr int GRID_SIZE = 50;
inline constexpr int WALL_SIZE = 10;

inline constexpr std::string_view num[10] = {"⓪", "①", "②", "③", "④", "⑤", "⑥", "⑦", "⑧", "⑨"};
inline constexpr std::string_view dir_cn[4] = {"上", "下", "左", "右"};
inline constexpr int k_DX_Direct[4] = {-1, 1, 0, 0};
inline constexpr int k_DY_Direct[4] = {0, 0, -1, 1};

inline constexpr std::string_view mode_str[9] = {"自定义", "经典", "狂野", "幻变", "疯狂", "按钮", "陷阱", "空旷", "无声"};

inline constexpr int HIDE_LIMIT = 4;

inline constexpr int MAX_BOSS_COUNT = 5;    // BOSS数量上限

inline constexpr int EXTRATIMECRAD_COUNT = 3;
inline constexpr int EXTRATIMECRAD_TIME = 60;

// 网页草稿本网址
inline constexpr std::string_view DRAFT_URL = "https://tdgame.tiedan.site/long_night/";


/* ========== enum class ========== */
// 方向
enum class Direct {
    UP,     // 上
    DOWN,   // 下
    LEFT,   // 左
    RIGHT,  // 右
};

// 声响
enum class Sound {
    NONE,   // 无
    SHASHA, // 沙沙
    PAPA,   // 啪啪
    BOSS,   // [米诺陶斯] 巨大响声
};

// 地形
enum class GridType {
    EMPTY,      // 空
    GRASS,      // 树丛
    WATER,      // 水洼
    PORTAL,     // 传送门
    EXIT,       // 逃生舱
    TRAP,       // 陷阱
    HEAT,       // 热源
    ONEWAYPORTAL,   // 单向传送门
    BERRY,      // 浆果丛
    HEART,      // 巨大的心脏
};

// 地形附着
enum class AttachType {
    EMPTY,      // 无
    BUTTON,     // 按钮
    BOMB,       // 炸弹
    BOX,        // 箱子
    HEATBOX,    // 小太阳[箱子]
    JAMMERBOX,  // 屏蔽器[箱子]
    COIN,       // 金币
};

// 墙壁类型（map/wall.h）
enum class WallType {
    EMPTY,      // 空
    NORMAL,     // 墙壁
    HEDGE,      // 树篱
    DOOR,       // 门
    DOOROPEN,   // 门 (开)
};

// 墙壁附着
enum class WallAttachType {
    EMPTY,      // 无
};

// BOSS
enum class BossType {
    NONE,       // 无
    MINOTAUR,   // [米诺陶斯]
    BANGBANG,   // [邦邦]
    TYRANT,     // [暴君]
};


/* ========== Game Option enum class ========== */
enum class BlockMode {
    CUSTOM,     // 自定义
    CLASSIC,    // 经典
    WILD,       // 狂野
    TWIST,      // 幻变
    CRAZY,      // 疯狂
    BUTTON,     // 按钮
    TRAP,       // 陷阱
    OPEN,       // 空旷
    SILENT,     // 无声
};

enum class SpecialEvent {
    NONE,           // 无
    RANDOM,         // 随机
    LAZYGARDENER,   // [怠惰的园丁]
    OVERGROWTH,     // [营养过剩]
    RAINSTORY,      // [雨天小故事]
    AMBUSH,         // [十面埋伏]
};

enum class Target {
    PREVIOUS,   // 上家
    NEXT,       // 下家
};

enum class HideMode {
    NONE,   // 无
    TURN,   // 回合隐匿
    STEP,   // 单步隐匿
};

enum class StopInfo {
    NONE,       // 无
    PRIVATE,    // 私信
    PUBLIC,     // 公开
};

enum class Texture {
    CLASSIC,    // 经典材质
    RETRO,      // 复古材质
    FRESH,      // 清新材质
};


/* ========== utils ========== */
inline const std::map<std::string, Direct> direction_map = {
    {"上", Direct::UP}, {"U", Direct::UP}, {"s", Direct::UP},
	{"下", Direct::DOWN}, {"D", Direct::DOWN}, {"x", Direct::DOWN},
	{"左", Direct::LEFT}, {"L", Direct::LEFT}, {"z", Direct::LEFT},
	{"右", Direct::RIGHT}, {"R", Direct::RIGHT}, {"y", Direct::RIGHT},
};

// 图片名称
inline std::string GetGridImage(const GridType type)
{
    switch (type) {
        case GridType::EMPTY:   return "empty.png";
        case GridType::GRASS:   return "grass.png";
        case GridType::WATER:   return "water.png";
        case GridType::ONEWAYPORTAL: return "oneway_portal.png";
        case GridType::PORTAL:  return "portal.png";
        case GridType::EXIT:    return "exit.png";
        case GridType::TRAP:    return "trap.png";
        case GridType::HEAT:    return "heat.png";
        case GridType::BERRY:   return "berry.png";
        case GridType::HEART:   return "heart.png";
        default:                return "unknown.png";
    }
}

inline std::string GetAttachImage(const AttachType type)
{
    switch (type) {
        case AttachType::EMPTY:     return "";
        case AttachType::BUTTON:    return "button.png";
        case AttachType::BOMB:      return "bomb.png";
        case AttachType::BOX:       return "box.png";
        case AttachType::HEATBOX:   return "heat_box.png";
        case AttachType::JAMMERBOX: return "jammer_box.png";
        case AttachType::COIN:      return "coin.png";
    }
    return "unknown.png";
}

inline std::string GetWallImage(const WallType type, const std::string& direction)
{
    switch (type) {
        case WallType::EMPTY:       return "empty_" + direction + ".png";
        case WallType::NORMAL:      return "wall_" + direction + ".png";
        case WallType::HEDGE:       return "hedge_" + direction + ".png";
        case WallType::DOOROPEN:    return "dooropen_" + direction + ".png";
        case WallType::DOOR:        return "door_" + direction + ".png";
    }
    return "unknown_" + direction + ".png";
}

inline std::string GetWallAttachImage(const WallAttachType attach, const std::string& direction)
{
    switch (attach) {
        case WallAttachType::EMPTY: return "";
    }
    return "";
}

// 相反方向
inline Direct opposite(Direct dir)
{
    switch (dir) {
        case Direct::UP:    return Direct::DOWN;
        case Direct::DOWN:  return Direct::UP;
        case Direct::LEFT:  return Direct::RIGHT;
        case Direct::RIGHT: return Direct::LEFT;
    }
    throw std::invalid_argument("Invalid direction");
}

// 计算两点间曼哈顿距离
inline int ManhattanDistance(int x1, int y1, int x2, int y2, int size)
{
    int dx = abs(x1 - x2);
    int dy = abs(y1 - y2);
    dx = std::min(dx, size - dx);
    dy = std::min(dy, size - dy);
    return dx + dy;
}

inline bool CompareMapId(const std::string& a, const std::string& b)
{
    bool a_is_digit = std::isdigit(a[0]);
    bool b_is_digit = std::isdigit(b[0]);
    if (a_is_digit != b_is_digit)
        return a_is_digit;
    if (!a_is_digit) {
        char a_letter = std::toupper(a[0]);
        char b_letter = std::toupper(b[0]);

        if (a_letter != b_letter)
            return a_letter < b_letter;

        int a_num = std::stoi(a.substr(1));
        int b_num = std::stoi(b.substr(1));
        return a_num < b_num;
    }
    return std::stoi(a) < std::stoi(b);
}

// UTF-8字符长度
inline size_t utf8CharLen(unsigned char c)
{
    if ((c & 0x80) == 0x00) return 1;        // 0xxxxxxx
    if ((c & 0xE0) == 0xC0) return 2;        // 110xxxxx
    if ((c & 0xF0) == 0xE0) return 3;        // 1110xxxx
    if ((c & 0xF8) == 0xF0) return 4;        // 11110xxx
    return 1; // 非法 UTF-8
}

// 替换html换行为文本换行
inline std::string replace_br_with_line(std::string s)
{
    const std::string from = "<br>";
    const std::string to   = "\n";
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.length(), to);
        pos += to.length();
    }
    return s;
}

// 转义 html/markdown
inline std::string esc_html(const std::string& input)
{
    std::string result = input;
    result = regex_replace(result, std::regex("&"), "&amp;");
    result = regex_replace(result, std::regex("<"), "&lt;");
    result = regex_replace(result, std::regex(">"), "&gt;");
    result = regex_replace(result, std::regex("\""), "&quot;");
    result = regex_replace(result, std::regex("'"), "&#39;");
    return result;
}

inline std::string clean_markdown(const std::string& input)
{
    std::string result = input;
    result = regex_replace(result, std::regex(R"(\*)"), R"(\*)");
    result = regex_replace(result, std::regex(R"(_)"), R"(\_)");
    result = regex_replace(result, std::regex(R"(\[)"), R"(\[)");
    result = regex_replace(result, std::regex(R"(\])"), R"(\])");
    result = regex_replace(result, std::regex(R"(`)"), R"(\`)");
    return result;
}

// 根据系统转换文件URL
inline std::string ToFileUrl(const std::string& path)
{
    std::string url = path;
    // 兼容 Windows 反斜杠
    replace(url.begin(), url.end(), '\\', '/');

    #ifdef _WIN32
    // 如果路径开头是盘符，保证 file:///C:/... 格式
    if (url.size() >= 2 && url[1] == ':') {
        url = "file:///" + url;
    } else {
        url = "file://" + url;
    }
    #else
    url = "file://" + url;
    #endif

    return url;
}


/* ========== 规则文本 ========== */
// 「#规则 漫漫长夜 <参数>」机制细节，下标与 mygame.cc 中 k_rule_commands 的参数映射对应
inline constexpr const char* rule_details[] = {
        // [0] 机制
        R"EOF(《游戏规则和机制细节》
可选参数列表，使用「)EOF" META_COMMAND_SIGN R"EOF(规则 漫漫长夜 <参数>」查看详情：
【计分】游戏各项得分规则
【模式】区块模式与特殊玩法
【事件】区块特殊事件规则
【地形】地形&附着&墙壁机制和特殊情况
【传送】开局随机和随机传送机制
【成就】成就触发判定和特殊情况
【区块】开局区块随机相关机制
【更新】近期版本更新内容)EOF",

        // [1] 地形
        R"EOF(【地形&附着】
出生在任何地形都不会触发其效果，所有效果均在移动时触发。
【墙壁】
相邻区块组合时，边界墙壁发生冲突则只保留优先级更高的墙壁。
优先级：门(开) > 门 > 树篱 > 墙壁 > 空
【树篱】
亚空间的传送不打断冲刺，比如向左出亚空间可继续向左穿过树篱。当玩家随机传送到树篱边，不能直接穿过。
【炸弹相关】
炸弹必须要有[进入]并[离开]两步才能引爆。炸弹下到玩家上因为没有[进入]的过程，[离开]不会引爆。同理，随机传送到炸弹上，[离开]也不会引爆。
[拆弹]也需要[进入]并[停止行动]，放置炸弹直接撞墙，因为没有[进入]的过程，不会拆弹。同理，随机传送到炸弹上，直接[停止行动]也不会拆弹。
特殊：抓人时脚下有炸弹不会触发拆弹；进入逃生舱不会引爆/拆弹；隐匿状态一样会引爆炸弹；亚空间内下包会放置于传送门入口处；踩中传送门入口的炸弹后，亚空间内移动同样会引爆
【箱子】
箱子可以进入地形，但是会被后方的任意附着、墙壁、存活玩家阻挡，但不会被逃生和出局玩家阻挡
【传送门】
如果传送门开局时被四周封闭（门(开)、门也算作封闭），对应的传送门将会失效，仅发出啪啪声不会传送（相当于水洼）
【亚空间】
玩家刚进入亚空间或在亚空间内移动一步，均视作在亚空间内。停留在亚空间内，获得的四周墙壁信息为“空空空空”，玩家本身依然在传送门入口处，捕捉需要在传送门入口处才能触发。
【热源相关】
玩家出生在热浪区域，如果第1步还在热浪范围会收到提示。如果第1步进入热源，在第2步才会收到热浪提示。
【金币】
经过金币时静默拾取，首轮捕捉成功也会抢夺目标全部金币，然后双方随机传送（首轮保护）。
完整赛况中昵称旁的金币徽章只显示已公示的数量，本回合新获得（含抢夺）要等回合结束公示后才会显示。
【巨大的心脏】
隐匿状态下心跳只私信本人。当位于屏蔽器附近时，遇到心跳声可得知自己位于屏蔽器附近。
【捕捉】
捕捉对象：同格的捕捉目标优先，其次为同格被[通缉]的玩家)EOF",

        // [2] 传送
        R"EOF(【玩家随机传送】
传送至最大联通区域，不会传送至[有效逃生舱、热源周围8格]、[存活玩家周围8格]、[BOSS当前步数可到达的区域]、[1步内可达金币]，但可能传送进[失效逃生舱]区块。如果没有有效候选点，直接在最大联通区域内随机。
【开局玩家随机】
所有玩家均在同一个最大联通区域内，且位于不同区块，不会出生在[1步内可达金币]，在满足条件的格子中等概率随机。按照顺序依次尝试（BFS会被[墙壁]、[树篱]、[热源]、[巨大的心脏]、[箱子]阻挡，但不考虑传送门）：
- 方案1：使用非逃生舱区块，相邻玩家之间的BFS路径距离≥5；每个玩家落点到所有逃生舱的BFS路径距离≥5
- 方案2：使用非逃生舱区块，仅要求相邻玩家之间的BFS路径距离≥5（不检查逃生舱距离）
- 方案3：直接使用非逃生舱区块随机分配（不检查距离）
- 方案4：允许使用逃生舱区块
- 保险方案：直接从最大连通区域中随机选取位置)EOF",

        // [3] 成就
        R"EOF(【成就相关】
[乒铃乓啷]撞箱子不会计数，必须要成功推动箱子，不同的箱子视为同一种地形。引爆/拆弹均会计数，但是算作同一种类型。“单向传送门”和“普通传送门”、“浆果丛”与“树丛”视为同一种地形。
隐匿状态进入树丛等声响地形，仍可以获得[无声]相关成就。
巨大的心脏的心跳声不影响[无声]相关成就；但声响被心跳掩盖仍视为发出过声响，无法获得[无声]相关成就。
[守株待兔]可以直接停止也可以第1步撞墙，[谋定后动模式]走1步抓不会触发此成就。)EOF",

        // [4] 区块
        R"EOF(【开局区块随机】
游戏开始时，根据当前游戏模式从对应区块池随机抽取区块。
默认情况下，逃生舱数量为本局玩家人数的一半；12×12地图固定为4个。
【热源&小太阳】
当区块池中同时存在[热源]与[小太阳]时，默认仅随机保留其中一种，不会同时出现在地图中。若区块数量不足以支持该限制，则自动取消该规则，从全部区块随机。
【自定义模式】
因自定义模式区块与逃生舱数量可能任意，采用如下规则：
- 情况1：逃生舱数量不足[默认数量]，使用全部逃生舱
- 情况2：逃生舱数量充足，且区块数量充足，逃生舱为[默认数量]（优先应用热源互斥规则）
- 情况3：当逃生舱为[默认数量]时普通区块不足，使用全部普通区块，并额外抽取逃生舱补足至区块总数（必要时取消热源互斥规则）)EOF",

        // [5] 模式
        R"EOF(【区块模式】
开局先按模式抽取 12+4 个区块组成随机池，再从中抽取地图区块。
- 经典：仅使用游戏最初的经典区块
- 幻变（默认）：从轮换区块中抽取，随版本更新变化
- 狂野：从所有区块中抽取
- 疯狂：抽取 10+4 个区块，并包含 2 个特殊区块
- 按钮/陷阱/空旷/无声：从对应主题的区块中抽取，每种区块池风格不同
- 自定义：自定义游戏区块池，详见「)EOF" META_COMMAND_SIGN R"EOF(规则 漫漫长夜 区块」
【拓展】随机池扩大为 16+6，区块不足时使用全部区块，自定义区块时不生效。
【地图边长】
- 10x10：9 个区块在大地图中随机排列且不会重叠，没有区块的空隙变成普通道路。若声源在两个方向上距离相等，裁判任选 1 个方向告知
- 11x11：9 个区块之间隔着横竖各 2 条大走廊，地图边缘两侧的区块仍直接相邻，地图中心为逃生舱
- 12x12：使用 16 个区块铺满地图，逃生舱固定为 4 个
【捕捉目标】默认上家，改为下家时顺位反转。
【停止信息】默认主动停止、超时、[谋定后动]结束行动时无法获得墙壁信息，可改为私信或在公屏公开。
【点杀】默认开启：仅在行动结束时与目标同格才算捕捉，路过不会触发。关闭后路过目标即可触发捕捉。
【隐匿】获得技能“隐匿”，隐匿时的行动转为私聊进行，不会发出声响，不会触发捕捉。
- 回合隐匿：持续一回合，仅可使用 1 次
- 单步隐匿：仅作用于下一步，可使用 4 次
【大乱斗】所有的逃生舱改为随机传送，但仍会统计逃生分。
【谋定后动】每回合仅能执行一次移动，可使用多步行动指令。
【炸弹人】玩家可在公屏安置炸弹，任何人经过炸弹并离开会引爆炸弹，立即出局并 -100 分；在炸弹上结束行动可拆除炸弹。更多细节见「)EOF" META_COMMAND_SIGN R"EOF(规则 漫漫长夜 地形」
【静音】一切归于死寂……游戏中不存在任何声响。路过声响地形不会通知任何人（包括自己），巨大的心脏与 BOSS 同样被静音。
【摸黑】伸手不见五指……游戏中无法获得任何墙壁信息。开局、撞墙与停止均不会告知四周墙壁。「停止信息」不生效，BOSS 公开的墙壁信息不受影响。
【传送紊乱】斗转星移……地图中所有传送门打乱传送目标，一一对应组成一个大环，从任意传送门进入都会传送至环中的下一个传送门，对应关系整局不变。单向传送门与开局失效的传送门不参与。
【BOSS】在游戏中添加 BOSS，可同时配置多个，不同 BOSS 独立生成和行动，详见「)EOF" META_COMMAND_SIGN R"EOF(规则 漫漫长夜 BOSS」)EOF",

        // [6] 事件
        R"EOF(【特殊事件】
默认不开启，可以指定或随机抽取，开局时公布本局事件。
- 怠惰的园丁：树丛（浆果丛）将在其区块内随机位置生成（有可能生成在中间）
- 营养过剩：树丛（浆果丛）和陷阱将额外向随机 1 个方向再次生成 1 个树丛（共 8 个方向，且不可隔墙生长）
- 雨天小故事：地图中所有树丛变成水洼，陷阱和浆果丛会发出)EOF" PAPA_STR R"EOF(声
- 十面埋伏：地图中所有树丛变成陷阱，浆果丛也视为陷阱，触发时强制停止且不发出声响)EOF",

        // [7] 更新
        R"EOF(## v2.0.7 模式更新
【更新】
1、新增11x11模式：9 个区块之间隔着横竖各 2 条大走廊，地图边缘两侧的区块仍直接相邻，地图中心为逃生舱
2、新增[传送紊乱]模式：地图中所有传送门打乱传送目标，一一对应组成一个大环，从任意传送门进入都会传送至环中的下一个传送门
3、新增特殊事件——[十面埋伏]：地图中所有树丛变成陷阱，浆果丛也视为陷阱，触发时强制停止且不发出声响
4、新增区块[拓展]模式：游戏随机池改成16+6（不足时使用全部）
【调整】
玩家出生和随机传送机制变更：不再会落在[金币]或第一步可以走到[金币]的格子内
【优化】
1、地图图例新增区块变化图例：所有门随按钮同时开关的区块，额外展示按钮踩下后的样子
2、新增“更新”规则指令，快捷查看近期更新
【草稿本】
1、地图支持向四个方向无限拓展，初始位置也可向左上方拖动。存档加载原点仍和地图加载位置一致
2、支持11x11地图边框

## v2.0.6 模式更新
【新增】
新游戏模式 [摸黑]：伸手不见五指...游戏中无法获得任何墙壁信息。开局、撞墙与停止均不会告知四周墙壁，BOSS公开的墙壁信息不受影响。
（高难游戏模式，不同于游戏常规规则，请谨慎开启）
【优化&修复】
1、修复谋定后动+停止公开下，结束行动时墙壁未能公开的问题
2、新增查询游戏计分规则的规则子指令
3、优化停止公开信息显示，改为紧跟在玩家行动消息中
4、谋定后动结束行动计入完整赛况

## v2.0.5 计分规则变更
新版本计分下，游戏将关闭倍率进入测试阶段，新规则和代码均稳定后再重新开启倍率
1、移除：当前版本的 探索分 和 额外探索分
2、新增：探索区块分（2分/个），首个探索该区块的玩家额外奖励分（3分/个）
3、新增：有衰减机制的探索分（4+4+4+3+3+3+2+2+2+1+1+…+1，最多 20 步，合计 38 分）。
【重构】
1、重构游戏底层开局玩家随机：重构随机排布方案，大幅缩减玩家随机中因地图性质导致距离5的概率，从全部可行落点中等概率随机出生点，提升随机性能
2、重构10x10模式区块坐标随机：合法布局共3600种，改成枚举后再随机，大幅提升性能，新版本下将不可能出现随机地图失败)EOF",
};


/* ========== 游戏文案 ========== */
inline std::string GetRandomHint(std::span<const std::string_view> hints)
{
    return std::string(hints[rand() % hints.size()]);
}

// 撞墙提示
inline constexpr std::string_view wall_hints[] = {
    "砰！你狠狠地撞在了一堵墙上！",     // 铁蛋
    "一阵剧痛传来，你撞上了一堵墙，看来这里走不通。",
    "黑暗中，你的身体撞击了一面粗糙的墙壁。",
    "你听到一声沉闷的回响——是墙壁挡住了你的去路。",
    "墙壁在你面前横亘，仿佛无情地阻挠着前行的路。",
    "墙壁冷漠地矗立在你面前，拒绝让你通过。",
    "前方一堵高墙挡住了你的去路。",
    "你试图继续前进，但一堵墙挡住了你的去路。",
    "砰！你撞上了一堵墙，幸好没人看到。",
    "你决定挑战墙壁，结果墙壁胜利了。",
    "看来你的“穿墙术”还没练成。",
    "你试图与墙壁讲道理，但它完全不想搭理你。",
    "你向前冲去，然后优雅地和墙壁来了个亲密接触。",
    "你试图用意念穿过墙壁，然而它比你的意念还坚定。",
    "黑暗中，你的手掌摸到一堵砖墙上，你期待的邂逅没有发生。",   //  大萝卜姬
    "噢是墙壁，你情不自禁地把耳朵贴了上去，你很失望。",
    "你的手掌按在冰冷的墙面上，它仿佛正在吞噬你的温度。你是南方人啊，试试舌头吧！",
    "砰！你撞到了墙上。看来你数错了，9又3/4并不是那么好找的。",
    "你似乎走到了墙面上？快醒醒，这里并不是匹诺康尼，别做白日梦了。",   // 三月七
    "这是什么？墙壁？撞一下。\n这是什么？墙壁？撞一下。\n这是什么？墙壁？撞一下。",
    "你申请对墙壁过一个说服，dm拒绝了你。",
    "你尝试使用闪现过墙，但可惜墙体的厚度超出了你的预期。",
    "墙壁温柔的注视着你，不再言语。",
    "仿生铁蛋bot会梦到电子萝卜吗？至少墙壁不会给你答案。",
    "你在平原上走着，突然迎面遇到一堵墙，这墙向上无限高，向下无限深，向左无限远，向右无限远，这墙是什么？\n当然不可能有这样的墙，无论材质是什么，都会因为无限大的重力坍缩的。这只是那些神经兮兮的糟老头子的臆想罢了。\n不过，你确实撞在了一堵墙上。",   // H3PO4
    "黑暗中突然出现的墙壁，像是命运在说：换个方向试试？",   // 月影
    "你试图用脸测量墙壁的硬度，恭喜获得物理系荣誉学位！",
    "砰！你与墙壁进行了深入交流，结论是它比你想象的更固执。",
    "砰！脑门和墙壁的亲密接触，证明了你对探索的执着！",     // 小葵
#if INCLUDE_RESTRICTED_CONTENT
    "是一面光滑湿软的墙！快趁机误导一下对手！信我的邪，你发出了只能自己听见的" PAPA_STR "声。",   //  大萝卜姬
    "彭！靠北啦，你不看路的吗？拜托~这么大一面墙你就这样撞啊。",
#endif
};
// 第一步撞墙提示
inline constexpr std::string_view firststep_wall_hints[] = {
    "这是什么？墙壁？撞一下。\n这是什么？墙壁？撞一下。\n这是什么？墙壁？撞一下。",     // 三月七
    "你知道吗，撞击同一面墙114514次即可将其撞倒！",
    "看来有人认为自己在玩多层迷宫……别想了，这是永久墙壁。",
    "俗话说不撞南墙不回头，但有人撞了南墙也不回头。",
    "你坚信着的，是正义还是谎言？\n你推理出的，是真理还是幻像？",   // shiga
};
// 树丛提示
inline constexpr std::string_view grass_hints[] = {
    "你踏入一片树丛，枯叶和树枝在脚下" SHASHA_STR "作响。",   // 铁蛋
    "你一脚踏入了一片树丛，树叶发出了" SHASHA_STR "声，仿佛某种回应。",
    "树叶微微颤动，" SHASHA_STR "声仿佛在轻声低语。",
    "脚下传出“" SHASHA_STR "”的声音……你希望这只是树叶，而不是别的东西。",
    "你踏入一片树丛，" SHASHA_STR "声在寂静的黑暗里显得格外刺耳。",
    "你突然跌进了一片黑暗的树丛，诡异的" SHASHA_STR "声勾起了你不好的回忆。",   // 大萝卜姬
    "随着一阵" SHASHA_STR "声，密集的枝杈无情地划开了你的衣物和皮肤。",
    "你路过一片树丛，出现了野生的妙蛙种子！快使用大师球！",     // 三月七
    "你踩到了一根萝卜……等等，树林里怎么会有萝卜？",
    SHASHA_STR "，" SHASHA_STR "，这片丛林的背后，会不会住着小红帽？",
    SHASHA_STR "，你踏入了危险的树丛。这里要是藏着一个老六，可就遭了……",   // Hyacinth
#if INCLUDE_RESTRICTED_CONTENT
    "枝叶在你身上扫过，" SHASHA_STR "声中，它刮破了你的蚕丝薄衫和渔网袜。",   // 大萝卜姬
#endif
};
// 啪啪提示
inline constexpr std::string_view papa_hints[] = {
    PAPA_STR "！常在迷宫走，哪有不湿鞋？是的，你踩到了某些液体。",   // 大萝卜姬
    PAPA_STR "！你问踩的是什么液体？也许是我为那情人留下的泪。",
    PAPA_STR "！冰冷的液体渗入了你的鞋里。",
    PAPA_STR "！冰冷的涟漪在你的鞋边回荡，你想起了那个陪着铁蛋看冰块的下午。",
    PAPA_STR "！突如其来的声音让你的动作瞬间冻结；被打破的静谧仿佛被劈开的水，迅速地恢复了无声。你的敏锐好像没有得到回应。",
    PAPA_STR "！你好像踩到了地雷？低头看一看，还好，只是一些液体。",     // 三月七
    "啪！你似乎有什么东西掉了进去，可惜这里并没有河神。",
#if INCLUDE_RESTRICTED_CONTENT
    "你向前动了动，下半身传来了" PAPA_STR "声。",   // 大萝卜姬
    PAPA_STR "！是谁那么不讲公德！随地……",
    "啪！尽管你动作已经很轻，但还是发出了很大的声音。啪！你决定不管了。",
#endif
};
// 陷阱触发提示
inline constexpr std::string_view trap_hints[] = {
    "深阱垂空百尺方，足悬铁索断人肠。",     // 齐齐
    "犹似魂断垓下道，恨满胸中万古刀。",
    "你想起守株待兔的故事，只是此刻，你成为了那只兔子。",   // 纤光
    "为坠落的人类命名：_________",
#if INCLUDE_RESTRICTED_CONTENT
    "恭喜🎉被特斯拉捕获，电击调教一回合",       // 特斯拉
#endif
};
// 热浪提示
inline constexpr std::string_view heat_wave_hints[] = {
    "你感受到了迎面扑来的热浪，炽热的空气仿佛要将你吞没",   // 铁蛋
    "你感受到周围弥漫着炽热的气息！",
    "！！请注意！！局部出现厄尔尼诺现象，气温异常升高，请注意做好防中暑措施。", // 纤光
    "你感到难以忍受的炎热，“要是周围有个水池就好了…”",
    "在这座冰冷的迷宫里，你感到一阵久违的温暖，周围似乎有明亮的光源，吸引你一探究竟…？",
};
// 热源进入提示
inline constexpr std::string_view heat_core_hints[] = {
    "你的脚被高温烫伤了，刺痛让你不由得倒吸一口凉气，然而周围的空气同样炙热无比",   // 铁蛋
    "然而，光源并不总象征着安全，烈焰利用人对光明的向往，试图再次吞噬一个失落的灵魂。", // 纤光
    "oops！检测到核心温度急剧上升，即将超过阈值…准备启动自毁程序…",
    "你相信自己的铜头铁臂可以击败一切，却不知眼前的岩浆能轻易融化所有金属。",
};
// 热源公开提示
inline constexpr std::string_view heat_active_hints[] = {
    "你被滚滚热浪淹没了...周围的一切都在高温中扭曲变形，只剩下火焰的深渊。",  // 铁蛋
    "哦不！你落入了巨人萝卜的火锅池里，这下你只好成为萝卜的夜宵了。",   // 纤光
    "你失败了！\nSteve试图在岩浆中游泳。",
};
// 炸弹爆炸提示
inline constexpr std::string_view bomb_hints[] = {
    "轰——你脚下的土地猛地炸裂，烈焰与烟尘把你吞没，眼前一片黑暗。",     // 铁蛋
    "轰隆！炸弹瞬间炸响，你被高高弹起，如同流星坠入未知深渊。",
    "炽热的冲击波把你掀起，四周的碎石与烟尘如同流星雨，你在空中划出最后的弧线。",
    "耳边传来震耳欲聋的巨响，火光吞噬了一切，你的身影消失在滚滚浓烟之中。",
    "世界在强光中褪色成负片，最后涌入意识的不是疼痛，而是童年某个夏日的蝉鸣。",
    "好消息：你确实飞起来了。坏消息：是以分子扩散的形式。",
    "这可能是你最亮眼的时刻——字面意义上的。",
    "你刚刚完成了一次无需火箭推进的轨道发射。遗憾的是，没有返回舱。",
};
// 逃生舱提示
inline constexpr std::string_view exit_hints[] = {
    "你坐进了逃生舱，在启动的轰鸣声中，你想起了那句话：“不要忘了，这个世界穿透一切高墙的东西，它就在我们的内心深处，他们无法达到，也接触不到，那就是希望。”",   // 大萝卜姬
    "你忘记躺了多久，你只记得这里很温暖、舒适、令人安心。直到一股力量把你从舱内抽离；强光穿透了你稚嫩的眼皮；你哭了，你向世界宣告着你的降临。也许你只是此刻降生的其中之一，但在她眼里，你就是她的唯一。",
    "是的。夜色再浓，也挡不住黎明的到来，就像再大的困难也挡不住我们的前进。黑暗即将过去，曙光就在前头！",
    "“我们城市崇尚和平，人人都善良和谐自觉拥护一切美好。是因为我们坚信堵不如疏。每年的今天我们都会举办 [长夜节]。在这一天我们可以放下所有原则，释放心中的恶意并且不被追究责任。”今年优胜的你当然可以如此说道。",
    "“恭喜逃生！请选择：①清空记忆并返回现实②复活所有选手重新逃杀”\n“②”你斩钉截铁道。\n无论还要重来多少次，我只要那个有你的未来。",
    "你和同伴睁开眼睛，熟练且迅速地摘下入耳式共享梦境机。\n“我觉得我们应该给足暗示了吧”同伴迅速地收拾着周围散落的仪器。\n“给的足足的，关键是他会认为是自己这样想的。他一定会给我们升至少两个职级。”你立马摘下床上目标的梦境机，与同伴一起安静又麻利地从窗户离开了卧室。",
    "最后一步了。真没想到，黑暗的旅途里，反复牵动我的却是射艺社师兄的话。那晚，在人工湖边，我忍不住把对未来的迷茫向他倾诉。他时而点头，听后认真地讲述着他曾经同样的困惑。他并没有教我如何解决，却梳理干净了我的处境。最后，他面向湖面拉紧弓，说：“尽管试试吧。”嗖——箭扎进水中，散开圈圈涟漪。“只要向湖中射箭，就一定会中十环。不是吗？”太中二了，怎么会有这么中二的人。",
    "你出生后 时间就已所剩无几\n在妈妈离世之后 我不知对你倾注了多少的爱呢\n但是你的微笑让爸爸备受鼓舞呀(^_^)\n其实要是能一起走就好了 但没能做到\n希望你能忘记一切继续前行 你一定可以做到的",    // 纤光
    "当逃生舱门缓缓关闭，伴随着沉闷的启动声，黑暗迷宫逐渐远去。依靠在逃生舱内冷冽的仪表光芒中，你仿佛听见遥远星辰的低语：“未来，总为勇者留下一缕希望。”",   // 铁蛋
    "舱门缓缓关闭，逃生舱的指示灯一一亮起，冰冷的金属包裹着你，但比起外面的黑暗，这里却意外地令人安心。你知道，一切都已经结束，或许，也是一切的开始。",
    "你说的对，但是《漫漫长夜》是由大萝卜姬自主研发的一款全新大逃杀游戏。游戏发生在一个被称作“黑暗迷宫”的地图，在这里，被铁蛋选中的人将被授予“树丛”，导引“" SHASHA_STR "”之力。你将扮演一位名为“狩猎者”的神秘角色，在自由的旅行中邂逅性格各异、能力独特的墙壁，和它们一起阻拦对手，找到隐匿的“逃生者”——同时，逐步发掘“逃生舱”的真相。",   // 三月七
    "进入逃生舱后，随着几下逐渐变弱的震动，周围的环境随之稳定下来。也就在这时，你眼前闪过一道白光，似乎是这使得你进入了一个全新的环境，伴随着的还有来自外部的一阵欢呼声：“太好了！成功抓住宝可梦了！”",     // faust
    "当逃生舱的舱门关闭时，你才发现手中的钥匙根本不属于这里。系统提示音冰冷地重复着：身份验证失败。原来从一开始，你就只是这个迷宫的装饰品而已。",   // 月影
    "逃生舱启动的瞬间，你突然想起那个古老的预言：'逃出迷宫的人将获得永生，但代价是永远孤独'。舱体剧烈震动起来，不知是故障还是某种警告...",
    "舱内显示屏突然亮起：'恭喜您成为第1024位逃生者！作为奖励，系统将向您展示迷宫的真相...'画面切换的瞬间，你看到了无数个一模一样的逃生舱，里面坐着无数个一模一样的你。",
    "当逃生舱启动时，你突然明白：黑暗不是终点，而是黎明前的温柔。舱内温度逐渐升高，那不是故障，而是新生的心跳。",
    "逃生舱启动的轰鸣声渐渐平息，取而代之的是轻柔的摇篮曲。透过舷窗，你看见繁星组成的银河缓缓流动——原来迷宫的出口，一直连接着整片宇宙。",
    "当舱门完全关闭的瞬间，你听见系统轻声说：'恭喜，这是第1024次模拟。根据数据，你这次终于选择相信自己了。' 周围突然亮起温暖的阳光，原来真正的逃生舱，一直都在你心里。",
    "逃生舱的显示屏突然亮起一行字：'记住，黑暗只是光明的候车室。' 随着这句话，整个舱体开始散发出柔和的金色光芒，照亮了通往新世界的道路。",
    "逃生舱门关了。\n那个声音还在说：你赢了，这是公平的较量，感谢我给了你逃出来的机会吧。\n你以为我逃出来是你放了手？不。是我在每一面死墙上凿出了自己的路，在每一个人的呼吸声里记住了他们的脚步。\n我不承认追杀能被包装成游戏，不承认那些岔路是“仁慈”，不承认迷宫的规则。\n\n“我要否定漫漫长夜！”",     // shiga
    "从迷宫中醒来那一刻起，危险便常伴我左右。\n第一次见到他，是在三岔路口。身后米诺陶斯的呼吸声越来越重，我茫然不知该选哪条路。他默默站在一条路尽头，示意我过去。\n后来，他出现在更多地方：在炸弹上，他迫使我停下；在箱子区，他提醒我堵路；在岩浆前，他伸手拦住我的脚步。\n逃生舱门口，他指引我正确的入口，无声地催促。光亮中我用力回头，只看见他仿佛带着微笑，缓缓消散。\n“谢谢你，皮特托先生。”",
    "我可是深信着，我的逃生一定能够成为离开迷宫的参考。\n如果真的这样……\n赞美我吧。\n把我的丰功伟业传下去，铸造我的铜像，敬仰我。\n称呼我……超高校级的逃生者。",
#if INCLUDE_RESTRICTED_CONTENT
    "躺在逃生舱内，平日并不虔诚的你颤巍巍地画着十字，双手合十，嘴里念念有词。诸如什么真主阿拉耶稣基督释迦牟利急急如律令之类。前窗仿佛响应了你的号召，一阵白色闪光迅速笼罩了你。正当你诧异得到了哪位神仙的庇佑时，眼前浮现出两个大字。一个振奋人心的声音在你耳边响起：“原神，启动！”",   // 大萝卜姬
    "一阵失重后，舱门终于打开。随着刺眼的白光，在指缝间你看见几个面目可憎的巨人在围观你。很快地你被巨大的餐叉粗暴地刺穿；顾不及对痛觉反应，你便殒命在血盘大口之中。健硕、坚定、智慧、乐观，这些优秀的品质在他们嘴里同样珍贵。",
#endif
};
// 捕捉提示
inline constexpr std::string_view catch_hints[] = {
    "星光黯淡，你们的相遇，是命中注定，亦是命终注定。",     // 纤光
    "你化作一道黑影，在血月之下，无情地终结了又一条生命。",
    "你想触碰一切的真相，但在对方空洞无神的双眼中，你没能找到答案。",
    "你叹了口气，在黑暗森林里，你不得不这样做。",
    "感谢你为了我自愿放弃逃生资格",   // 克里斯丁
#if INCLUDE_RESTRICTED_CONTENT
    "我说我杀人不眨眼，你问我眼睛干不干？永别了",   // 克里斯丁
#endif
};
// 同格树丛声响提示
inline constexpr std::string_view grass_sound_hints[] = {
    "你听见有人进入了你所在的树丛，他从旁边匆匆走过，没有发现你。阴暗的想法在你心里成长起来，是让他帮你探路，还是直接干掉。甚至运气好的话，抢在前面牛了他的逃生舱……（额外探索分只有1分并逃生一事在漫漫长夜中亦有记载）",    // Hyacinth
#if INCLUDE_RESTRICTED_CONTENT
    "你听见有人进入了你的小树丛，" SHASHA_STR "声很近很近；一股接一股热气扑向了你的耳朵；呼…哈……呼…哈……他好像很累的样子。积极地想，他也许没有察觉到你",   // 大萝卜姬
#endif
};
// 同格啪啪声响提示
inline constexpr std::string_view papa_sound_hints[] = {
    "同一片液体里溅起了另一阵" PAPA_STR "声，近得只剩一臂之遥。你屏住呼吸，听那脚步从身旁缓缓掠过——他似乎并未察觉你的存在。",   // 铁蛋
#if INCLUDE_RESTRICTED_CONTENT
    "“啪！”你汗毛直立，有人来了。。幸好，你并没有站在中间，多疑多虑的性格给了你久违的回报。你小心翼翼地蹲了下来，尽力减少接触概率。只是黑暗中你低估了脚下液体的深度。“等他走远，再把内裤脱了吧。。”你暗暗地想。",   // 大萝卜姬
    "啪！啪！看来是有人来了。两个人，狭小的隔间，不间断地" PAPA_STR "声……'淫秽的人！'你的脑海回想起了她的声音。是啊。我承认，我确实有点想她了。",
#endif
};
// 无逃生舱最后生还
inline constexpr std::string_view withoutE_win_hints[] = {
    "官方简单地公示了那次事件仅有一人生还，只有你知道你都经历了些什么。每当你听到树丛的“" SHASHA_STR "”声和水的“" PAPA_STR "”声，你都不由自主回想起那个时候。幸好，现在的你已经安全了。",     // shiga
#if INCLUDE_RESTRICTED_CONTENT
    "我睁开了双眼，眼前的一切既熟悉又陌生。看来这次终于是我赢了。我用力地端详着周遭的一切，试图捕捉错过的几日时光的任何蛛丝马迹。“我真希望他们彻底离开了 ......”说完，我在床脚拿起了本该在枕边的剃须刀；“看来上次赢的是萝卜。”我下意识地抹了抹嘴唇。在指尖晕开的口红证实了我的猜测。我笑了。",     // 大萝卜姬
    "你醒啦？现在已经是第二天了哦。\n明媚的阳光照进迷宫，耳旁传来小鸟的叫声，一切美好的不太真实，唯有眼前冰冷的血迹，无声的诉说着昨晚的那场噩梦，而有些人，永远留在了那场梦中。\n可你，真的从中逃出来了吗？\n“地形参数设置完毕，新的循环正在重启……”",   // 纤光
#endif
};
// 有逃生舱但死斗取胜
inline constexpr std::string_view withE_win_hints[] = {
    "“已经没事啦~”她温柔地从背后把你抱住，轻轻抚摸着头。“你很勇敢，这一步太不容易了。”你转过身，紧紧埋进她柔软的身体里放肆哭泣。“一切都结束了。不用再害怕了。”她低下头凑近你的耳边，“咱们回家叭…”",     // 大萝卜姬
    "多年以后，在家族背景下你在事业上取得了巨大成就。大家把你的性情大变归功于当年失踪逃生的经历。“之前那个玩世不恭的我已经死了。”你每次都这样认真回答大家。至于细节的提问嘛，失忆这个理由你很喜欢。",
};
// 浆果丛私信提示
inline constexpr std::string_view berry_hints[] = {
    SHASHA_STR "！不对，这是【浆果丛】！你闻了闻手指刚被荆棘划开的伤口，那丝丝的果甜让你非常确信。",
    SHASHA_STR "！是【浆果丛】！这微甜和刺痛，像极了与他一起的时光。噢！我想你了，我的浆果丛男孩。",
    SHASHA_STR "！【浆果丛】？！和树丛一样的" SHASHA_STR "声，但是只有我知道它是浆果丛是吗？好啊，我有了阴暗的想法。",
#if INCLUDE_RESTRICTED_CONTENT
    SHASHA_STR "！【浆果丛】熟悉的触觉和气味，让你回想起那个和她一起躲藏其中的下午。紧闭的双眼，潮红的脸颊，笨拙地嘟起的双唇，一幕幕恍如昨日。",
#endif
};
// [巨大的心脏] 屏蔽器范围内心跳提示（每回合仅第一声）
inline constexpr std::string_view heart_jammer_hints[] = {
    "是【屏蔽器】？耳朵已经熟悉了这个富有节律的心跳BGM，突然的寂静让你无所适从。",
    "看来是【屏蔽器】。地面借助脚掌传来了固定节奏的震颤，真是即使身处寂静也无法让人忽视这喷薄而出的生命力。",
};


/* ========== 声响信息 ========== */
// 声响私信文案：jammed 为被屏蔽器屏蔽；direction 为"同格"时使用同格文案
inline std::string GetSoundMessage(const Sound sound, const std::string& step_info, const std::string& boss_tag,
                                   const std::string& direction, const bool jammed)
{
    if (jammed) {
        std::string message;
        switch (sound) {
        case Sound::SHASHA: message = step_info + "你感到地面有所振动，那是踩动草木（" SHASHA_STR "）特有的动静；但周围？却没有任何声音？"; break;
        case Sound::PAPA:   message = step_info + "你感到地面有所振动，那是踩动水体（" PAPA_STR "）特有的动静；但周围？却没有任何声音？"; break;
        case Sound::BOSS:   message = boss_tag + " 你感到地面正在剧烈振动！但是，声响好像来自四面八方？"; break;
        default:            message = "[错误] 未知声音类型：被屏蔽的未知声音";
        }
        return message + "\n声响方向被屏蔽，当前位置附近存在【屏蔽器】";
    }
    if (direction == "同格") {
        switch (sound) {
        case Sound::SHASHA: return step_info + "\n" + GetRandomHint(grass_sound_hints);
        case Sound::PAPA:   return step_info + "\n" + GetRandomHint(papa_sound_hints);
        default:            return "[错误] 未知声音类型：相同格子的未知声音";
        }
    }
    switch (sound) {
    case Sound::SHASHA: return step_info + "你听见了来自【" + direction + "方】的" SHASHA_STR "声！";
    case Sound::PAPA:   return step_info + "你听见了来自【" + direction + "方】的" PAPA_STR "声！";
    case Sound::BOSS:   return boss_tag + " 你听见了来自【" + direction + "方】的巨大响声！";
    default:            return "[错误] 未知声音类型：不同格子来自【" + direction + "方】的未知声音";
    }
}
