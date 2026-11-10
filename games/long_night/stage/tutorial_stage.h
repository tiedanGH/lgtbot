// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).
//
// 新手教程：固定地图分章节讲解规则与推理
// 1 名玩家 + 2 名电脑：陪练按预设路线行动并故意发声；人偶关在封闭格里，不行动、不留记录，只用来保持多人对局
// 由 mygame.cc 在游戏命名空间之外包含，只有开启「教程」配置时才会创建这里的阶段

#include <cmath>

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

/* ========== 配置 ========== */
inline constexpr uint32_t k_tutorial_player_num = 3;        // 玩家 + 陪练 + 人偶
inline constexpr uint32_t k_tutorial_think_time = 600;      // 回合开始时的等待秒数，长页面按阅读时间延长
inline constexpr uint32_t k_tutorial_act_time = 360;        // 开始行动后追加的秒数
inline constexpr unsigned k_tutorial_teleport_seed = 3;     // 捕捉后的传送落点固定为第 6 章起点

// 区块池沿用攻略对局，去除部分复杂区块
inline const vector<string> k_tutorial_pool = {
    "1", "2", "5", "6",
    "9", "11", "13", "14",
    "23", "24", "47", "48",
    "E2", "E4", "E5", "E6",
};
inline const vector<string> k_tutorial_layout = {"11", "13", "23", "9", "2", "47", "5", "6", "E5"};
inline constexpr pair<int, int> k_tutorial_puppet_pos = {1, 4};     // 13 镜面迷宫A 中心：四面墙
inline constexpr pair<int, int> k_tutorial_teleport_pos = {5, 5};   // 捕捉后的传送固定落点
// 陪练路线：第 4 章每次停止安排一段，走完后原地停止
inline const vector<string> k_tutorial_companion_routes = {"上", "上上上"};

/* ========== 章节与页面 ========== */
// 章节起点：玩家回合开始时的状态，跳过、跳转时玩家与电脑更新到这里
struct TutorialChapterStart
{
    int round;
    pair<int, int> player;
    pair<int, int> companion;
    bool companion_out = false;     // 陪练已被捕捉
    int companion_routes = 0;       // 陪练已走完的路线段数
    bool player_sounded = false;    // 玩家已用掉首次声响保护
};

enum class TutorialImage {
    NONE,
    POOL,               // 区块池
    START_CANDIDATES,   // 开局墙形候选
    CORNER_CANDIDATES,  // 四角排除后的候选
    ROUND1_MAP,         // 锁定出生区块
    ROUND2_MAP,         // 组合墙
    RIGHT_CANDIDATES,   // 右侧区块候选
    SOUND_DIRECTION,    // 声响方向范围
    EXCLUDE_ABOVE,      // 出生规则排除上方
    LEFT_CANDIDATES,    // 左侧区块候选
    COMPANION_FOUND,    // 锁定陪练
    RELOCATE,           // 传送后重新定位
    EXIT_SEARCH,        // 逃生舱候选
    EXIT_FOUND,         // 确认逃生舱
};

// 推理地图的标注：seen 显示真实地形；blocks 已确定区块的布局序号
struct TutorialMapMarks
{
    vector<pair<int, int>> seen;
    vector<pair<int, int>> marks;           // 玩家序号
    vector<pair<int, int>> rings;           // 圆圈
    vector<pair<int, int>> crosses;         // 红叉
    vector<pair<int, int>> shaded;          // 浅红色区域
    vector<tuple<int, int, Direct>> walls;  // 撞墙得知的墙
    vector<int> blocks = {4};
    optional<BoardArea> area;               // 只截取这块区域
};

enum TutorialPageFlag : uint8_t {
    ARM_COMPANION = 0x01,   // 玩家本回合结束后陪练走下一段路线
    SEED_TELEPORT = 0x02,   // 本回合捕捉后的传送落点固定
};

struct TutorialPage
{
    const char* title;
    const char* rule = "";
    const char* detail = "";
    const char* reasoning = "";
    const char* action = "";
    const char* hint = "";
    TutorialImage image = TutorialImage::NONE;
    uint8_t flags = 0;
};

struct TutorialChapter
{
    const char* name;
    TutorialChapterStart start;
    vector<TutorialPage> pages;
};

