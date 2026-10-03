
// 回合阶段前置声明：RoundStage 定义于游戏命名空间内
namespace lgtbot { namespace game { namespace GAME_MODULE_NAME { class RoundStage; } } }
using RoundStage = lgtbot::game::GAME_MODULE_NAME::RoundStage;

// 玩家单次行动的统计：由 RoundStage 记录，玩家结束行动时交给各 BOSS 判定（[暴君]铁律）
struct TurnRecord
{
    int moves = 0;              // 成功移动的步数（撞墙不计）
    bool made_sound = false;    // 发出过声响（首次发声、被心跳掩盖同样计入；隐匿与静音不计）
    bool caught_player = false; // 捕捉过玩家
    bool escaped = false;       // 乘坐过逃生舱（含[大乱斗]逃生）
    bool hidden = false;        // 结束行动时处于隐匿状态
};

// BOSS 基类：一局游戏中可同时存在多个 BOSS，由 Board::bosses 持有
class Boss
{
  public:
    Boss(const BossType type, int& size, vector<Player>& players)
        : type(type), size(size), players(players) {}
    virtual ~Boss() = default;

    // BOSS信息
    const BossType type;
    int x = 0, y = 0;
    int steps = -1;
    lgtbot::PlayerID target = 0;
    // 同类型BOSS存在多个时的编号后缀
    string index_label;

    // 辅助成员变量
    int& size;   // 地图大小
    vector<Player>& players;  // Board玩家引用

    // BOSS 基础信息
    virtual string GetBossBaseName() const = 0;
    virtual string GetBossIcon() const = 0;             // emoji 图标（文字显示）
    virtual string GetBossImage() const = 0;            // 图片图标：图片名（图片显示）
    virtual string GetBossDescription() const = 0;      // 开场描述

    // 显示名称：同类型有多个时带编号
    string GetBossName() const { return GetBossBaseName() + index_label; }
    string GetBossStartInfo() const { return GetBossName() + " " + GetBossDescription(); }
    // BOSS标签：[BOSS-名称]
    string GetBossTag() const { return "[BOSS-" + GetBossName() + "]"; }
    // BOSS说话：【名称】台词；[静音] 内心想法：【名称】……？（台词）
    string GetBossSay(const string& words, const bool silence) const
    {
        return "【" + GetBossName() + "】" + (silence ? "……？（" + words + "）" : words);
    }

    // 回合结算行动：由各 BOSS 自行实现（函数体见本文件末尾 RoundActionImpl）
    virtual void HandleRoundAction(RoundStage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender) = 0;

    // ========== 扩展接口 ==========
    // 回合开始、玩家行动前（[暴君]复活、颁布铁律、公布通缉）
    virtual void HandleRoundBegin(RoundStage& stage) {}
    // 玩家结束行动时，每位玩家每回合一次（[暴君]铁律判定、弑君）
    virtual void HandlePlayerTurnEnd(RoundStage& stage, Player& player, const TurnRecord& turn) {}
    // 玩家被捕捉后，向捕捉的公屏消息追加内容（[暴君]通缉赏金）
    virtual void HandlePlayerCaught(Player& catcher, Player& caught, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender) {}
    // 玩家能否被所有玩家捕捉（[暴君]通缉）
    virtual bool IsWanted(const Player& player) const { return false; }
    // 玩家状态角标：头像四周、完整赛况昵称旁的图标（resource/badges/ 下的图片名）
    virtual void AppendPlayerBadges(const Player& player, vector<string>& badges) const {}
    // 是否有追击目标（[暴君]铁律）
    virtual bool HasTarget() const { return true; }
    // 是否在地图上（[暴君]复活）
    virtual bool IsAlive() const { return true; }
    // 查看赛况、地图时显示在上方的提示（[暴君]本回合铁律），为空则不显示
    virtual string GetStatusHint() const { return ""; }
    // 随机传送需避开的距离：默认避开当前步数可到达的区域
    virtual int TeleportAvoidRange() const { return steps; }

    // BOSS行为信息
    struct BossMoveRecord {
        string content;
        Sound sound;
        vector<string> propagation; // 声音向其他所有玩家的传播方向（私信完整赛况）

        BossMoveRecord(string content, Sound sound) : content(content), sound(sound) {}
    };
    vector<BossMoveRecord> boss_all_record;

    void NewRecord(const string& content, const Sound sound = Sound::NONE) { boss_all_record.push_back({content, sound}); }
    void UpdateContentRecord(const string& content) { if (!boss_all_record.empty()) boss_all_record.back().content = content; }
    void UpdateSoundRecord(const Sound sound) { if (!boss_all_record.empty()) boss_all_record.back().sound = sound; }
    void AddSoundPropagation(const string& direct_str) { if (!boss_all_record.empty()) boss_all_record.back().propagation.push_back(direct_str); }
    // 添加BOSS开局记录
    void InitBossStartRecord()
    {
        NewRecord(HasTarget() ? "【开局】初始锁定玩家为 [" + to_string(target) + "号]" : "【开局】暂无追击目标", StartRecordSound());
    }

