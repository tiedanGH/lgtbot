#pragma once

#include <array>
#include <string>
#include <string_view>


/* ========== 定点数值 ========== */
// 肥度会因平分饲料而出现两位小数，统一以 1/100 为单位存储，避免浮点比较误差
using Num = int32_t;
inline constexpr Num NUM_UNIT = 100;

// 整数转定点
inline constexpr Num N(const int integer) { return integer * NUM_UNIT; }

// 定点数转显示字符串，自动省略无意义的小数位
inline std::string NumStr(const Num v)
{
    const bool neg = v < 0;
    const Num a = neg ? -v : v;
    std::string s = std::to_string(a / NUM_UNIT);
    const Num frac = a % NUM_UNIT;
    if (frac != 0) {
        s += '.';
        s += static_cast<char>('0' + frac / 10);
        if (frac % 10 != 0) {
            s += static_cast<char>('0' + frac % 10);
        }
    }
    return neg ? "-" + s : s;
}

// 将总量在若干个对象之间平分，四舍五入保留两位小数
inline constexpr Num SplitShare(const Num total, const int count)
{
    if (count <= 0) {
        return 0;
    }
    return (total * 2 + count) / (count * 2);
}


/* ========== 体力 ========== */
// 体力取 0 ~ 5 之间 0.5 的倍数，以 0.5 为单位存储，取值范围 0 ~ 10
inline constexpr int STAMINA_MIN = 0;
inline constexpr int STAMINA_MAX = 10;
// 渔夫垂钓时无法猜测 0 体力，最低只能猜 0.5
inline constexpr int GUESS_MIN = 1;
// 渔夫猜测满体力时，出 0 体力的鱼一并上钩
inline constexpr int GUESS_FULL = STAMINA_MAX;

// 体力折算为肥度：1 点体力恰好等于 1 点肥度
inline constexpr Num StaminaToNum(const int stamina) { return stamina * (NUM_UNIT / 2); }

// 体力转显示字符串：0 → "0"，5 → "2.5"，10 → "5"
inline std::string StaminaStr(const int stamina)
{
    return NumStr(StaminaToNum(stamina));
}


/* ========== 人数与回合 ========== */
inline constexpr int MIN_PLAYER = 5;            // 最少玩家数
inline constexpr int MAX_PLAYER = 15;           // 最多玩家数
inline constexpr int DEFAULT_MAX_ROUND = 8;     // 默认回合上限

// 依据总人数确定渔夫数量，其余玩家全部为鱼
inline constexpr int FishermanNum(const int player_num)
{
    if (player_num <= 6) {
        return 1;
    }
    if (player_num <= 9) {
        return 2;
    }
    if (player_num <= 13) {
        return 3;
    }
    return 4;
}


/* ========== 水池 ========== */
inline constexpr int POOL_NUM = 2;
inline constexpr int POOL_A = 0;
inline constexpr int POOL_B = 1;
inline constexpr std::string_view pool_cn[POOL_NUM] = {"A", "B"};

// 另一个水池
inline constexpr int OtherPool(const int pool) { return pool == POOL_A ? POOL_B : POOL_A; }


/* ========== 鱼的数值 ========== */
inline constexpr Num INIT_FAT = N(5);           // 初始肥度
inline constexpr Num NATURAL_CONSUME = N(1);    // 每回合自然消耗
inline constexpr Num MOVE_COST = N(1) / 2;      // 鱼移动水池的代价
inline constexpr Num STARVING_LINE = N(-3);     // 达到此肥度进入极度饥饿
inline constexpr Num DEAD_LINE = N(-5);         // 达到此肥度饿死腐烂
inline constexpr Num PLAGUE_CONSUME = N(1) / 2; // 每点瘟疫值追加的自然消耗
inline constexpr Num HUNT_REWARD = N(3);        // 捕猎成功后由猜中者平分的肥度


/* ========== 渔夫的数值 ========== */
inline constexpr int DIG_LIMIT = 2;             // 每名渔夫的挖鱼饵次数上限
inline constexpr Num MIN_WEIGHT = N(1) / 2;     // 肥度不足此值的鱼统一按此重量计入公斤数

// 鱼饵种类
enum class Bait { PRIMARY = 0, MEDIUM, ADVANCED, SANDWORM, NONE };
inline constexpr int BAIT_KIND_NUM = 4;
inline constexpr std::string_view bait_cn[BAIT_KIND_NUM] = {"初级鱼饵", "中等鱼饵", "高等鱼饵", "沙虫"};
// 各类鱼饵的营养价值
inline constexpr Num bait_nutrition[BAIT_KIND_NUM] = {N(2), N(4), N(6), N(5)};
// 各类鱼饵的初始持有数量，沙虫只能靠挖取获得
inline constexpr int bait_init_count[BAIT_KIND_NUM] = {3, 2, 1, 0};

inline constexpr int BaitIndex(const Bait bait) { return static_cast<int>(bait); }
inline std::string BaitName(const Bait bait)
{
    return bait == Bait::NONE ? "无" : std::string(bait_cn[BaitIndex(bait)]);
}


/* ========== 身份与行动 ========== */
enum class Role { FISH, FISHERMAN };

// 渔夫每回合三选一的行动
enum class FisherAct { NONE, FISHING, GIVE_UP, DIG };

// 鱼每回合在咬钩与捕猎之间二选一
enum class FishAct { NONE, BITE, HUNT };

// 玩家离场的原因
enum class OutReason { NONE, CAUGHT, STARVED, DEVOURED, QUIT };

inline constexpr std::string_view out_reason_cn[] = {"存活", "被钓上", "饿死腐烂", "被吞噬", "中途退出"};
inline std::string OutReasonName(const OutReason reason)
{
    return std::string(out_reason_cn[static_cast<int>(reason)]);
}


/* ========== 展示 ========== */
// 玩家代号，展示与指令中一律使用从 1 开始的编号
inline std::string PlayerNo(const int pid) { return std::to_string(pid + 1); }