// 文案中的 {陪练} {人偶} {方向} {墙形} 在发送时替换
inline const vector<TutorialChapter> k_tutorial_chapters = {
    {.name = "认识迷宫", .start = {.round = 1, .player = {5, 3}, .companion = {8, 4}}, .pages = {
        {.title = "目标", .rule = "迷宫里漆黑一片，你既是逃生者也是猎人：抓住自己的捕捉目标，或者找到逃生舱撤离。", .detail = "教程包含 2 名电脑：{陪练}在第四章开始行动；{人偶}不会行动。"},
        {.title = "地图", .rule = "地图全程不可视。它由下图区块池里的 9 个区块拼成 9×9，其中 1 个是逃生舱区块。", .detail = "地图的上下、左右边缘相连，从最右边走出去会回到最左边。随时发送「地图」可以重看区块池。", .image = TutorialImage::POOL},
        {.title = "开局墙壁", .rule = "开局时私信会告诉你所在格子四周的墙，按「上下左右」的顺序。你收到的是「空墙空墙」：上、左没有墙，下、右有墙。", .detail = "之后只有撞墙才会再收到墙壁信息，主动停止拿不到。"},
        {.title = "辨识初始形状", .detail = "记住它们在区块中的位置，这决定往哪走能最快排除。区块交界处的墙由两边合成，实际候选可能更多。", .reasoning = "在区块池里找同样墙形的格子：2、5、6、13、23 号区块都有，见图中圆圈。E6 也有，但出生点不会在逃生舱区块，可以排除。", .image = TutorialImage::START_CANDIDATES},
    }},
    {.name = "声响与位置", .start = {.round = 1, .player = {5, 3}, .companion = {8, 4}}, .pages = {
        {.title = "移动", .rule = "每次行动可以连续走任意步，每步一格，没撞墙就能一直走。", .detail = "可以一步一步发送方向，也可以一次发送多个方向，例如「左下」。", .action = "左", .hint = "也可以用 z / L"},
        {.title = "继续走", .rule = "这一步没有声音，说明左边是普通空地。", .action = "下", .hint = "x / D"},
        {.title = "啪嗒声", .rule = "踩进水洼或传送门会发出“" PAPA_STR "”声，踩进树丛是“" SHASHA_STR "”声；出生时站在上面不会发声。", .detail = "这是你本局第一次发声，受首次声响保护：别人只知道你发了声，不知道方向。"},
        {.title = "声响的代价", .rule = "多人局里，除了第一次，每次发声都会私信告诉其他玩家：声音来自他们的哪个方向。", .reasoning = "声响能帮你判断脚下的地形，也会把猎人引过来。善用首次声响保护，把它用在最关键的确认上；之后尽量绕开树丛和水洼。"},
        {.title = "四角排除", .reasoning = "这个区块池里，水洼和传送门全都在区块的四个角上。“左、下”的第 2 步就响了，说明出生点左下方是某个区块的角，只剩 2、6、13 三个候选，见图中圆圈。", .image = TutorialImage::CORNER_CANDIDATES},
        {.title = "水还是传送门", .rule = "踩进传送门会进入亚空间：再任意走 2 步，就从同区块的另一扇传送门出来，并再响一次" PAPA_STR "；水洼不会。", .reasoning = "常规做法是原路退回两步。但候选 2 和 13 的出生格本身就是传送门，退回去会踩进它，响声就说不清了，所以改为向上连走两步。", .action = "上上", .hint = "ss / UU"},
        {.title = "是水洼", .reasoning = "两步都没有声音：刚才踩到的是水洼，不是传送门。"},
        {.title = "回到出生区块", .rule = "撞墙会立即结束本回合，并私信告诉你所在格子的四周墙壁。", .action = "右上", .hint = "回到出生区块"},
        {.title = "锁定位置", .detail = "发送「预览 0 0 0 0 2 0 0 0 0」可以把推理画成草稿，0 表示未知区块；网页草稿本支持更多实用功能。", .reasoning = "出生在 6 的话，第 5 步向右会走进它左上角的传送门；出生在 13 的话，第 6 步向上会走进它左上角的水洼，都和实际不符。所以你出生在 2 号「表里空间A」的左下角，现在在它的左侧中间。", .image = TutorialImage::ROUND1_MAP},
    }},
    {.name = "传送门与组合墙", .start = {.round = 2, .player = {4, 3}, .companion = {8, 4}, .player_sounded = true}, .pages = {
        {.title = "踩回传送门", .rule = "你的出生格是 2 号区块左下角的传送门，就在你下方。从别的格子走进传送门才会触发传送。", .action = "下"},
        {.title = "亚空间", .rule = "你进入了亚空间：接下来 2 步不会撞墙，也不分方向。", .action = "左左", .hint = "2 步后离开亚空间"},
        {.title = "出传送门", .rule = "你从同区块右上角的传送门出来了，又响了一次" PAPA_STR "，这一声已经不受保护。", .reasoning = "按区块图，这个格子右边没有墙。", .action = "右"},
        {.title = "组合墙", .reasoning = "区块 2 里这格右边明明没墙，你却撞墙了：墙来自右边相邻的区块。区块交界处的墙由两边合成，任何一边有墙都过不去。所以右边区块左上角的格子左侧有墙，可以用来筛选它是哪个区块。", .image = TutorialImage::ROUND2_MAP},
        {.title = "筛选右侧区块", .reasoning = "在区块池里找左上角格子左侧有墙的区块：只有 24 和 47 符合，右边的区块就是两者之一。", .image = TutorialImage::RIGHT_CANDIDATES},
        {.title = "换个位置", .rule = "下一章要在区块 2 的右下角听声音。", .action = "下右下左上", .hint = "最后一步向上撞墙"},
        {.title = "赛况与完整赛况", .rule = "「赛况」查看本回合的行动，「完整赛况」查看所有回合的记录。", .detail = "私信发送「完整赛况」还能看到只有你自己可见的记录：每回合的墙壁信息、你听到的声响方向。现在就私信试试，看完再继续。"},
    }},
    {.name = "多人：听声辨位", .start = {.round = 4, .player = {5, 5}, .companion = {8, 4}, .player_sounded = true}, .pages = {
        {.title = "多人规则", .rule = "玩家按顺序轮流行动：每个人的行动方向都公开，但位置不公开；声响方向只私信给听到的人。", .detail = "方向共 8 种，「右上」指正上与正右之间的夹角。地图边缘相连，所以按最近的方向计算。"},
        {.title = "陪练登场", .rule = "{陪练}会按固定路线行动，并踩进树丛。", .action = "停止", .hint = "原地等它行动", .flags = TutorialPageFlag::ARM_COMPANION},
        {.title = "首次声响保护", .reasoning = "{陪练}向上走进了树丛，但这是它本局第一次发声：公屏只显示它发了声，你没有收到方向。", .action = "停止", .hint = "再听一回合", .flags = TutorialPageFlag::ARM_COMPANION},
        {.title = "听声辨位", .rule = "这次它又踩进树丛，私信告诉你：声音来自你的【{方向}方】。地图边缘相连，方向按离你更近的一侧计算，上下、左右最远都只有 4 格，所以范围是图中浅红色的 4×4 区域。", .reasoning = "树丛都在区块中心。浅红色区域里有 4 个区块中心，{陪练}就在其中一处，见图中圆圈。", .hint = "排除上方", .image = TutorialImage::SOUND_DIRECTION},
        {.title = "排除上方", .rule = "开局时每个区块最多出生 1 名玩家，不同玩家不会出生在同一个区块。", .reasoning = "{陪练}先向上 1 步踩进树丛，又向上 3 步再次踩进树丛，所以它出生在第一处树丛的正下方。如果它现在在上方区块的中心，第一处树丛就是区块 2 的中心，它会和你一样出生在区块 2，所以排除上方。", .hint = "排除左侧", .image = TutorialImage::EXCLUDE_ABOVE},
        {.title = "排除左侧", .detail = "第 1 回合你走过左侧区块的右下角两格：都是空地，右下角向上、下、右都能走，右侧中间也能向右走。", .reasoning = "符合的只有 9 和 47，中心都不是树丛。所以{陪练}不可能在左侧区块中心，如果在左上区块，第一处树丛也是左侧区块中心。所以左侧两处都排除。（首次声音没有方向依然是重要的推理信息）", .hint = "推理完成", .image = TutorialImage::LEFT_CANDIDATES},
        {.title = "锁定陪练", .reasoning = "排除后只剩区块 2 的中心：{陪练}就在你所在区块的中心，紧挨在你的左上方。", .image = TutorialImage::COMPANION_FOUND},
    }},
    {.name = "追击与捕捉", .start = {.round = 6, .player = {5, 5}, .companion = {4, 4}, .companion_routes = 2, .player_sounded = true}, .pages = {
        {.title = "捕捉规则", .rule = "你的捕捉目标是上家，也就是{陪练}。点杀模式下，行动结束时和目标在同一格才算捕捉，路过不算。", .detail = "第 1 回合的捕捉不生效，双方都会被传送；和抓你的人同格不会被抓。", .action = "左上", .hint = "前往陪练位置"},
        {.title = "撞墙捕捉", .rule = "你和{陪练}已经在同一格。点杀模式下，回合结束时同格就会捕捉成功。", .reasoning = "推荐撞墙而不是停止：撞墙同样会结束回合并触发捕捉，万一没抓到还能收到墙壁信息，停止没抓到就什么也拿不到。", .action = "右", .hint = "这格左右都是墙", .flags = TutorialPageFlag::SEED_TELEPORT},
        {.title = "捕捉之后", .rule = "捕捉成功会把你随机传送到地图其他地方，剩余玩家按顺序重新排列捕捉目标。", .detail = "你的新目标是{人偶}，在本教程中无需捕捉它。"},
        {.title = "重新定位", .reasoning = "传送后先看私信里的新墙壁信息「{墙形}」，在已知区块里找同样墙形的格子：区块 2 里只有右下角符合，你就在图中圆圈处。但实际游戏中，随机传送很可能将你传送至未知区域，则需要重新探索周边区域，并利用已知信息确定自己的位置。", .image = TutorialImage::RELOCATE},
    }},
    {.name = "寻找逃生舱", .start = {.round = 7, .player = k_tutorial_teleport_pos, .companion = {4, 4}, .companion_out = true, .companion_routes = 2, .player_sounded = true}, .pages = {
        {.title = "寻找逃生舱", .reasoning = "区块池里的逃生舱区块是 E2、E4、E5、E6，地图上只有 1 个。你右下方的区块还没去过，去那里看看。", .action = "右下", .hint = "第 2 步会走进右下区块的左上角", .image = TutorialImage::EXIT_SEARCH},
        {.title = "再走两步", .reasoning = PAPA_STR "！又是区块角上的声音。照第二章的办法再走两步：再响就是传送门。", .action = "上左"},
        {.title = "辨识逃生舱", .reasoning = "又响了：左上角是传送门。左上角有传送门的区块有 6、14 和逃生舱区块 E5「快速逃生通道A」。如果是 E5，你现在在它的右下角，逃生舱就在左上方。", .action = "左上", .image = TutorialImage::EXIT_FOUND},
    }},
};
inline const int k_tutorial_chapter_count = k_tutorial_chapters.size();
inline constexpr std::string_view k_tutorial_chapter_cn[] = {"一", "二", "三", "四", "五", "六", "七", "八", "九", "十"};

