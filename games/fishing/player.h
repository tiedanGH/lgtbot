#pragma once

#include <array>
#include <string>
#include <vector>


/* ========== 水池 ========== */
struct Pool
{
    // 永久瘟疫值，只作用于本池
    int plague = 0;

    // 本池当前每回合的自然消耗肥度
    Num Consume() const { return NATURAL_CONSUME + plague * PLAGUE_CONSUME; }
};


/* ========== 计分 ========== */
class Score
{
  public:
    // 鱼存活到终局的基础分
    static constexpr int FISH_SURVIVE = 30;
    // 终局肥度第一、第二名的加成
    static constexpr int FISH_FIRST = 25;
    static constexpr int FISH_SECOND = 15;
    // 全局无鱼被钓上时，存活的鱼额外获得的群体胜利分
    static constexpr int FISH_GROUP = 20;
    // 渔夫钓上过鱼的基础分
    static constexpr int FISHER_CATCH = 30;
    // 公斤数最高的渔夫的加成
    static constexpr int FISHER_TOP = 25;
    // 中途退出的惩罚
    static constexpr int QUIT_SCORE = -50;

    // 基础胜利分
    int base = 0;
    // 名次或公斤数带来的加成分
    int extra = 0;
    // 鱼的群体胜利分
    int group = 0;
    // 退出惩罚
    int quit = 0;

    int FinalScore() const { return base + extra + group + quit; }
};


/* ========== 玩家 ========== */
class Player
{
  public:
    Player(const int pid, const std::string& name, const std::string& avatar)
        : pid(pid), name(name), avatar(avatar) {}

    /* ===== 基本信息 ===== */
    int pid;
    std::string name;
    std::string avatar;
    // 身份全程公开
    Role role = Role::FISH;
    // 所在水池，开局全员位于 a 池
    int pool = POOL_A;
    // 0 在场；1 已淘汰；2 已退出
    int out = 0;
    OutReason out_reason = OutReason::NONE;
    // 离场的回合，用于终局展示
    int out_round = 0;

    /* ===== 鱼的状态 ===== */
    // 肥度，决定生死与终局排名
    Num fat = INIT_FAT;

    /* ===== 渔夫的状态 ===== */
    // 各类鱼饵的剩余数量
    std::array<int, BAIT_KIND_NUM> baits{};
    // 已挖鱼饵的次数
    int dig_used = 0;
    // 累计钓上的鱼公斤数与条数
    Num weight = 0;
    int caught = 0;

    /* ===== 本回合行动 ===== */
    // 移动的目标水池，-1 表示尚未提交
    int move_to = -1;
    // 鱼在咬钩与捕猎之间二选一
    FishAct fish_act = FishAct::NONE;
    // 咬住的钩所属的渔夫，-1 表示未咬钩
    int hook = -1;
    // 咬钩投入的体力
    int stamina = 0;
    // 捕猎的目标，-1 表示未捕猎
    int hunt_target = -1;
    // 对捕猎目标所投入体力的猜测，-1 表示尚未提交
    int hunt_guess = -1;
    // 渔夫本回合的行动
    FisherAct fisher_act = FisherAct::NONE;
    // 本回合下钩使用的鱼饵
    Bait bait = Bait::NONE;
    // 垂钓猜测的体力，-1 表示尚未提交
    int guess = -1;

    /* ===== 本回合结算数据 ===== */
    // 回合开始时的肥度，用于展示本回合的增减
    Num fat_start = INIT_FAT;
    // 本回合吃到的营养
    Num gain = 0;
    // 本回合实际扣除的体力肥度，渔夫钓上鱼时此项为 0
    Num paid = 0;
    // 捕猎是否猜中
    bool hunt_hit = false;
    // 本回合是否移动过水池
    bool moved = false;
    // 本回合被钓上的鱼，供渔夫查看
    std::vector<int> catches;
    // 本回合私信给本人的结算详情
    std::string detail;

    /* ===== 计分 ===== */
    Score score;
    // 终局肥度名次，0 表示未参与排名
    int rank = 0;

    /* ===== 查询 ===== */
    bool Alive() const { return out == 0; }
    bool IsFish() const { return role == Role::FISH; }
    bool IsFisherman() const { return role == Role::FISHERMAN; }
    // 肥度达到 -3 的存活的鱼陷入极度饥饿，可以成为被捕猎的目标
    bool Starving() const { return Alive() && IsFish() && fat <= STARVING_LINE; }
    // 肥度达到 -5 即饿死腐烂
    bool Starved() const { return fat <= DEAD_LINE; }

    int BaitTotal() const
    {
        int total = 0;
        for (const int count : baits) {
            total += count;
        }
        return total;
    }
    bool HasBait() const { return BaitTotal() > 0; }
    // 身处 b 池且挖鱼饵次数尚未用尽
    bool CanDig() const { return Alive() && IsFisherman() && pool == POOL_B && dig_used < DIG_LIMIT; }
    // 本回合是否向水中投下了鱼钩。渔夫中途退出后，其已公示的钩在本回合依然生效
    bool HasHook() const { return IsFisherman() && fisher_act == FisherAct::FISHING; }

    // 被钓上时计入渔夫的公斤数，肥度不足 0.5 的一律按 0.5 公斤计
    Num Weight() const { return fat <= MIN_WEIGHT ? MIN_WEIGHT : fat; }

    std::string No() const { return PlayerNo(pid); }
    std::string RoleName() const { return IsFish() ? "鱼" : "渔夫"; }

    // 清空本回合的临时数据
    void ClearRoundData()
    {
        move_to = -1;
        fish_act = FishAct::NONE;
        hook = -1;
        stamina = 0;
        hunt_target = -1;
        hunt_guess = -1;
        fisher_act = FisherAct::NONE;
        bait = Bait::NONE;
        guess = -1;
        fat_start = fat;
        gain = 0;
        paid = 0;
        hunt_hit = false;
        moved = false;
        catches.clear();
        detail.clear();
    }
};