    string GetBossRecord(const int query_pid, const bool is_public, const bool is_html = true) const
    {
        string result;

        for (const auto& mv : boss_all_record) {
            string sound_d;
            if (mv.sound != Sound::NONE && !is_public) {
                if (query_pid >= 0 && query_pid < mv.propagation.size()) {
                    sound_d = "[" + mv.propagation[query_pid] + "]";
                } else if (query_pid >= 0) {
                    sound_d = "{越界异常[pid=" + to_string(query_pid) + "]}";
                }
            }

            result += is_html ? "<br>" : "\n";

            if (mv.sound == Sound::BOSS) {  // [巨响]
                result += mv.content + "（巨响" + sound_d + "）";
            } else {
                result += mv.content;
            }
        }

        return result;
    }

    // BOSS初始化
    void BossInitialize()
    {
        this->steps = InitialSteps();
        BossSpawn();
        BossChangeTarget(true);
    }

    // BOSS生成
    void BossSpawn()
    {
        // 不生成在玩家周围8格
        for (int attempt = 0; attempt < 500; attempt++) {
            int x = rand() % size;
            int y = rand() % size;
            bool nearPlayer = false;
            for (int i = 0; i < players.size() && !nearPlayer; i++) {
                for (int dx = -1; dx <= 1 && !nearPlayer; dx++) {
                    for (int dy = -1; dy <= 1 && !nearPlayer; dy++) {
                        int nx = (players[i].x + dx + size) % size;
                        int ny = (players[i].y + dy + size) % size;
                        if (nx == x && ny == y)
                            nearPlayer = true;
                    }
                }
            }
            this->x = x;
            this->y = y;
            if (!nearPlayer) break;
        }
    }

    // BOSS更换目标（更换返回true）
    bool BossChangeTarget(const bool reset)
    {
        int curTarget = this->target;
        int curDist = ManhattanDistance(this->x, this->y, players[this->target].x, players[this->target].y, size);
        if (reset || players[this->target].out > 0) curDist = INT_MAX;
        for (auto& player : players) {
            if (player.out > 0) continue;
            int d = ManhattanDistance(this->x, this->y, player.x, player.y, size);
            if (SkipSameCellTarget() && d == 0) continue;   // [邦邦]位置相同需强制更换目标
            if (d < curDist) {
                this->target = player.pid;
                OnTargetSelected();     // [米诺陶斯]需要重置步数
                curDist = d;
            }
        }
        return curTarget != this->target;
    }

    // BOSS移动和更换目标（抓到人返回true）
    bool BossMove()
    {
        OnMoveBegin();      // [米诺陶斯]需要增加步数
        int targetDist = ManhattanDistance(this->x, this->y, players[this->target].x, players[this->target].y, size);
        // 步数足够直接走到目标位置
        if (this->steps >= targetDist) {
            this->x = players[this->target].x;
            this->y = players[this->target].y;
            OnReachTarget();    // [米诺陶斯]需要重置步数
            return true;
        }
        // 执行移动
        int stepsRemaining = this->steps;
        while (stepsRemaining > 0) {
            targetDist = ManhattanDistance(this->x, this->y, players[this->target].x, players[this->target].y, size);
            int tx = players[this->target].x, ty = players[this->target].y;
            int dx = tx - this->x, dy = ty - this->y;
            if (abs(dx) > size / 2) { dx = (dx > 0) ? dx - size : dx + size; }
            if (abs(dy) > size / 2) { dy = (dy > 0) ? dy - size : dy + size; }

            // 如果|dx|≠|dy|则沿较大差值轴走一步
            if (abs(dx) != abs(dy)) {
                if (abs(dx) > abs(dy)) {
                    this->x = (this->x + ((dx > 0) ? 1 : -1) + size) % size;
                } else {
                    this->y = (this->y + ((dy > 0) ? 1 : -1) + size) % size;
                }
            } else {
                // 如果已经正方形，随机选择在 x 或 y 方向上移动一步
                if (rand() % 2 == 0)
                    this->x = (this->x + ((dx > 0) ? 1 : -1) + size) % size;
                else
                    this->y = (this->y + ((dy > 0) ? 1 : -1) + size) % size;
            }
            stepsRemaining--;
        }
        return false;
    }

    bool IsBossNearby(const Player& player) const
    {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = (this->x + dx + size) % size;
                int ny = (this->y + dy + size) % size;
                if (player.x == nx && player.y == ny) {
                    return true;
                }
            }
        }
        return false;
    }

    bool Is(const BossType type) const { return this->type == type; }

  protected:
    // ========== 行为钩子：默认无特殊行为，由派生类按需覆盖 ==========
    virtual int InitialSteps() const = 0;                       // 初始步数
    virtual Sound StartRecordSound() const { return Sound::NONE; }  // 开局记录携带的声响
    virtual bool SkipSameCellTarget() const { return false; }   // 同格玩家是否跳过（不作为目标）
    virtual void OnTargetSelected() {}                          // 选中新目标时
    virtual void OnMoveBegin() {}                               // 每次移动开始时
    virtual void OnReachTarget() {}                             // 抵达目标位置时
};


// 【🐮米诺陶斯】步数逐回合递增，抵达目标即捕捉出局；移动结束发出巨响
class MinotaurBoss : public Boss
{
  public:
    MinotaurBoss(int& size, vector<Player>& players) : Boss(BossType::MINOTAUR, size, players) {}