inline string TutorialChapterLabel(const int chapter)
{
    return "第" + string(k_tutorial_chapter_cn[chapter]) + "章";
}

/* ========== 阶段 ========== */
class TutorialStage;
class TutorialIntroStage;
class TutorialRoundStage;

// 拦截守卫：只匹配不符合当前提示的行动指令，其余消息照常交给回合阶段
class TutorialGuardChecker : public MsgArgChecker<string>
{
  public:
    explicit TutorialGuardChecker(const TutorialStage& tutorial) : tutorial_(&tutorial) {}
    string FormatInfo() const override { return "<行动指令>"; }
    string EscapedFormatInfo() const override { return HTML_ESCAPE_LT "行动指令" HTML_ESCAPE_GT; }
    string ColoredFormatInfo() const override { return HTML_COLOR_FONT_HEADER(green) HTML_ESCAPE_LT "行动指令" HTML_ESCAPE_GT HTML_FONT_TAIL; }
    string ExampleInfo() const override { return "上"; }
    std::optional<string> Check(MsgReader& reader) const override;
    string ArgString(const string& value) const override { return value; }

  private:
    const TutorialStage* tutorial_;
};

class TutorialStage : public SubGameStage<TutorialIntroStage, TutorialRoundStage>
{
  public:
    explicit TutorialStage(MainStage& main_stage);

    void FirstStageFsm(SubStageFsmSetter setter) override;
    void NextStageFsm(TutorialIntroStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter) override;
    void NextStageFsm(TutorialRoundStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter) override;

    PlayerID player;            // 唯一的真人玩家
    PlayerID companion;         // 陪练：玩家的捕捉目标
    PlayerID puppet;            // 人偶：保持多人对局
    int chapter = 0;
    int page = 0;
    int progress = 0;           // 当前操作页已走的步数
    bool started = false;
    bool ended = false;
    bool pending = false;       // 当前页等到玩家回合开始时发出
    bool send_walls = false;    // 从第 3 章及之后开始时，发出当前页前先私信墙壁信息
    int companion_armed = 0;    // 已安排的陪练路线段数
    int companion_done = 0;     // 陪练已走完的路线段数
    string heard_direction;     // 玩家最近一次听到陪练声响的方向
    TutorialRoundStage* round_stage = nullptr;

    const TutorialPage& CurrentPage() const { return k_tutorial_chapters[chapter].pages[page]; }
    bool Intercepts(const string& arg) const;
    string CatalogText() const;
    void PostPending();
    void OnPlayerStep(lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender);
    void OnPlayerTurnEnd();
    void RecordHeard(const Player& sound_maker);
    void End(const string& reason);

  private:
    CompReqErrCode Continue_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply);
    CompReqErrCode Hint_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply);
    CompReqErrCode Catalog_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply);
    CompReqErrCode Skip_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply);
    CompReqErrCode Jump_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply, const int target);
    CompReqErrCode Guard_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply, const string& action);

    CompReqErrCode JumpTo(const int target, lgtbot::ChildMsgSenderBase& reply);
    void SetupBoard();
    void ApplyChapterStart();
    void AdvancePage();
    void AppendPage(lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender);
    void Finish();
    string Fill(const string& text) const;
    vector<Direct> ExpectedDirections() const;
    string RemainingAction() const;
    void PlaceBlock(GridMap& map, const int i) const;                       // 按布局放置第 i 个区块
    vector<pair<int, int>> BlockCells(const int i) const;                   // 布局中第 i 个区块的全部格子
    pair<string, uint32_t> ImageHtml(const TutorialImage image) const;      // 配图 HTML 与渲染宽度
    pair<string, uint32_t> CandidateBlocks(const vector<pair<string, vector<pair<int, int>>>>& blocks, const int cols = 3) const;   // 候选区块图
    string KnownMap(const TutorialMapMarks& m) const;
};

// 准备阶段：显示目录。座位会被打乱，玩家发送「开始」时确定真人
class TutorialIntroStage : public SubGameStage<>
{
  public:
    TutorialIntroStage(MainStage& main_stage, TutorialStage& tutorial)
        : StageFsm(main_stage, "准备",
            MakeStageCommand(*this, "开始教程，可指定从第几章开始", &TutorialIntroStage::Start_,
                VoidChecker("开始"), OptionalDefaultChecker<ArithChecker<int>>(1, 1, k_tutorial_chapter_count, "章节")))
        , tutorial_(tutorial)
    {}

    void OnStageBegin() override
    {
        Global().Boardcast() << tutorial_.CatalogText();
        Global().StartTimer(k_tutorial_think_time);
    }

