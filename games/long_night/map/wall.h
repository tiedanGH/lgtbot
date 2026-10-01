// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).
//
// 墙壁：相邻两格共用同一面墙（类型、附着、计数、标注只存一份）
// 墙壁被撞击时的结算见 Board::PassWall

class Wall
{
  public:
    Wall() = default;
    explicit Wall(const WallType type) : type_(type) {}

    WallType Type() const { return type_; }
    void SetType(const WallType type) { type_ = type; }

    // [墙壁附着]
    WallAttachType Attach() const { return attach_; }
    void SetAttach(const WallAttachType attach) { attach_ = attach; }

    // [计数类墙壁] 计数接口，两侧格子共用同一计数
    int Count() const { return count_; }
    int AddCount(const int n = 1) { return count_ += n; }
    void ResetCount() { count_ = 0; }

    // 图鉴中的墙面标注（门与按钮对应的字母）
    const string& Label() const { return label_; }
    void SetLabel(const string& label) { label_ = label; }

    // 能否直接通行（不含撞击类墙壁的特殊效果）
    bool CanPass() const
    {
        switch (type_) {
            case WallType::EMPTY:    return true;
            case WallType::NORMAL:   return false;
            case WallType::HEDGE:    return false;
            case WallType::DOOR:     return false;
            case WallType::DOOROPEN: return true;
        }
        return false;
    }

    // 私信墙壁显示：树篱与打开的门显示为空墙，关闭的门显示为普通墙壁
    Wall Hidden() const
    {
        Wall hidden = *this;
        switch (type_) {
            case WallType::EMPTY:    hidden.type_ = WallType::EMPTY;  break;
            case WallType::NORMAL:   hidden.type_ = WallType::NORMAL; break;
            case WallType::HEDGE:    hidden.type_ = WallType::EMPTY;  break;
            case WallType::DOOR:     hidden.type_ = WallType::NORMAL; break;
            case WallType::DOOROPEN: hidden.type_ = WallType::EMPTY;  break;
        }
        return hidden;
    }

    // [传送门] 四周均为封闭墙壁时传送门失效：墙壁、门、打开的门均视为封闭
    bool SealsPortal() const
    {
        switch (type_) {
            case WallType::EMPTY:    return false;
            case WallType::NORMAL:   return true;
            case WallType::HEDGE:    return false;
            case WallType::DOOR:     return true;
            case WallType::DOOROPEN: return true;
        }
        return false;
    }

    // 区块拼接时边界墙壁的优先级（连通性越好优先级应越高）
    int Priority() const
    {
        switch (type_) {
            case WallType::EMPTY:    return 0;
            case WallType::NORMAL:   return 1;
            case WallType::HEDGE:    return 2;
            case WallType::DOOR:     return 3;
            case WallType::DOOROPEN: return 4;
        }
        return 0;
    }

    // [门] 切换门的开关
    void SwitchDoor()
    {
        if (type_ == WallType::DOOR) type_ = WallType::DOOROPEN;
        else if (type_ == WallType::DOOROPEN) type_ = WallType::DOOR;
    }

    // 合并相邻区块边界上的同一面墙：类型取优先级较高者，附着与标注保留已有的
    void Merge(const Wall& other)
    {
        if (other.Priority() > Priority()) type_ = other.type_;
        if (attach_ == WallAttachType::EMPTY) attach_ = other.attach_;
        if (label_.empty()) label_ = other.label_;
    }

  private:
    WallType type_ = WallType::EMPTY;
    WallAttachType attach_ = WallAttachType::EMPTY;
    int count_ = 0;
    string label_;
};
