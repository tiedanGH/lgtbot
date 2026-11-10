#pragma once

#include <algorithm>
#include <array>
#include <random>
#include <string>
#include <vector>


/* ========== 全局状态 ========== */
class Board
{
  public:
#ifdef TEST_BOT
    Board() : g(0x66666666) {}
#else
    Board() : g(std::random_device{}()) {}
#endif

    // 所有玩家，下标即 PlayerID
    std::vector<Player> players;
    // 并列排放的两口水池
    std::array<Pool, POOL_NUM> pools;
    // 回合上限
    int max_round = DEFAULT_MAX_ROUND;
    // 全局是否有鱼被渔夫钓上过，决定鱼能否取得群体胜利
    bool any_caught = false;

    // 随机数发生器
    std::mt19937 g;

    /* ===== 基础查询 ===== */
    int PlayerNum() const { return static_cast<int>(players.size()); }
    Player& P(const int pid) { return players[pid]; }
    const Player& P(const int pid) const { return players[pid]; }

    int RandomInt(const int n) { return n <= 0 ? 0 : static_cast<int>(g() % static_cast<uint32_t>(n)); }
    int RandomPick(const std::vector<int>& list) { return list[RandomInt(static_cast<int>(list.size()))]; }

    /* ===== 玩家查询 ===== */
    std::vector<int> AliveFish() const
    {
        std::vector<int> list;
        for (const Player& p : players) {
            if (p.Alive() && p.IsFish()) {
                list.push_back(p.pid);
            }
        }
        return list;
    }

    std::vector<int> AliveFishInPool(const int pool) const
    {
        std::vector<int> list;
        for (const Player& p : players) {
            if (p.Alive() && p.IsFish() && p.pool == pool) {
                list.push_back(p.pid);
            }
        }
        return list;
    }

    std::vector<int> AliveFishermen() const
    {
        std::vector<int> list;
        for (const Player& p : players) {
            if (p.Alive() && p.IsFisherman()) {
                list.push_back(p.pid);
            }
        }
        return list;
    }

    // 本回合在指定水池投下鱼钩的渔夫。中途退出的渔夫，其已公示的钩本回合依然有效
    std::vector<int> HooksInPool(const int pool) const
    {
        std::vector<int> list;
        for (const Player& p : players) {
            if (p.IsFisherman() && p.pool == pool && p.HasHook()) {
                list.push_back(p.pid);
            }
        }
        return list;
    }

    // 咬住指定鱼钩的鱼
    std::vector<int> BitersOf(const int fisher) const
    {
        std::vector<int> list;
        for (const Player& p : players) {
            if (p.Alive() && p.IsFish() && p.fish_act == FishAct::BITE && p.hook == fisher) {
                list.push_back(p.pid);
            }
        }
        return list;
    }

    // 指定的鱼可以捕猎的目标：同池中其余陷入极度饥饿的鱼，且自身未陷入极度饥饿
    std::vector<int> HuntTargets(const int pid) const
    {
        std::vector<int> list;
        const Player& self = P(pid);
        if (!self.Alive() || !self.IsFish() || self.Starving()) {
            return list;
        }
        for (const Player& p : players) {
            if (p.pid != self.pid && p.pool == self.pool && p.Starving()) {
                list.push_back(p.pid);
            }
        }
        return list;
    }

    // 存活的鱼中肥度的名次，并列者同名次
    int FatRank(const int pid) const
    {
        const Player& self = P(pid);
        int rank = 1;
        for (const Player& p : players) {
            if (p.Alive() && p.IsFish() && p.fat > self.fat) {
                rank++;
            }
        }
        return rank;
    }

    /* ===== 界面绘制 ===== */
    static int ImageWidth() { return 736; }
    static std::string PanelStyle();
    std::string BoardHtml(const std::string& phase, const int round, const bool reveal) const;
    std::string RosterHtml(const int appointer, const int fisherman_num) const;
    std::string ScoreHtml() const;