    AtomReqErrCode OnComputerAct(const PlayerID pid, lgtbot::ChildMsgSenderBase& reply) override
    {
        if (Global().IsReady(pid)) return StageErrCode::OK;
        return StageErrCode::READY;
    }

    CheckoutErrCode OnStageTimeout() override
    {
        tutorial_.End("等待超时");
        return StageErrCode::CHECKOUT;
    }

    CheckoutErrCode OnPlayerLeave(const PlayerID pid) override
    {
        tutorial_.End("您已退出游戏");
        return StageErrCode::CHECKOUT;
    }

  private:
    AtomReqErrCode Start_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply, const int chapter)
    {
        tutorial_.player = pid;
        tutorial_.chapter = chapter - 1;
        for (PlayerID other = 0; other < Global().PlayerNum(); ++other) {
            if (other != pid) Global().SetReady(other);
        }
        return StageErrCode::READY;
    }

    TutorialStage& tutorial_;
};

// 教程回合：玩法完全复用 RoundStage，只覆盖电脑行动、讲解插入、超时与退出
class TutorialRoundStage : public RoundStage
{
  public:
    TutorialRoundStage(MainStage& main_stage, const uint64_t round, TutorialStage& tutorial)
        : RoundStage(main_stage, round), tutorial_(tutorial)
    {
        tutorial_.round_stage = this;
        // 不需要行动的电脑不进入行动顺序
        while (currentPlayer < Global().PlayerNum() - 1 && (Main().board.players[currentPlayer].out > 0 || SkipTurn(currentPlayer))) {
            currentPlayer = currentPlayer + 1;
        }
    }

    ~TutorialRoundStage() override
    {
        if (tutorial_.round_stage == this) tutorial_.round_stage = nullptr;
    }

    void OnStageBegin() override
    {
        RoundStage::OnStageBegin();
        if (currentPlayer == tutorial_.player) tutorial_.PostPending();
    }

    CheckoutErrCode OnStageOver() override
    {
        if (currentPlayer == tutorial_.player) tutorial_.OnPlayerTurnEnd();
        const CheckoutErrCode rc = RoundStage::OnStageOver();
        if (rc == StageErrCode::CONTINUE && currentPlayer == tutorial_.player) tutorial_.PostPending();
        return rc;
    }

    CheckoutErrCode OnStageTimeout() override
    {
        tutorial_.End("行动超时");
        return StageErrCode::CHECKOUT;
    }

    CheckoutErrCode OnPlayerLeave(const PlayerID pid) override
    {
        if (pid != tutorial_.player) return StageErrCode::CONTINUE;
        tutorial_.End("您已退出游戏");
        return StageErrCode::CHECKOUT;
    }

    // 陪练按安排的路线逐步行动，走完后主动停止
    AtomReqErrCode OnComputerAct(const PlayerID pid, lgtbot::ChildMsgSenderBase& reply) override
    {
        if (Global().IsReady(pid)) return StageErrCode::OK;
        if (pid != tutorial_.companion || tutorial_.companion_done >= tutorial_.companion_armed) return StageErrCode::READY;
        Player& player = Main().board.players[pid];
        vector<Direct> directions;
        Board::parseDirections(k_tutorial_companion_routes[tutorial_.companion_done++], directions);
        for (size_t i = 0; i < directions.size(); i++) {
            if (i > 0) std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            auto sender = Global().Boardcast();
            const bool turn_over = MoveOneStep(player, directions[i], sender, true);
            tutorial_.RecordHeard(player);
            if (turn_over) return StageErrCode::READY;
        }
        step++;
        active_stop = true;
        player.NewContentRecord("(停止)");
        Global().Boardcast() << "[第 " << step << " 步] " << At(pid) << " 主动停止行动";
        return StageErrCode::READY;
    }