    static constexpr const char* k_name = "米诺陶斯";
    static constexpr const char* k_icon = "🐮";
    static constexpr const char* k_image = "minotaur";
    static constexpr const char* k_brief = "追击最近的玩家，步数逐回合递增，追上即捕捉出局；移动后发出巨响";
    static constexpr const char* k_rule = R"EOF(    【🐮米诺陶斯】
1、随机生成在地图中，每回合最后行动，首回合锁定最近玩家为目标（公屏显示）
2、每回合移动步数递增，发现更近玩家则更换目标并重置步数。
3、BOSS无视地形，移动结束时会发出巨响，如果玩家在其周围会听到喘息声。
4、BOSS踩到玩家则玩家出局，玩家经过米诺陶斯不会出局。)EOF";

    string GetBossBaseName() const override { return k_name; }
    string GetBossIcon() const override { return k_icon; }
    string GetBossImage() const override { return k_image; }
    string GetBossDescription() const override
    {
        return "现身于地图中，会在回合结束时追击最近的玩家。BOSS发出震耳欲聋的巨响！请所有玩家留意BOSS开局所在的方位！";
    }

    void HandleRoundAction(RoundStage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender) override;
    // 角标：被追击的玩家
    void AppendPlayerBadges(const Player& player, vector<string>& badges) const override
    {
        if (player.out == 0 && target == player.pid) badges.push_back(k_image);
    }

  protected:
    int InitialSteps() const override { return 0; }
    Sound StartRecordSound() const override { return Sound::BOSS; }
    void OnTargetSelected() override { steps = 0; }     // 更换目标重置步数
    void OnMoveBegin() override { steps++; }            // 每回合步数递增
    void OnReachTarget() override { steps = 0; }        // 抓到玩家重置步数

  private:
    template <class Stage> void RoundActionImpl(Stage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender);
};


// 【💣邦邦】固定速度追击，抵达目标不捕捉但会换目标；每回合在落点放置炸弹
class BangBangBoss : public Boss
{
  public:
    BangBangBoss(int& size, vector<Player>& players) : Boss(BossType::BANGBANG, size, players) {}

    static constexpr const char* k_name = "邦邦";
    static constexpr const char* k_icon = "💣";
    static constexpr const char* k_image = "bangbang";
    static constexpr const char* k_brief = "以固定速度追击最近的玩家但不捕捉，每回合在落点放置炸弹并公开其四周墙壁";
    static constexpr const char* k_rule = R"EOF(    【💣邦邦】
1、所有玩家移动后，BOSS以固定速度（3-5 随机）跟着距离最近玩家移动。接触玩家后停止移动，但无法捕捉玩家；追到玩家后，会转移目标到第二近的玩家。
2、BOSS每回合结束时下一个[炸弹]，[炸弹]的四墙信息将会公开。
3、BOSS一定每回合都移动，无视地形，转换目标将公屏显示。)EOF";

    string GetBossBaseName() const override { return k_name; }
    string GetBossIcon() const override { return k_icon; }
    string GetBossImage() const override { return k_image; }
    string GetBossDescription() const override
    {
        return "带着[炸弹]现身于地图中，会在回合结束时追击最近玩家，并在结束位置放置[炸弹]。玩家经过并离开会炸飞并出局！";
    }

    void HandleRoundAction(RoundStage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender) override;
    // 角标：被追击的玩家
    void AppendPlayerBadges(const Player& player, vector<string>& badges) const override
    {
        if (player.out == 0 && target == player.pid) badges.push_back(k_image);
    }

  protected:
    int InitialSteps() const override { return rand() % 3 + 3; }    // 固定速度 3-5 随机
    bool SkipSameCellTarget() const override { return true; }       // 位置相同需强制更换目标

  private:
    template <class Stage> void RoundActionImpl(Stage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender);
};


// 【👑暴君】每回合颁布铁律：追捕最近的违反者，其余违反者遭到通缉；可被玩家点杀，复活后优先追捕弑君者
class TyrantBoss : public Boss
{
  public:
    TyrantBoss(int& size, vector<Player>& players)
        : Boss(BossType::TYRANT, size, players)
        , wanted(players.size(), false), wanted_count(players.size(), 0), slayer(players.size(), false) {}