  private:
    std::string PoolCell_(const int pool, const bool reveal) const;
    std::string FishermanBlock_(const Player& p, const bool reveal) const;
    std::string FishRow_(const Player& p, const int font, const bool reveal) const;
};


/* ========== 绘制辅助 ========== */

// 转义昵称中的 HTML 特殊字符，避免破坏表格结构
inline std::string EscHtml(const std::string& text)
{
    std::string out;
    for (const char c : text) {
        switch (c) {
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '&': out += "&amp;"; break;
            case '"': out += "&quot;"; break;
            default: out += c;
        }
    }
    return out;
}

// 头像与其右下角的编号角标
inline std::string AvatarBadge(const std::string& avatar, const int pid, const int size)
{
    const int badge = size <= 18 ? 8 : 9;
    const int box = badge + 3;
    return "<span class='pav' style='width:" + std::to_string(size) + "px; height:" + std::to_string(size) +
           "px'>" + avatar + "<i class='pno' style='font-size:" + std::to_string(badge) + "px; min-width:" +
           std::to_string(box) + "px; height:" + std::to_string(box) + "px; line-height:" + std::to_string(box) +
           "px'>" + PlayerNo(pid) + "</i></span>";
}

// 不带编号角标的头像，用于已经单列出编号的表格
inline std::string PlainAvatar(const std::string& avatar, const int size)
{
    return "<span class='pav' style='width:" + std::to_string(size) + "px; height:" + std::to_string(size) +
           "px'>" + avatar + "</span>";
}

// 侧视的鱼，肥度越低颜色越暗淡
inline std::string FishIcon(const std::string& color, const int width)
{
    const int height = width * 7 / 11;
    return "<svg class='ico' width='" + std::to_string(width) + "' height='" + std::to_string(height) +
           "' viewBox='0 0 44 28'>"
           "<polygon points='11,14 0,4 0,24' fill='" + color + "'/>"
           "<ellipse cx='26' cy='14' rx='17' ry='9' fill='" + color + "'/>"
           "<circle cx='35' cy='11' r='2.4' fill='#ffffff'/>"
           "<circle cx='35' cy='11' r='1.3' fill='#0b2d45'/></svg>";
}

// 垂下的鱼钩
inline std::string HookIcon(const int width)
{
    return "<svg class='ico' width='" + std::to_string(width) + "' height='" + std::to_string(width * 3 / 2) +
           "' viewBox='0 0 20 30'>"
           "<path d='M10 2 V16 a6 6 0 1 1 -6 6' stroke='#b07d12' stroke-width='2.4' fill='none' "
           "stroke-linecap='round'/></svg>";
}

// 依据肥度取鱼身的颜色
inline std::string FishColor(const Num fat)
{
    if (fat <= STARVING_LINE) {
        return "#c0392b";
    }
    if (fat <= 0) {
        return "#c2851f";
    }
    if (fat >= N(8)) {
        return "#1f8f63";
    }
    return "#2b87b5";
}