    void AppendWalls(lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
    {
        AppendSurroundingWalls(Main().board.players[tutorial_.player], sender);
    }

    // 跳转章节：本回合步数、隐匿、冲刺等临时状态清零
    void ResetTurn()
    {
        ResetTurnState();
        door_modified = 0;
    }

  protected:
    bool SkipTurn(const PlayerID pid) const override
    {
        return pid != tutorial_.player && !(pid == tutorial_.companion && tutorial_.companion_done < tutorial_.companion_armed);
    }

    void AfterStep(Player& player, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender) override
    {
        if (player.pid == tutorial_.player) tutorial_.OnPlayerStep(sender);
    }

  private:
    TutorialStage& tutorial_;
};

/* ========== 拦截守卫 ========== */
inline std::optional<string> TutorialGuardChecker::Check(MsgReader& reader) const
{
    if (!reader.HasNext()) return std::nullopt;
    const string arg = reader.NextArg();
    if (reader.HasNext() || !tutorial_->Intercepts(arg)) return std::nullopt;
    return arg;
}

/* ========== 教程阶段 ========== */
inline TutorialStage::TutorialStage(MainStage& main_stage)
    : StageFsm(main_stage, "新手教程",
        MakeStageCommand(*this, "[教程] 读完本页，进入下一步", &TutorialStage::Continue_, VoidChecker("继续")),
        MakeStageCommand(*this, "[教程] 重新查看当前页面", &TutorialStage::Hint_, VoidChecker("提示")),
        MakeStageCommand(*this, "[教程] 查看教程目录", &TutorialStage::Catalog_, VoidChecker("目录")),
        MakeStageCommand(*this, "[教程] 跳过本章：玩家和电脑更新到下一章开头的位置和状态", &TutorialStage::Skip_, VoidChecker("跳过")),
        MakeStageCommand(*this, "[教程] 跳到指定章节：玩家和电脑更新到该章开头的位置和状态", &TutorialStage::Jump_,
            VoidChecker("跳转"), ArithChecker<int>(1, k_tutorial_chapter_count, "章节")),
        MakeStageCommand(*this, "[教程] 教程中只能执行提示的行动", &TutorialStage::Guard_, TutorialGuardChecker(*this)))
{}

inline void TutorialStage::FirstStageFsm(SubStageFsmSetter setter)
{
    setter.Emplace<TutorialIntroStage>(Main(), *this);
}

inline void TutorialStage::NextStageFsm(TutorialIntroStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter)
{
    if (ended) return;
    started = true;
    SetupBoard();
    pending = true;
    send_walls = Main().round_ > 1;     // 第 1 回合开局时游戏本身会私信墙壁信息
    setter.Emplace<TutorialRoundStage>(Main(), Main().round_, *this);
}

inline void TutorialStage::NextStageFsm(TutorialRoundStage& sub_stage, const CheckoutReason reason, SubStageFsmSetter setter)
{
    round_stage = nullptr;
    if (ended) return;
    if (Main().board.players[player].out == 2) {
        Finish();
        return;
    }
    // 回合结束时完成当前操作页
    if (*CurrentPage().action) AdvancePage();
    pending = true;
    setter.Emplace<TutorialRoundStage>(Main(), ++Main().round_, *this);
}

inline void TutorialStage::PlaceBlock(GridMap& map, const int i) const
{
    const UnitMaps& unit_maps = Main().board.unitMaps;
    const auto [x, y] = unit_maps.pos[i];
    const string& id = k_tutorial_layout[i];
    if (id[0] == 'E') unit_maps.SetExitBlock(x, y, map, id.substr(1), SpecialEvent::NONE);
    else unit_maps.SetMapBlock(x, y, map, id, SpecialEvent::NONE);
}

// 玩家固定为上家规则下陪练的捕捉者：陪练是玩家的目标，人偶是剩下的电脑
inline void TutorialStage::SetupBoard()
{
    Board& board = Main().board;
    const uint32_t n = Global().PlayerNum();
    companion = PlayerID((player.Get() + n - 1) % n);
    for (PlayerID pid = 0; pid < n; ++pid) {
        if (pid != player && pid != companion) puppet = pid;
    }
    board.size = 9;
    board.playerNum = n;
    board.players.reserve(n);
    for (PlayerID pid = 0; pid < n; ++pid) {
        const string role = pid == companion ? "（陪练）" : pid == puppet ? "（人偶）" : "";
        board.players.emplace_back(pid, StripNameBrackets(Global().PlayerName(pid)) + role, Global().PlayerAvatar(pid, 40), board.size, board.unitMaps.pos);
        board.players.back().x = board.players.back().y = 0;
        board.players.back().extra_time_card = 0;   // 超时即结束教程
    }
    board.players[puppet].hidden_in_record = true;
    board.grid_map = GridMap(board.size, true);
    board.player_map.assign(board.size, vector<vector<PlayerID>>(board.size));
    for (size_t i = 0; i < k_tutorial_layout.size(); i++) PlaceBlock(board.grid_map, i);
    board.exit_num = board.TypeCount(GridType::EXIT);
    // 陪练被捕捉时只在游戏内出局，不调用框架淘汰，跳回之前的章节时可以复活
    Main().framework_eliminated_[companion] = Main().framework_eliminated_[puppet] = true;
    ApplyChapterStart();
    board.SaveGameStartMap();
}

// 玩家与电脑重置到当前章节开头：位置、出局状态、首次声响保护、捕捉目标；清空全部行动记录
inline void TutorialStage::ApplyChapterStart()
{
    const TutorialChapterStart& start = k_tutorial_chapters[chapter].start;
    Board& board = Main().board;
    Main().round_ = start.round;
    const auto place = [&](const PlayerID pid, const pair<int, int> pos) {
        board.RemovePlayerFromMap(pid);
        board.players[pid].x = pos.first;
        board.players[pid].y = pos.second;
        board.player_map[pos.first][pos.second].push_back(pid);
    };
    place(player, start.player);
    place(companion, start.companion);
    place(puppet, k_tutorial_puppet_pos);
    for (Player& p : board.players) {
        p.subspace = -1;
        p.heated = false;
        p.in_heat_zone = false;
        p.bomb_trigger = false;
        p.hook_status = false;
        p.move_record.clear();
        p.move_record.round = start.round;
        p.all_record.clear();
        p.private_record.clear();
    }
    board.all_extra_record.clear();
    board.players[companion].out = start.companion_out ? 1 : 0;
    board.players[player].achievement.trigger_sound = start.player_sounded;
    board.players[companion].achievement.trigger_sound = start.companion_routes > 0;
    board.UpdatePlayerTarget(Target::PREVIOUS);
    companion_armed = companion_done = start.companion_routes;
    heard_direction.clear();
    progress = 0;
}

inline void TutorialStage::AdvancePage()
{
    progress = 0;
    if (++page < static_cast<int>(k_tutorial_chapters[chapter].pages.size())) return;
    page = 0;
    ++chapter;
}

inline void TutorialStage::PostPending()
{
    if (!pending || ended) return;
    pending = false;
    if (send_walls && round_stage) {
        send_walls = false;
        auto tell = Global().Tell(player);
        round_stage->AppendWalls(tell);
    }
    auto sender = Global().Boardcast();
    AppendPage(sender);
}

// 回合内每步移动后：操作页的步数走完即翻页，下一页接在这一步的消息后面
inline void TutorialStage::OnPlayerStep(lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
{
    const vector<Direct> expected = ExpectedDirections();
    if (expected.empty() || ++progress < static_cast<int>(expected.size())) return;
    AdvancePage();
    sender << "\n\n";
    AppendPage(sender);
}

inline void TutorialStage::OnPlayerTurnEnd()
{
    const uint8_t flags = CurrentPage().flags;
    if (flags & TutorialPageFlag::ARM_COMPANION) ++companion_armed;
    if (flags & TutorialPageFlag::SEED_TELEPORT) Main().board.g.seed(k_tutorial_teleport_seed);
}

inline void TutorialStage::RecordHeard(const Player& sound_maker)
{
    if (sound_maker.move_record.empty()) return;
    const auto& propagation = sound_maker.move_record.round_move.back().propagation;
    if (propagation.size() > player.Get() && propagation[player] != "首次") heard_direction = propagation[player];
}

inline bool TutorialStage::Intercepts(const string& arg) const
{
    if (!round_stage || ended) return false;
    vector<Direct> directions;
    const bool is_move = direction_map.count(arg) || (Board::parseDirections(arg, directions).empty() && !directions.empty());
    if (!is_move && arg != "停止" && arg != "隐匿" && arg != "下包") return false;
    const string action = CurrentPage().action;
    if (action.empty()) return true;
    if (action == "停止") return arg != "停止";
    if (!is_move) return true;
    if (directions.empty()) directions.push_back(direction_map.at(arg));
    const vector<Direct> expected = ExpectedDirections();
    if (progress + directions.size() > expected.size()) return true;
    return !std::equal(directions.begin(), directions.end(), expected.begin() + progress);
}

inline vector<Direct> TutorialStage::ExpectedDirections() const
{
    vector<Direct> directions;
    const string action = CurrentPage().action;
    if (!action.empty() && action != "停止") Board::parseDirections(action, directions);
    return directions;
}

inline string TutorialStage::RemainingAction() const
{
    const string action = CurrentPage().action;
    if (action == "停止") return action;
    const vector<Direct> expected = ExpectedDirections();
    string remaining;
    for (size_t i = progress; i < expected.size(); i++) remaining += dir_cn[static_cast<int>(expected[i])];
    return remaining;
}

inline string TutorialStage::Fill(const string& text) const
{
    const Board& board = Main().board;
    const vector<pair<string, string>> values = {
        {"{陪练}", to_string(companion) + "号陪练"},
        {"{人偶}", to_string(puppet) + "号人偶"},
        {"{方向}", heard_direction},
        {"{墙形}", board.GetSurroundingWalls(player).first},
    };
    string filled = text;
    for (const auto& [key, value] : values) {
        for (size_t p; (p = filled.find(key)) != string::npos;) filled.replace(p, key.size(), value);
    }
    return filled;
}

// 页面：标题带章节进度，正文分规则、细节、推理，最后一行提示下一步；阅读时间按字数放宽
inline void TutorialStage::AppendPage(lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
{
    const TutorialChapter& ch = k_tutorial_chapters[chapter];
    const TutorialPage& p = ch.pages[page];
    sender << "📖【" << TutorialChapterLabel(chapter) << " " << page + 1 << "/" << ch.pages.size() << "】" << p.title;
    size_t text_size = 0;
    for (const auto& [label, text] : {pair{"规则", p.rule}, pair{"细节", p.detail}, pair{"推理", p.reasoning}}) {
        if (!*text) continue;
        const string filled = Fill(text);
        sender << "\n" << label << "：" << filled;
        text_size += filled.size();
    }
    sender << "\n👉 发送「" << (*p.action ? RemainingAction() : string("继续")) << "」";
    if (*p.hint) sender << "（" << p.hint << "）";
    if (p.image != TutorialImage::NONE) {
        const auto [html, width] = ImageHtml(p.image);
        sender << "\n" << Markdown(html, width);
    }
    // 阅读时间约每字 1 秒，只延长不缩短剩余时间
    const int64_t seconds = std::min<int64_t>(180 + text_size / 3 + (p.image != TutorialImage::NONE ? 120 : 0), 900);
    const auto& finish = Global().TimerFinishTime();
    const int64_t left = finish ? std::chrono::duration_cast<std::chrono::seconds>(*finish - std::chrono::steady_clock::now()).count() : 0;
    if (left < seconds) Global().StartTimer(seconds);
}

inline string TutorialStage::CatalogText() const
{
    string text = "📖 教程目录";
    if (started) {
        text += "（当前：" + TutorialChapterLabel(chapter) + " " + to_string(page + 1) + "/"
              + to_string(k_tutorial_chapters[chapter].pages.size()) + "）";
    }
    for (int i = 0; i < k_tutorial_chapter_count; i++) {
        text += "\n" + TutorialChapterLabel(i) + " " + k_tutorial_chapters[i].name;
        if (started && i < chapter) text += " ✓";
        if (started && i == chapter) text += " ▶";
    }
    if (!started) return text + "\n👉 发送「开始」从第一章开始，或发送「开始 N」从第 N 章开始";
    return text + "\n「跳过」跳过本章\n「跳转 N」跳到第 N 章";
}

inline void TutorialStage::End(const string& reason)
{
    if (ended) return;
    ended = true;
    auto sender = Global().Boardcast();
    sender << reason << "，教程已结束";
    if (started) sender << "（进度：" << TutorialChapterLabel(chapter) << " " << page + 1 << "/" << k_tutorial_chapters[chapter].pages.size() << "）";
    sender << "\n下次发送「" META_COMMAND_SIGN "新游戏 漫漫长夜 教程」后，再发送「开始 " << chapter + 1 << "」即可从" << TutorialChapterLabel(chapter) << "继续";
}

inline void TutorialStage::Finish()
{
    ended = true;
    Global().Boardcast() << "📖【教程完成】\n"
        "你学会了读墙壁信息、用声响排除区块、分辨水洼与传送门、听声辨位、捕捉与逃生。\n"
        "📚 完整规则：「" META_COMMAND_SIGN "规则 漫漫长夜」\n"
        "🧩 更多玩法：「" META_COMMAND_SIGN "规则 漫漫长夜 模式/事件/BOSS」\n"
        "🌹 鸣谢：漫漫长夜作者大萝卜姬、攻略作者黑桃3、攻略大图奇策十二和隐身鱼、程序开发铁蛋\n"
        << Markdown(Main().board.GetFinalBoard(), (GRID_SIZE + WALL_SIZE) * 2 * (Main().board.size + 1));
}

/* ========== 指令 ========== */
inline CompReqErrCode TutorialStage::Continue_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply)
{
    if (!started) {
        reply() << "📖 请先发送「开始」";
        return StageErrCode::FAILED;
    }
    if (*CurrentPage().action) {
        reply() << "📖 本页需要行动：发送「" << RemainingAction() << "」";
        return StageErrCode::FAILED;
    }
    AdvancePage();
    auto sender = reply();
    AppendPage(sender);
    return StageErrCode::OK;
}

inline CompReqErrCode TutorialStage::Hint_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply)
{
    if (!started) {
        reply() << "📖 发送「开始」从第一章开始，或发送「开始 N」从第 N 章开始";
        return StageErrCode::OK;
    }
    auto sender = reply();
    AppendPage(sender);
    return StageErrCode::OK;
}

inline CompReqErrCode TutorialStage::Catalog_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply)
{
    reply() << CatalogText();
    return StageErrCode::OK;
}