    static constexpr const char* k_name = "暴君";
    static constexpr const char* k_icon = "👑";
    static constexpr const char* k_image = "tyrant";
    static constexpr const char* k_brief = "每回合颁布铁律，追捕并制裁违反者，其余违反者遭到通缉；可被点杀，复活后追捕弑君者";
    static constexpr const char* k_rule = R"EOF(    【👑暴君】
1、吾即律法：每回合开始时，公开自己四周墙壁信息、本回合需要遵守的[铁律]，以及被[通缉]的玩家和赏金。
2、审判：回合结束时，从本回合违反[铁律]的玩家中选择距离最近的作为追捕目标（距离相同则随机），其余违反者遭到[通缉]。已有追捕目标时出现新的违反者，则转移目标；无人违反时保持原目标，没有目标则原地不动。
3、通缉：被[通缉]的玩家可被所有玩家捕捉，持续至下回合结束。捕捉被[通缉]的玩家可获得赏金，金币数量等于其本局被[通缉]的次数。
4、立即执行：暴君无视地形，速度初始为 3，每回合 +1；本回合违反铁律的玩家多于 1 人时，速度临时增加违反人数。追上目标即制裁出局。
5、铁律：第 1 回合从[禁足]（移动不多于 1 步）、[活动]（移动不少于 1 步）中随机；之后每回合从以下随机：
- 噤声：不得发出声响（静音下不会出现）
- 仁爱：不得捕捉玩家
- 牢笼：不得逃生（大乱斗生效）
- 活动：移动不少于 2~5 步（随回合增长）
- 禁足：移动不多于 2~5 步（随机）
- 戒严：结束行动时，不得停留在区块九宫格中随机 3 个位置
移动步数按成功移动计算，撞墙不计。
6、弑君：在暴君所在位置结束行动即可点杀暴君（隐匿状态除外），弑君者不会被传送，本回合剩余时间铁律失效。暴君于下回合开始时复活，速度临时提升至 8，并优先追捕弑君者。弑君后成功逃生或存活至最后，可获得成就[弑君者]。)EOF";

    string GetBossBaseName() const override { return k_name; }
    string GetBossIcon() const override { return k_icon; }
    string GetBossImage() const override { return k_image; }
    string GetBossDescription() const override
    {
        return "降临于地图中，每回合开始时颁布[铁律]。违反铁律者将被暴君追捕制裁，其余违反者遭到通缉，可被所有玩家捕捉！暴君可被点杀，但会复活并追捕弑君者。";
    }

    void HandleRoundAction(RoundStage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender) override;
    void HandleRoundBegin(RoundStage& stage) override;
    void HandlePlayerTurnEnd(RoundStage& stage, Player& player, const TurnRecord& turn) override;

    // [通缉] 捕捉被通缉的玩家获得赏金：金币数量等于其本局被通缉的次数
    void HandlePlayerCaught(Player& catcher, Player& caught, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender) override
    {
        if (!wanted[caught.pid]) return;
        wanted[caught.pid] = false;
        const int bounty = wanted_count[caught.pid];
        catcher.coins += bounty;
        catcher.coins_this_round += bounty;
        catcher.score.coin_score += bounty * Score::COIN_SCORE;
        sender << "\n" << lgtbot::At(catcher.pid) << " 捕获了被" << GetBossName() << "通缉的 " << lgtbot::At(caught.pid) << "，获得赏金 " << bounty << " 枚【金币】！";
    }
    bool IsWanted(const Player& player) const override { return wanted[player.pid]; }
    // 角标：弑君者始终显示；追捕目标、通缉中仅在场时显示（通缉图标金币数随赏金增加，最多 3 枚）
    void AppendPlayerBadges(const Player& player, vector<string>& badges) const override
    {
        if (slayer[player.pid]) badges.push_back("regicide");
        else if (player.out == 0 && chase_target == player.pid) badges.push_back("target");
        if (player.out == 0 && wanted[player.pid]) {
            const int bounty = wanted_count[player.pid];
            badges.push_back(bounty >= 3 ? "wanted_3" : bounty == 2 ? "wanted_2" : "wanted");
        }
    }
    bool HasTarget() const override { return chase_target.has_value() && players[*chase_target].out == 0; }
    bool IsAlive() const override { return alive; }
    string GetStatusHint() const override
    {
        return GetBossTag() + " 铁律：" + LawText(law) + (alive ? "" : "（已被点杀，本回合失效）");
    }
    int TeleportAvoidRange() const override { return -1; }     // 只追捕违反铁律的玩家，随机传送无需避开

  protected:
    int InitialSteps() const override { return 3; }     // 速度初始为 3，每回合 +1

  private:
    // 铁律
    enum class LawType { GROUNDED, ACTIVE, SILENCE, MERCY, CAGE, CURFEW };
    struct Law
    {
        LawType type = LawType::ACTIVE;
        int limit = 1;                      // [禁足][活动] 步数
        std::array<bool, 9> curfew{};       // [戒严] 区块内禁止停留的位置（按行：左上、正上、右上……右下）
    };
    static constexpr const char* k_curfew_names[9] = {"左上", "正上", "右上", "正左", "中央", "正右", "左下", "正下", "右下"};

    Law law;                                // 本回合铁律
    string round_walls;                     // 本回合开始时公布的四周墙壁信息
    bool alive = true;                      // 被点杀后至下回合复活前为 false
    bool revived = false;                   // 本回合刚复活：速度临时提升至 8
    std::optional<lgtbot::PlayerID> chase_target;   // 追捕目标（无人违反铁律时为空）
    std::optional<lgtbot::PlayerID> regicide;       // 弑君者：优先追捕
    vector<lgtbot::PlayerID> violators;             // 本回合违反铁律的玩家（按行动顺序）
    vector<bool> wanted;                    // 通缉中：持续至下回合结束
    vector<int> wanted_count;               // 本局被通缉次数（赏金）
    vector<bool> slayer;                    // 点杀过本暴君（弑君者角标）

    // 随机颁布铁律：第 1 回合仅[禁足][活动]（1 步）；[静音]下没有[噤声]
    static Law RandomLaw(const int round, const bool silence)
    {
        Law law;
        if (round <= 1) {
            law.type = rand() % 2 == 0 ? LawType::GROUNDED : LawType::ACTIVE;
            return law;
        }
        vector<LawType> pool = {LawType::MERCY, LawType::CAGE, LawType::ACTIVE, LawType::GROUNDED, LawType::CURFEW};
        if (!silence) pool.push_back(LawType::SILENCE);
        law.type = pool[rand() % pool.size()];
        if (law.type == LawType::ACTIVE) law.limit = std::min(round, 5);    // 2~5 随回合增长
        if (law.type == LawType::GROUNDED) law.limit = rand() % 4 + 2;      // 2~5 随机
        if (law.type == LawType::CURFEW) {
            int cells[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
            for (int i = 0; i < 3; i++) {
                std::swap(cells[i], cells[i + rand() % (9 - i)]);
                law.curfew[cells[i]] = true;
            }
        }
        return law;
    }

    static string LawText(const Law& law)
    {
        switch (law.type) {
            case LawType::GROUNDED: return "「禁足」移动不得多于 " + to_string(law.limit) + " 步";
            case LawType::ACTIVE:   return "「活动」移动不得少于 " + to_string(law.limit) + " 步";
            case LawType::SILENCE:  return "「噤声」不得发出任何声响";
            case LawType::MERCY:    return "「仁爱」不得捕捉玩家";
            case LawType::CAGE:     return "「牢笼」不得乘坐逃生舱逃生";
            case LawType::CURFEW: {
                string cells;
                for (int i = 0; i < 9; i++) {
                    if (law.curfew[i]) cells += (cells.empty() ? "" : "、") + string(k_curfew_names[i]);
                }
                return "「戒严」结束行动时不得停留在区块的 " + cells + " 位置";
            }
        }
        return "";
    }

    // 本次行动是否违反铁律（blocks：各区块左上角坐标）
    bool Violates(const Player& player, const TurnRecord& turn, const vector<pair<int, int>>& blocks) const
    {
        switch (law.type) {
            case LawType::GROUNDED: return turn.moves > law.limit;
            case LawType::ACTIVE:   return turn.moves < law.limit;
            case LawType::SILENCE:  return turn.made_sound;
            case LawType::MERCY:    return turn.caught_player;
            case LawType::CAGE:     return turn.escaped;    // [大乱斗] 逃生算违反
            case LawType::CURFEW:
                for (const auto& [bx, by] : blocks) {
                    const int dx = (player.x - bx + size) % size, dy = (player.y - by + size) % size;
                    if (dx < 3 && dy < 3) return law.curfew[dx * 3 + dy];
                }
                return false;   // 不属于任何区块（特殊边长地图空隙）
        }
        return false;
    }

    // 追捕目标、弑君者已出局（被捕捉、逃生等）时放弃追捕
    void DropLeftTargets()
    {
        if (regicide.has_value() && players[*regicide].out > 0) regicide.reset();
        if (chase_target.has_value() && players[*chase_target].out > 0) chase_target.reset();
    }

    template <class Stage> void RoundBeginImpl(Stage& stage);
    template <class Stage> void PlayerTurnEndImpl(Stage& stage, Player& player, const TurnRecord& turn);
    template <class Stage> void RoundActionImpl(Stage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender);
};