inline std::string Board::PanelStyle()
{
    return R"(
<style>
body { margin: 0; background: #cbe6f3; }
.fs {
    width: 700px;
    padding: 15px 18px 17px 18px;
    background:
        radial-gradient(circle at 14% 4%, rgba(255,255,255,0.85) 0%, rgba(255,255,255,0) 46%),
        radial-gradient(circle at 88% 94%, rgba(96,168,204,0.30) 0%, rgba(96,168,204,0) 50%),
        linear-gradient(168deg, #f4fbfe 0%, #ddf0fa 56%, #c4e4f3 100%);
    border: 1px solid #6cb0d2;
    box-shadow: inset 0 0 0 4px rgba(255,255,255,0.72), inset 0 0 0 5px rgba(108,176,210,0.45);
    color: #103a52;
    font-size: 14px;
}
.fs .title {
    text-align: center;
    font-size: 25px;
    font-weight: bold;
    letter-spacing: 11px;
    color: #0c4c6e;
    text-shadow: 0 1px 0 rgba(255,255,255,0.9);
}
.fs .wave {
    height: 3px;
    margin: 7px 0 5px 0;
    background: linear-gradient(90deg, rgba(63,155,196,0) 0%, #3f9bc4 50%, rgba(63,155,196,0) 100%);
    opacity: 0.8;
}
.fs .sub { text-align: center; color: #487f9c; letter-spacing: 2px; margin-bottom: 10px; font-size: 13px; }
.fs .notice {
    margin: 0 0 10px 0;
    padding: 6px 10px;
    font-size: 12px;
    line-height: 19px;
    color: #7a4a08;
    border-left: 3px solid #d9891f;
    background: rgba(217,137,31,0.13);
}
.fs table.pools { width: 100%; border-collapse: separate; border-spacing: 9px 0; table-layout: fixed; }
.fs table.pools td {
    width: 50%;
    vertical-align: top;
    padding: 7px 9px 9px 9px;
    border: 1px solid #7cbada;
    border-radius: 7px;
    background:
        linear-gradient(180deg, rgba(255,255,255,0.92) 0%, rgba(255,255,255,0.34) 30%),
        linear-gradient(165deg, rgba(214,238,249,0.95) 0%, rgba(176,216,236,0.95) 100%);
}
.fs .phead { margin-bottom: 5px; white-space: nowrap; }
.fs .pname { font-size: 18px; font-weight: bold; letter-spacing: 3px; color: #0c4c6e; }
.fs .tag {
    display: inline-block;
    margin-left: 5px;
    padding: 0 6px;
    font-size: 11px;
    line-height: 17px;
    border-radius: 9px;
    vertical-align: 2px;
}
.fs .tag.cs { background: rgba(16,58,82,0.11); color: #245c7c; }
.fs .tag.pl { background: rgba(140,50,150,0.16); color: #7c2a84; }
.fs .sect {
    margin: 7px 0 3px 0;
    padding-left: 6px;
    font-size: 11px;
    letter-spacing: 2px;
    color: #4d87a4;
    border-left: 2px solid #5aa5c9;
}
.fs .fb {
    margin-bottom: 5px;
    padding: 4px 6px 5px 6px;
    border-radius: 5px;
    background: rgba(255,204,102,0.22);
    border: 1px solid rgba(196,140,30,0.45);
}
.fs .row { white-space: nowrap; overflow: hidden; }
.fs .fr { white-space: nowrap; overflow: hidden; }
.fs .fr .nm { display: inline-block; width: 120px; }
.fs .fr .num { display: inline-block; width: 42px; text-align: right; }
.fs .pav { position: relative; display: inline-block; vertical-align: middle; line-height: 0; margin-right: 7px; }
.fs .pav img { width: 100% !important; height: 100% !important; border-radius: 50% !important; }
.fs .pno {
    position: absolute;
    right: -3px;
    bottom: -3px;
    display: block;
    text-align: center;
    font-style: normal;
    font-weight: bold;
    color: #ffffff;
    background: #1d6e92;
    border-radius: 50%;
    box-shadow: 0 0 0 1px rgba(255,255,255,0.9);
}
.fs .ico { vertical-align: middle; margin-right: 3px; }
.fs .nm {
    display: inline-block;
    max-width: 168px;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    vertical-align: middle;
}
.fs .num { font-weight: bold; color: #9c5a00; }
.fs .num.low { color: #c0392b; }
.fs .mk {
    display: inline-block;
    margin-left: 4px;
    padding: 0 4px;
    font-size: 10px;
    line-height: 15px;
    border-radius: 3px;
    vertical-align: 1px;
}
.fs .mk.hungry { background: #c0392b; color: #fff0ee; }
.fs .mk.bite { background: #1d6e92; color: #eaf7fd; }
.fs .mk.hunt { background: #7d3c98; color: #f7eafb; }
.fs .bait { margin-top: 3px; }
.fs .bait i {
    display: inline-block;
    margin-right: 4px;
    padding: 0 5px;
    font-size: 11px;
    line-height: 16px;
    font-style: normal;
    border-radius: 3px;
    background: rgba(16,58,82,0.10);
    color: #1c4f68;
}
.fs .bait i.out { background: rgba(16,58,82,0.04); color: #93aebd; }
.fs .act { margin-top: 3px; font-size: 11px; color: #7a4a08; }
.fs .empty { padding: 4px 0 2px 2px; font-size: 12px; color: #7ba3b8; }
.fs table.list { width: 100%; border-collapse: collapse; margin-top: 10px; font-size: 12px; }
.fs table.list th {
    padding: 4px 6px;
    color: #35708f;
    font-weight: normal;
    letter-spacing: 1px;
    border-bottom: 1px solid rgba(70,140,175,0.55);
}
.fs table.list td {
    padding: 4px 6px;
    text-align: center;
    border-bottom: 1px solid rgba(70,140,175,0.22);
}
.fs table.list td.left { text-align: left; }
.fs table.list tr.gone td { color: #8aa8b8; }
.fs .foot { margin-top: 9px; font-size: 11px; line-height: 18px; color: #4d87a4; }
.fs .foot b { color: #0c4c6e; }
.fs .win { color: #a85a10; font-weight: bold; }
</style>
)";
}


inline std::string Board::FishermanBlock_(const Player& p, const bool reveal) const
{
    std::string html = "<div class='fb'><div class='row'>" + AvatarBadge(p.avatar, p.pid, 20) +
                       "<span class='nm'>" + EscHtml(p.name) + "</span>";
    if (p.weight > 0) {
        html += "<span class='mk bite'>" + NumStr(p.weight) + " 公斤</span>";
    }
    if (!p.Alive()) {
        html += "<span class='mk hungry'>已退出</span>";
    }
    html += "</div><div class='bait'>";
    for (int i = 0; i < BAIT_KIND_NUM; i++) {
        const std::string name = std::string(bait_cn[i]).substr(0, 3);
        html += std::string("<i") + (p.baits[i] > 0 ? ">" : " class='out'>") + name + " " +
                std::to_string(p.baits[i]) + "</i>";
    }
    html += "</div>";
    // 渔夫的行动一经确认便全场公示
    if (p.fisher_act == FisherAct::FISHING) {
        const int biters = static_cast<int>(BitersOf(p.pid).size());
        html += "<div class='act'>" + HookIcon(10) + "下钩 · " + BaitName(p.bait);
        if (biters > 0) {
            html += " · <b>" + std::to_string(biters) + "</b> 条鱼争抢";
        } else if (reveal) {
            html += " · 无鱼问津";
        }
        if (reveal && p.guess >= 0) {
            html += " · 垂钓 " + StaminaStr(p.guess);
        }
        html += "</div>";
    } else if (p.fisher_act == FisherAct::DIG) {
        html += "<div class='act'>挖鱼饵（第 " + std::to_string(p.dig_used) + " / " +
                std::to_string(DIG_LIMIT) + " 次）</div>";
    } else if (p.fisher_act == FisherAct::GIVE_UP) {
        html += "<div class='act'>放弃钓鱼</div>";
    }
    return html + "</div>";
}


inline std::string Board::FishRow_(const Player& p, const int font, const bool reveal) const
{
    const int avatar = font >= 13 ? 20 : 18;
    std::string html = "<div class='fr' style='font-size:" + std::to_string(font) + "px; line-height:" +
                       std::to_string(font + 9) + "px'>" + AvatarBadge(p.avatar, p.pid, avatar) +
                       FishIcon(FishColor(p.fat), 17) + "<span class='nm'>" + EscHtml(p.name) +
                       "</span><span class='num" + (p.fat <= 0 ? " low" : "") + "'>" + NumStr(p.fat) + "</span>";
    if (p.Starving()) {
        html += "<span class='mk hungry'>饥饿</span>";
    }
    if (p.fish_act == FishAct::BITE && p.hook >= 0) {
        html += "<span class='mk bite'>咬" + PlayerNo(p.hook) + (reveal ? "→" + StaminaStr(p.stamina) : "") +
                "</span>";
    } else if (reveal && p.fish_act == FishAct::HUNT && p.hunt_target >= 0) {
        html += "<span class='mk hunt'>猎" + PlayerNo(p.hunt_target) +
                (p.hunt_guess >= 0 ? "→" + StaminaStr(p.hunt_guess) : "") + "</span>";
    }
    return html + "</div>";
}


inline std::string Board::PoolCell_(const int pool, const bool reveal) const
{
    const Pool& info = pools[pool];
    std::string html = "<td><div class='phead'><span class='pname'>" + std::string(pool_cn[pool]) +
                       " 池</span><span class='tag cs'>消耗 " + NumStr(info.Consume()) + "</span>";
    if (info.plague > 0) {
        html += "<span class='tag pl'>瘟疫 " + std::to_string(info.plague) + "</span>";
    }
    html += "</div>";

    std::vector<int> fishermen;
    for (const Player& p : players) {
        if (p.IsFisherman() && p.pool == pool && (p.Alive() || p.HasHook())) {
            fishermen.push_back(p.pid);
        }
    }
    html += "<div class='sect'>渔 夫</div>";
    if (fishermen.empty()) {
        html += "<div class='empty'>此池无人垂钓</div>";
    }
    for (const int pid : fishermen) {
        html += FishermanBlock_(P(pid), reveal);
    }

    const std::vector<int> fish = AliveFishInPool(pool);
    html += "<div class='sect'>鱼 群　" + std::to_string(fish.size()) + " 条</div>";
    if (fish.empty()) {
        html += "<div class='empty'>水中空无一物</div>";
    }
    const int font = fish.size() > 8 ? 12 : (fish.size() > 5 ? 13 : 14);
    for (const int pid : fish) {
        html += FishRow_(P(pid), font, reveal);
    }
    return html + "</td>";
}


inline std::string Board::BoardHtml(const std::string& phase, const int round, const bool reveal) const
{
    const std::vector<int> fish = AliveFish();
    const std::vector<int> fishermen = AliveFishermen();

    std::string html = PanelStyle() + "<div class='fs'>";
    html += "<div class='title'>钓　鱼</div><div class='wave'></div>";
    html += "<div class='sub'>第 " + std::to_string(round) + " / " + std::to_string(max_round) + " 回合　·　" +
            phase + "　·　鱼 " + std::to_string(fish.size()) + "　渔夫 " + std::to_string(fishermen.size()) +
            "</div>";

    std::string notice;
    for (const Player& p : players) {
        if (p.Starving()) {
            notice += "⚠ " + PlayerNo(p.pid) + " 号 " + EscHtml(p.name) +
                      " 已陷入极度饥饿，同池未饥饿的鱼可对其发起捕猎<br>";
        }
    }
    if (round >= max_round) {
        notice += "⏳ 本回合即为最后一回合，结算后游戏结束<br>";
    }
    if (!notice.empty()) {
        html += "<div class='notice'>" + notice.substr(0, notice.size() - 4) + "</div>";
    }

    html += "<table class='pools'><tr>";
    for (int pool = 0; pool < POOL_NUM; pool++) {
        html += PoolCell_(pool, reveal);
    }
    html += "</tr></table>";

    std::string gone;
    for (const Player& p : players) {
        if (p.Alive()) {
            continue;
        }
        if (p.IsFisherman() && p.HasHook()) {
            continue;   // 退出的渔夫在钩仍生效的回合内照常展示于水池中
        }
        if (!gone.empty()) {
            gone += "　";
        }
        gone += "<b>" + PlayerNo(p.pid) + " 号</b> " + EscHtml(p.name) + "（" + OutReasonName(p.out_reason) +
                " R" + std::to_string(p.out_round) + "）";
    }
    if (!gone.empty()) {
        html += "<div class='foot'>已离场：" + gone + "</div>";
    }
    return html + "</div>";
}


inline std::string Board::RosterHtml(const int appointer, const int fisherman_num) const
{
    std::string html = PanelStyle() + "<div class='fs'>";
    html += "<div class='title'>钓　鱼</div><div class='wave'></div>";
    html += "<div class='sub'>共 " + std::to_string(PlayerNum()) + " 人　·　本局需要 " +
            std::to_string(fisherman_num) + " 名渔夫，其余全部为鱼</div>";
    html += "<div class='notice'>" + PlayerNo(appointer) + " 号 " + EscHtml(P(appointer).name) +
            " 被水神选中，由其一次性任命全部 " + std::to_string(fisherman_num) +
            " 名渔夫；亦可放弃任命，改由水神随机指派</div>";

    html += "<table class='list'><tr><th style='width:52px'>编号</th><th class='left'>玩家</th>"
            "<th style='width:96px'>身份</th></tr>";
    for (const Player& p : players) {
        html += "<tr><td><b>" + PlayerNo(p.pid) + "</b></td>";
        html += "<td class='left'>" + PlainAvatar(p.avatar, 22) + "<span class='nm' "
                "style='max-width:300px'>" + EscHtml(p.name) + "</span></td>";
        html += "<td>" + std::string(p.pid == appointer ? "<span class='win'>任命者</span>" : "待定") + "</td></tr>";
    }
    return html + "</table></div>";
}


inline std::string Board::ScoreHtml() const
{
    std::string html = PanelStyle() + "<div class='fs'>";
    html += "<div class='title'>钓　鱼</div><div class='wave'></div>";
    html += "<div class='sub'>终　局</div>";
    if (!any_caught) {
        html += "<div class='notice'>全局无一条鱼被钓上，存活的鱼取得群体胜利</div>";
    }

    html += "<table class='list'><tr><th style='width:44px'>编号</th><th class='left'>玩家</th>"
            "<th style='width:52px'>身份</th><th style='width:118px'>结局</th>"
            "<th style='width:76px'>肥度/公斤</th><th style='width:54px'>得分</th></tr>";
    for (const Player& p : players) {
        html += std::string("<tr") + (p.Alive() ? ">" : " class='gone'>");
        html += "<td><b>" + PlayerNo(p.pid) + "</b></td>";
        html += "<td class='left'>" + PlainAvatar(p.avatar, 22) + "<span class='nm' "
                "style='max-width:210px'>" + EscHtml(p.name) + "</span></td>";
        html += "<td>" + p.RoleName() + "</td>";

        std::string result;
        if (p.IsFish()) {
            result = p.Alive() ? (p.rank == 1 ? "<span class='win'>存活 · 最肥</span>"
                                              : (p.rank == 2 ? "<span class='win'>存活 · 次肥</span>" : "存活"))
                               : OutReasonName(p.out_reason) + " R" + std::to_string(p.out_round);
        } else {
            result = p.out == 2 ? "中途退出 R" + std::to_string(p.out_round)
                                : (p.caught > 0 ? "<span class='win'>钓上 " + std::to_string(p.caught) + " 条</span>"
                                                : "空军而归");
        }
        html += "<td>" + result + "</td>";
        html += "<td>" + std::string(p.IsFish() ? NumStr(p.fat) : NumStr(p.weight)) + "</td>";
        html += "<td><b>" + std::to_string(p.score.FinalScore()) + "</b></td></tr>";
    }
    html += "</table>";
    html += "<div class='foot'>两池瘟疫：<b>A</b> " + std::to_string(pools[POOL_A].plague) + "　<b>B</b> " +
            std::to_string(pools[POOL_B].plague) + "</div>";
    return html + "</div>";
}