inline CompReqErrCode TutorialStage::Skip_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply)
{
    if (chapter + 1 >= k_tutorial_chapter_count) {
        reply() << "📖 已经是最后一章了";
        return StageErrCode::FAILED;
    }
    return JumpTo(chapter + 1, reply);
}

inline CompReqErrCode TutorialStage::Jump_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply, const int target)
{
    return JumpTo(target - 1, reply);
}

inline CompReqErrCode TutorialStage::JumpTo(const int target, lgtbot::ChildMsgSenderBase& reply)
{
    if (!started) {
        reply() << "📖 准备阶段请发送「开始 " << target + 1 << "」从" << TutorialChapterLabel(target) << "开始";
        return StageErrCode::FAILED;
    }
    if (!round_stage || round_stage->currentPlayer != player) {
        reply() << "📖 请在你的回合跳转章节";
        return StageErrCode::FAILED;
    }
    chapter = target;
    page = 0;
    pending = false;
    ApplyChapterStart();
    round_stage->ResetTurn();
    {
        auto tell = Global().Tell(player);
        round_stage->AppendWalls(tell);
    }
    auto sender = reply();
    sender << "⏩ 已跳到" << TutorialChapterLabel(chapter) << "：游戏状态已重置，新位置的墙壁信息已私信发送\n\n";
    AppendPage(sender);
    return StageErrCode::OK;
}

inline CompReqErrCode TutorialStage::Guard_(const PlayerID pid, const bool is_public, lgtbot::ChildMsgSenderBase& reply, const string& action)
{
    if (*CurrentPage().action) {
        reply() << "📖 教程中请按提示操作：发送「" << RemainingAction() << "」\n发送「提示」可重看本页";
    } else {
        reply() << "📖 请先读完本页，发送「继续」进入下一步\n发送「提示」可重看本页";
    }
    return StageErrCode::FAILED;
}