// BOSS 工厂：按类型创建对应的 BOSS 对象
inline std::unique_ptr<Boss> CreateBoss(const BossType type, int& size, vector<Player>& players)
{
    switch (type) {
        case BossType::MINOTAUR:    return std::make_unique<MinotaurBoss>(size, players);
        case BossType::BANGBANG:    return std::make_unique<BangBangBoss>(size, players);
        case BossType::TYRANT:      return std::make_unique<TyrantBoss>(size, players);
        default:                    return nullptr;
    }
}

// 按配置顺序创建全部 BOSS（允许同类型多只）；同类型存在多只时依次编号
inline vector<std::unique_ptr<Boss>> CreateBosses(const vector<BossType>& types, int& size, vector<Player>& players)
{
    vector<std::unique_ptr<Boss>> bosses;
    for (const BossType type : types) {
        if (auto boss = CreateBoss(type, size, players); boss) {
            bosses.push_back(std::move(boss));
        }
    }
    for (size_t i = 0; i < bosses.size(); i++) {
        int total = 0, index = 0;
        for (size_t j = 0; j < bosses.size(); j++) {
            if (bosses[j]->type != bosses[i]->type) continue;
            total++;
            if (j <= i) index++;
        }
        if (total > 1) bosses[i]->index_label = to_string(index);
    }
    return bosses;
}

// 全部 BOSS 的名称、简介与规则（「#规则 漫漫长夜 BOSS [名称]」）
struct BossRuleInfo
{
    const char* name;
    const char* icon;
    const char* brief;
    const char* rule;
};
inline constexpr BossRuleInfo k_boss_rule_infos[] = {
    {MinotaurBoss::k_name, MinotaurBoss::k_icon, MinotaurBoss::k_brief, MinotaurBoss::k_rule},
    {BangBangBoss::k_name, BangBangBoss::k_icon, BangBangBoss::k_brief, BangBangBoss::k_rule},
    {TyrantBoss::k_name, TyrantBoss::k_icon, TyrantBoss::k_brief, TyrantBoss::k_rule},
};

