
// 地图上的格子（地形与附着，传送门目标、按钮关联、陷阱状态）
// 墙面见 Wall，四周墙壁见 GridMap

class Grid
{
  public:
    // 按钮触发位置关联（相对位置 + 可选墙壁方向）
    struct ButtonTarget {
        int dx;
        int dy;
        std::optional<Direct> dir;
    };

    void PortalTeleport(Player& player) const
    {
        player.x += portalRelPos.first;
        player.y += portalRelPos.second;
        player.subspace = -1;
    }

    void TrapTrigger() { trap = !trap; }

    bool HasBox() const
    {
        switch (attach) {
            case AttachType::BOX:       return true;
            case AttachType::HEATBOX:   return true;
            case AttachType::JAMMERBOX: return true;
            default: return false;
        }
    }

    Grid& SetType(const GridType type)
    {
        this->grid = type;
        return *this;
    }
    Grid& SetAttach(const AttachType type)
    {
        this->attach = type;
        return *this;
    }
    Grid& SetPortal(const int relPosX, const int relPosY) {
        this->portalRelPos = {relPosX, relPosY};
        return *this;
    }
    Grid& SetButton(const vector<ButtonTarget>& button_targets)
    {
        this->buttonTargetPos = button_targets;
        return *this;
    }
    void SetContent(const string& content) { this->content = content; }

    GridType Type() const { return grid; }
    AttachType Attach() const { return attach; }
    pair<int, int> PortalPos() const { return portalRelPos; }
    bool TrapStatus() const { return trap; }
    const string& GetContent() const { return content; }
    vector<ButtonTarget> ButtonTargetPos() const { return buttonTargetPos; }

  private:
    // 区块类型
	GridType grid = GridType::EMPTY;
    // 附着类型
    AttachType attach = AttachType::EMPTY;

    // 预览用提示文本
    string content;

    // 传送门相对位置（PORTAL）
    pair<int, int> portalRelPos = {0, 0};
    // 按钮触发位置（BUTTON）
    vector<ButtonTarget> buttonTargetPos;

    // 陷阱状态（TRAP）
    bool trap = true;
};