/* ========== 配图：引擎 Markdown 渲染 ========== */
// 格子覆盖层：行内块铺满格子，不额外撑高行高
inline string TutorialCellOverlay(const string& background, const string& inner)
{
    return "<span style=\"display:inline-flex; vertical-align:top; width:" + to_string(GRID_SIZE) + "px; height:" + to_string(GRID_SIZE)
           + "px; align-items:center; justify-content:center; background:" + background + ";\">" + inner + "</span>";
}

inline const string k_tutorial_ring = "<span style=\"display:inline-block; width:40px; height:40px; border:4px solid #ff4d88; "
                                      "border-radius:50%; box-sizing:border-box;\"></span>";

// 红叉
inline const string k_tutorial_cross = "<span style=\"position:relative; display:inline-block; width:40px; height:40px;\">"
    "<span style=\"position:absolute; left:17px; top:-3px; width:6px; height:46px; background:#e53935; -webkit-transform:rotate(45deg); transform:rotate(45deg);\"></span>"
    "<span style=\"position:absolute; left:17px; top:-3px; width:6px; height:46px; background:#e53935; -webkit-transform:rotate(-45deg); transform:rotate(-45deg);\"></span></span>";

// 两点间的连线：由一条旋转的细条画出
inline string TutorialLine(const double x0, const double y0, const double x1, const double y1)
{
    const string rotate = "rotate(" + to_string(std::atan2(y1 - y0, x1 - x0)) + "rad)";
    return "<span style=\"position:absolute; left:" + to_string(x0) + "px; top:" + to_string(y0 - 1.5) + "px; width:"
           + to_string(std::hypot(x1 - x0, y1 - y0)) + "px; height:3px; background:#ff4d88; -webkit-transform-origin:0 50%; transform-origin:0 50%; "
           "-webkit-transform:" + rotate + "; transform:" + rotate + ";\"></span>";
}

// 区块 2 以外走过的格子，推理地图上显示真实地形
inline const vector<pair<int, int>> k_tutorial_seen_round1 = {{5, 2}, {6, 2}, {4, 2}};
inline const vector<pair<int, int>> k_tutorial_seen_round3 = {{5, 2}, {6, 2}, {4, 2}, {4, 6}, {5, 6}};

inline pair<string, uint32_t> TutorialStage::CandidateBlocks(const vector<pair<string, vector<pair<int, int>>>>& blocks, const int cols) const
{
    const Board& board = Main().board;
    html::Table table((blocks.size() + cols - 1) / cols * 2, cols);
    table.SetTableStyle("align=\"center\" cellpadding=\"0\" cellspacing=\"0\" style=\"font-size: 25px; line-height: 1.1;\"");
    for (size_t i = 0; i < blocks.size(); i++) {
        const auto& [id, cells] = blocks[i];
        GridMap block = board.unitMaps.FindBlockById(id, false, SpecialEvent::NONE);
        for (const auto [x, y] : cells) block[x][y].SetContent(TutorialCellOverlay("transparent", k_tutorial_ring));
        const int row = i / cols * 2;
        table.Get(row, i % cols).SetStyle("style=\"padding: 20px 20px 5px 20px\"")
            .SetContent(board.GetBoard(block, GetBoardOptions{.with_content = true}));
        table.Get(row + 1, i % cols).SetContent(board.GetMapTitleContent(board.unitMaps.FindMapById(board.unitMaps.all_maps, id)->title, id, false));
    }
    return {table.ToString(), 250 * std::min<uint32_t>(blocks.size(), cols) + 10};
}

inline vector<pair<int, int>> TutorialStage::BlockCells(const int i) const
{
    const auto [bx, by] = Main().board.unitMaps.pos[i];
    vector<pair<int, int>> cells;
    for (int x = bx; x < bx + 3; x++) {
        for (int y = by; y < by + 3; y++) cells.push_back({x, y});
    }
    return cells;
}

// 推理地图：画出已确定的区块和走过的格子，其余格子盖灰色
inline string TutorialStage::KnownMap(const TutorialMapMarks& m) const
{
    const Board& board = Main().board;
    GridMap view(board.size, true);
    vector<pair<int, int>> known_cells = m.seen;
    for (const int i : m.blocks) {
        PlaceBlock(view, i);
        for (const auto& cell : BlockCells(i)) known_cells.push_back(cell);
    }
    for (const auto& [x, y] : m.seen) view[x][y] = board.grid_map[x][y];
    for (const auto& [x, y, d] : m.walls) view.WallAt(x, y, d).SetType(WallType::NORMAL);
    const auto has = [](const vector<pair<int, int>>& cells, const int x, const int y) {
        return std::find(cells.begin(), cells.end(), pair{x, y}) != cells.end();
    };
    for (int x = 0; x < board.size; x++) {
        for (int y = 0; y < board.size; y++) {
            const bool known = has(known_cells, x, y);
            const string background = has(m.shaded, x, y) ? (known ? "rgba(255, 80, 80, 0.45)" : "#e0a6a6") : (known ? "transparent" : "#c8c8c8");
            string inner = has(m.rings, x, y) ? k_tutorial_ring : has(m.crosses, x, y) ? k_tutorial_cross : "";
            if (has(m.marks, x, y)) inner = Board::GetPlayerMark(string(num[player.Get()]), view[x][y].Type(), view[x][y].Attach());
            if (background != "transparent" || !inner.empty()) view[x][y].SetContent(TutorialCellOverlay(background, inner));
        }
    }
    return board.GetBoard(view, GetBoardOptions{.with_content = true, .area = m.area});
}