// BOSS 规则查询：名称匹配时返回该 BOSS 的规则，否则返回列表与简介
inline string GetBossRuleText(const string& name)
{
    for (const auto& info : k_boss_rule_infos) {
        if (name == info.name) return info.rule;
    }
    string text = "《BOSS 列表》";
    for (const auto& info : k_boss_rule_infos) {
        text += "\n【" + string(info.icon) + info.name + "】" + info.brief;
    }
    text += "\n\n详细规则和技能：「" META_COMMAND_SIGN "规则 漫漫长夜 BOSS 名称」";
    return text;
}


/* ========== BOSS 回合结算行动 ========== */
/* Stage 恒为 RoundStage：由 mygame.cc 末尾 HandleRoundAction 转发调用并完成实例化 */

// 【🐮米诺陶斯】回合结算行动
template <class Stage>
void MinotaurBoss::RoundActionImpl(Stage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
{
    auto& Main = stage.Main();
    auto& Global = stage.Global();
    const bool silence = GET_OPTION_VALUE(Global.Options(), 静音);
    const string boss_tag = GetBossTag();

    if (BossChangeTarget(false)) {
        // 更换目标，重置步数
        boss_record += "发现更近的目标，变更目标至 [" + to_string(target) + "号]";
        sender << "\n" << boss_tag << " 发现了距离更近的玩家，变更锁定目标至 " << lgtbot::At(target);
    } else {
        // 未更换目标，执行移动
        if (BossMove()) {
            // 抓住玩家
            for (const auto pid: Main.board.player_map[x][y]) {
                Player& catched_player = Main.board.players[pid];
                if (catched_player.out > 0) continue;
                catched_player.NewContentRecord("(" + GetBossName() + "捕捉出局)", "end");
                catched_player.all_record.back() = catched_player.move_record;  // 回合已经结束，需强制更新完整赛况
                catched_player.out = 1;
                Main.EliminatePlayer(pid);
                catched_player.score.catch_score -= 100;        // 抓人分
                boss_record += "[" + to_string(pid) + "号] ";
                sender << "\n" << lgtbot::At(pid);
            }
            boss_record += "被BOSS捕捉出局！";
            sender << "\n被BOSS捕捉出局！";
            if (Main.Alive_() > 1) {
                BossChangeTarget(true);    // 重置锁定目标
                boss_record += "变更目标至 [" + to_string(target) + "号]";
                sender << "\n\nBOSS更换锁定目标至 " << lgtbot::At(target) << "，同时玩家捕捉目标顺位发生变更！\n";
                stage.RefreshCatchOrder(sender);    // 捕捉顺位变更
            }
        } else {
            // 未抓住玩家
            boss_record += "向 [" + to_string(target) + "号] 移动了 " + to_string(steps) + " 步";
            sender << "\n" << boss_tag << " 向 " << lgtbot::At(target) << " 移动了 " << steps << " 步";
            if (steps == 3) Main.board.players[target].achievement.boss_chase_four_steps = true;    // 成就【牛头魅魔】
        }
        // BOSS移动后发出巨响（[静音] BOSS 同样无声）
        if (!silence && (Main.Alive_() > 1 || (Global.PlayerNum() == 1 && Main.board.players[0].out == 0))) {
            UpdateSoundRecord(Sound::BOSS);
            sender << "\n\n" << GetBossName() << "发出震耳欲聋的巨响！请所有玩家留意私信声响信息！";
            stage.SendSoundMessage(x, y, Sound::BOSS, true, false, this);
        }
    }
    // BOSS周围8格内获得喘息提示（[静音] 喘息声同样消失）
    for (auto& player : Main.board.players) {
        if (!silence && IsBossNearby(player) && player.out == 0) {
            player.private_record += "\n" + boss_tag + " 你听到来自BOSS沉重的喘息声！";
            Global.Tell(player.pid) << "「呼……呼……」你听到来自[" << GetBossName() << "]沉重的喘息声！";
        }
    }
}

// 【💣邦邦】回合结算行动
template <class Stage>
void BangBangBoss::RoundActionImpl(Stage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
{
    auto& Main = stage.Main();
    const bool silence = GET_OPTION_VALUE(stage.Global().Options(), 静音);
    const string boss_tag = GetBossTag();

    // 更新目标
    if (BossChangeTarget(false)) {
        boss_record += "变更目标至 [" + to_string(target) + "号]，";
        sender << "\n" << boss_tag << " 发现了距离更近的玩家，变更锁定目标至 " << lgtbot::At(target);
    }
    // 每回合一定移动
    boss_record += "BOSS 移动中...";
    sender << "\n" << boss_tag << " 移动中...";
    if (BossMove()) {
        // 到达玩家位置（不会捕捉）
        boss_record += "追上了玩家 [" + to_string(target) + "号]，";
        sender << "\n" << GetBossSay("追到你了 [" + to_string(target) + "号]！你说邦邦不邦邦！", silence);
        BossChangeTarget(true);    // 重置锁定目标
        boss_record += "变更目标至 [" + to_string(target) + "号]，";
        sender << "\n" << boss_tag << " 抵达目标位置，更换新目标 " << lgtbot::At(target);
    }
    // 放置炸弹
    Grid& grid = Main.board.grid_map[x][y];
    // [巨大的心脏] BOSS无视地形可站上心脏格：跳过放置炸弹与墙壁播报
    if (grid.Type() == GridType::HEART) {
        boss_record += "站上了巨大的心脏，未放置炸弹";
        sender << "\n\n" << GetBossSay("这个巨大的东西是什么？算了，这回合不放炸弹了~", silence);
        return;
    }
    if (grid.Attach() == AttachType::EMPTY) grid.SetAttach(AttachType::BOMB);
    // 公屏展示炸弹墙壁信息
    auto [info, md] = Main.board.GetBossSurroundingWalls(x, y, AttachType::BOMB);
    string wall_info = GetBossName() + "所在位置的四周墙壁信息，按照 上下左右 顺序分别是：\n" + info;
    boss_record += "放置炸弹（" + info + "）";
    sender << "\n\n" << GetBossSay("哈哈，炸弹来喽~", silence)
           << "\n" << wall_info
           << "\n" << lgtbot::Markdown(md, (GRID_SIZE + WALL_SIZE * 2) + 40);
}

// 【👑暴君】回合开始：复活、颁布铁律，公布所在位置的四周墙壁与通缉名单
template <class Stage>
void TyrantBoss::RoundBeginImpl(Stage& stage)
{
    auto& Main = stage.Main();
    const bool silence = GET_OPTION_VALUE(stage.Global().Options(), 静音);
    DropLeftTargets();

    auto sender = stage.Global().Boardcast();
    sender << "【回合开始[" << GetBossIcon() << GetBossName() << " 颁布铁律]】";
    // [弑君] 回合开始时复活：重新随机位置，本回合速度临时提升至 8
    if (!alive) {
        alive = true;
        revived = true;
        BossSpawn();
        sender << "\n" << GetBossSay("朕……回来了！", silence)
               << "\n" << GetBossName() << "复活了！本回合速度临时提升至 8";
        if (chase_target.has_value()) sender << "，并优先追捕弑君者 " << lgtbot::At(*chase_target);
        sender << "\n";
    }
    // 颁布铁律
    law = RandomLaw(Main.round_, silence);
    sender << "\n" << GetBossSay("吾即律法！", silence)
           << "\n" << LawText(law);
    // 公开所在位置的四周墙壁信息
    auto [info, md] = Main.board.GetBossSurroundingWalls(x, y, AttachType::EMPTY);
    round_walls = info;
    sender << "\n\n" << GetBossName() << "所在位置的四周墙壁信息，按照 上下左右 顺序分别是：\n" << info
           << "\n" << lgtbot::Markdown(md, (GRID_SIZE + WALL_SIZE * 2) + 40);
    // 通缉名单
    bool any_wanted = false;
    for (const auto& player : players) {
        if (!wanted[player.pid] || player.out > 0) continue;
        sender << (any_wanted ? "\n" : "\n【通缉】可被所有人捕捉，持续至本回合结束：\n")
               << lgtbot::At(player.pid) << " 赏金 " << wanted_count[player.pid] << " 枚金币";
        any_wanted = true;
    }
}

// 【👑暴君】玩家结束行动：弑君、铁律判定
template <class Stage>
void TyrantBoss::PlayerTurnEndImpl(Stage& stage, Player& player, const TurnRecord& turn)
{
    auto& Global = stage.Global();
    const bool silence = GET_OPTION_VALUE(Global.Options(), 静音);
    // 被点杀后，本回合剩余时间铁律失效
    if (!alive) return;
    // [弑君] 在暴君所在位置结束行动（隐匿状态除外）：暴君倒下，弑君者不会被传送
    if (player.out == 0 && !turn.hidden && player.x == x && player.y == y) {
        alive = false;
        regicide = player.pid;
        chase_target = target = player.pid;
        slayer[player.pid] = true;
        violators.clear();
        player.achievement.regicide = true;     // 成就【弑君者】辅助
        player.NewContentRecord("(弑君)", "catch");
        Global.Boardcast() << "【弑君】" << lgtbot::At(player.pid) << " 在黑暗中点杀了" << GetBossName() << "！"
                           << "\n" << GetBossSay("你……竟敢……", silence)
                           << "\n" << GetBossName() << "倒下了，本回合剩余时间铁律失效。暴君将于下回合开始时复活，速度临时提升至 8，并优先追捕弑君者！";
        return;
    }
    // 铁律判定：逃生违反[牢笼]回合结算时公布并无能狂怒（但[大乱斗]算作违反）；其余出局玩家无需判定
    if (player.out == 2 && law.type == LawType::CAGE) {
        violators.push_back(player.pid);
    } else if (player.out == 0 && Violates(player, turn, stage.Main().board.unitMaps.pos)) {
        violators.push_back(player.pid);
        Global.Tell(player.pid) << "[" << GetBossName() << "] 违反了本回合铁律" << LawText(law) << "，回合结束时将接受审判！";
    }
}

// 【👑暴君】回合结算：通缉到期，审判违反者（逃生只能无能狂怒，追捕最近者、通缉其余），向追捕目标移动，追上即制裁（移动轨迹不公开）
template <class Stage>
void TyrantBoss::RoundActionImpl(Stage& stage, string& boss_record, lgtbot::ChildMsgSenderBase::MsgSenderGuard& sender)
{
    auto& Main = stage.Main();
    auto& Global = stage.Global();
    const bool silence = GET_OPTION_VALUE(Global.Options(), 静音);

    // [通缉] 上回合的通缉在本回合结算时到期
    wanted.assign(wanted.size(), false);
    boss_record += "四周墙壁（" + round_walls + "），" + LawText(law) + "，";
    if (!alive) {
        boss_record += "已被点杀，无法行动";
        sender << "\n" << GetBossName() << "已被点杀，本回合无法行动……将于下回合开始时复活！";
        return;
    }
    DropLeftTargets();
    // 审判：按行动顺序公布违反者，仍在场的违反者接受审判
    vector<lgtbot::PlayerID> announced, guilty;
    for (const lgtbot::PlayerID pid : violators) {
        if (players[pid].out == 1) continue;
        announced.push_back(pid);
        if (players[pid].out == 0) guilty.push_back(pid);
    }
    violators.clear();
    const bool raged = announced.size() > guilty.size();
    if (announced.empty()) {
        boss_record += "无人违反，";
        sender << "\n本回合无人违反铁律";
    } else {
        boss_record += "违反者";
        sender << "\n本回合违反铁律的玩家：";
        for (const lgtbot::PlayerID pid : announced) {
            boss_record += " [" + to_string(pid) + "号]";
            sender << lgtbot::At(pid) << " ";
        }
        boss_record += "，";
    }
    if (raged) {
        boss_record += "无能狂怒，";
        sender << "\n" << GetBossSay("可恶！竟敢从朕的牢笼中逃走！", silence) << "\n" << GetBossName() << "发动无能狂怒！";
    }
    if (!guilty.empty()) {
        // 弑君者优先；否则选择距离最近的违反者（距离相同则随机）
        lgtbot::PlayerID chosen = guilty.front();
        if (regicide.has_value()) {
            chosen = *regicide;
        } else {
            vector<lgtbot::PlayerID> nearest;
            int best = INT_MAX;
            for (const lgtbot::PlayerID pid : guilty) {
                const int d = ManhattanDistance(x, y, players[pid].x, players[pid].y, size);
                if (d < best) { best = d; nearest.clear(); }
                if (d == best) nearest.push_back(pid);
            }
            chosen = nearest[rand() % nearest.size()];
        }
        if (chase_target != chosen) {
            sender << "\n" << GetBossName() << (chase_target.has_value() ? " 转移追捕目标至 " : " 锁定追捕目标 ") << lgtbot::At(chosen);
            chase_target = target = chosen;
        }
        // 其余违反者遭到通缉
        bool any_wanted = false;
        for (const lgtbot::PlayerID pid : guilty) {
            if (pid == chosen) continue;
            wanted[pid] = true;
            wanted_count[pid]++;
            boss_record += (any_wanted ? " [" : "通缉 [") + to_string(pid) + "号]";
            sender << (any_wanted ? " " : "\n以下玩家遭到通缉，下回合可被所有玩家捕捉：") << lgtbot::At(pid);
            any_wanted = true;
        }
        if (any_wanted) boss_record += "，";
    }
    // 速度：初始为 3，每回合 +1；复活回合临时提升至 8；违反者多于 1 人时临时增加违反人数
    int speed = InitialSteps() + Main.round_ - 1;
    if (revived) speed = std::max(speed, 8);
    if (guilty.size() > 1) speed += static_cast<int>(guilty.size());
    revived = false;
    // 追捕：移动轨迹不公开
    if (!chase_target.has_value()) {
        boss_record += "无追捕目标";
        if (announced.empty()) sender << "\n" << GetBossSay("嗯，都很守规矩……", silence);
        return;
    }
    steps = speed;              // 基类 BossMove 按 target（与追捕目标同步）移动
    if (BossMove()) {
        Player& executed = players[target];
        executed.NewContentRecord("(" + GetBossName() + "制裁出局)", "end");
        executed.all_record.back() = executed.move_record;  // 回合已经结束，需强制更新完整赛况
        executed.out = 1;
        Main.EliminatePlayer(target);
        executed.score.catch_score -= 100;      // 抓人分
        boss_record += "制裁 [" + to_string(target) + "号]";
        sender << "\n" << GetBossSay("立即执行！", silence) << "\n" << lgtbot::At(target) << " 被" << GetBossName() << "制裁出局！";
        if (regicide == chase_target) regicide.reset();
        chase_target.reset();
        if (Main.Alive_() > 1) {
            sender << "\n\n玩家捕捉目标顺位发生变更！\n";
            stage.RefreshCatchOrder(sender);
        }
    } else {
        boss_record += "追捕 [" + to_string(target) + "号]";
        sender << "\n" << GetBossName() << " 以速度 " << speed << " 追捕 " << lgtbot::At(target) << "……";
    }
}