inline pair<string, uint32_t> TutorialStage::ImageHtml(const TutorialImage image) const
{
    const Board& board = Main().board;
    const uint32_t map_width = (GRID_SIZE + WALL_SIZE) * (board.size + 1);
    const pair<int, int> me = {board.players[player].x, board.players[player].y};
    // 声响范围覆盖的格子
    const auto sound_region = [&] {
        vector<pair<int, int>> region;
        for (int x = 0; x < board.size; x++) {
            for (int y = 0; y < board.size; y++) {
                if (board.GetSoundDirection(x, y, board.players[player]) == heard_direction) region.push_back({x, y});
            }
        }
        return region;
    };
    switch (image) {
        case TutorialImage::NONE:
            return {"", 0};
        case TutorialImage::POOL:
            return {board.GetAllBlocksInfo(SpecialEvent::NONE, false), (GRID_SIZE + WALL_SIZE) * 16 + 40};
        case TutorialImage::START_CANDIDATES:
            return CandidateBlocks({{"2", {{2, 0}}}, {"5", {{0, 0}}}, {"6", {{1, 0}}}, {"13", {{0, 1}, {2, 0}}}, {"23", {{0, 0}, {2, 2}}}});
        case TutorialImage::CORNER_CANDIDATES:
            return CandidateBlocks({{"2", {{2, 0}}}, {"6", {{1, 0}}}, {"13", {{2, 0}}}});
        case TutorialImage::ROUND1_MAP:
            return {KnownMap({.seen = k_tutorial_seen_round1, .marks = {me}, .rings = {{5, 3}}}), map_width};
        case TutorialImage::ROUND2_MAP:
            return {KnownMap({.seen = k_tutorial_seen_round1, .marks = {me}, .rings = {{3, 6}}, .walls = {{3, 5, Direct::RIGHT}}}), map_width};
        case TutorialImage::RIGHT_CANDIDATES:
            return CandidateBlocks({{"24", {{0, 0}}}, {"47", {{0, 0}}}});
        case TutorialImage::SOUND_DIRECTION:
            return {KnownMap({.seen = k_tutorial_seen_round3, .marks = {me}, .rings = {{1, 1}, {1, 4}, {4, 1}, {4, 4}}, .shaded = sound_region()}), map_width};
        case TutorialImage::EXCLUDE_ABOVE:
            return {KnownMap({.seen = k_tutorial_seen_round3, .marks = {me}, .rings = {{1, 1}, {4, 1}, {4, 4}}, .crosses = {{1, 4}}, .shaded = sound_region()}), map_width};
        case TutorialImage::LEFT_CANDIDATES: {
            const int half = ((GRID_SIZE + WALL_SIZE) * 9 + WALL_SIZE + 4) / 2;
            const int gap = 60;
            const int center = 20 + (GRID_SIZE * 3 + WALL_SIZE * 4 + 4) / 2;
            html::Table table(2, 3);
            table.SetTableStyle("align=\"center\" cellpadding=\"0\" cellspacing=\"0\"");
            table.MergeDown(0, 0, 2);
            table.Get(0, 0).SetContent(KnownMap({.seen = k_tutorial_seen_round3, .crosses = {{1, 1}, {4, 1}}, .area = BoardArea{0, 0, 9, 3}}));
            uint32_t blocks_width = 0;
            for (const auto& [row, id] : vector<pair<int, string>>{{0, "9"}, {1, "47"}}) {
                const string line = TutorialLine(0, row == 0 ? half : 0, gap + 20, center);
                table.Get(row, 1).SetStyle("valign=\"top\"")
                    .SetContent("<div style=\"position:relative; width:" + to_string(gap) + "px; height:" + to_string(half) + "px;\">" + line + "</div>");
                const auto [html, width] = CandidateBlocks({{id, {{1, 1}}}}, 1);
                table.Get(row, 2).SetStyle("valign=\"top\"").SetContent(html);
                blocks_width = width;
            }
            return {table.ToString(), (GRID_SIZE + WALL_SIZE) * 4 + gap + blocks_width};
        }
        case TutorialImage::COMPANION_FOUND:
            return {KnownMap({.seen = k_tutorial_seen_round3, .marks = {me}, .rings = {{4, 4}}, .crosses = {{1, 1}, {1, 4}, {4, 1}}, .shaded = sound_region()}), map_width};
        case TutorialImage::RELOCATE: {
            vector<pair<int, int>> same;
            const string walls = board.GetSurroundingWalls(player).first;
            for (const auto& [x, y] : BlockCells(4)) {
                string info;
                for (int d = 0; d < 4; d++) info += board.grid_map.WallAt(x, y, static_cast<Direct>(d)).Hidden().CanPass() ? "空" : "墙";
                if (info == walls) same.push_back({x, y});
            }
            return {KnownMap({.seen = k_tutorial_seen_round3, .rings = same}), map_width};
        }
        case TutorialImage::EXIT_SEARCH:
            return {KnownMap({.seen = k_tutorial_seen_round3, .marks = {me}, .shaded = BlockCells(8)}), map_width};
        case TutorialImage::EXIT_FOUND: {
            vector<pair<int, int>> exits;
            for (const auto& [x, y] : BlockCells(8)) {
                if (board.grid_map[x][y].Type() == GridType::EXIT) exits.push_back({x, y});
            }
            return {KnownMap({.seen = k_tutorial_seen_round3, .marks = {me}, .rings = exits, .blocks = {4, 8}}), map_width};
        }
    }
    return {"", 0};
}

/* ========== 开局配置 ========== */
// 仅限 1 名真人并带 2 名电脑；其余配置全部改为教程固定值
bool AdaptTutorialOptions(lgtbot::ChildMsgSenderBase& reply, CustomOptions& game_options, const GenericOptions& generic_options_readonly)
{
    if (generic_options_readonly.user_num_ > 1 || generic_options_readonly.PlayerNum() != k_tutorial_player_num) {
        reply() << "[错误] 新手教程仅限 1 名玩家，并需要 2 名电脑配合，请单独发送「" META_COMMAND_SIGN "新游戏 漫漫长夜 教程」开启";
        return false;
    }
    const CustomOptions defaults;
    string ignored;
#define TUTORIAL_OPTION(name, value)                                                                        \
    if (GET_OPTION_VALUE(game_options, name) != GET_OPTION_VALUE(defaults, name)) ignored += "、" #name;   \
    GET_OPTION_VALUE(game_options, name) = value;
    TUTORIAL_OPTION(模式, BlockMode::CUSTOM)
    TUTORIAL_OPTION(区块, k_tutorial_pool)
    TUTORIAL_OPTION(拓展, false)
    TUTORIAL_OPTION(边长, 9)
    TUTORIAL_OPTION(回合数, GET_OPTION_VALUE(defaults, 回合数))
    TUTORIAL_OPTION(捕捉目标, Target::PREVIOUS)
    TUTORIAL_OPTION(停止信息, StopInfo::NONE)
    TUTORIAL_OPTION(特殊事件, SpecialEvent::NONE)
    TUTORIAL_OPTION(BOSS, vector<BossType>{})
    TUTORIAL_OPTION(点杀, true)
    TUTORIAL_OPTION(隐匿, HideMode::NONE)
    TUTORIAL_OPTION(大乱斗, false)
    TUTORIAL_OPTION(谋定后动, false)
    TUTORIAL_OPTION(炸弹, 0u)
    TUTORIAL_OPTION(静音, false)
    TUTORIAL_OPTION(摸黑, false)
    TUTORIAL_OPTION(传送紊乱, false)
    TUTORIAL_OPTION(思考时限, k_tutorial_think_time)
    TUTORIAL_OPTION(行动时限, k_tutorial_act_time)
    TUTORIAL_OPTION(纹理, Texture::CLASSIC)
#undef TUTORIAL_OPTION
    if (!ignored.empty()) {
        reply() << "[警告] 新手教程使用固定配置，以下配置在教程中不生效：" << ignored.substr(string("、").size());
    }
    return true;
}

} // namespace GAME_MODULE_NAME

} // namespace game

} // namespace lgtbot
