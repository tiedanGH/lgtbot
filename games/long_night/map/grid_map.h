// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).
//
// 方格地图：格子与墙壁（区块与游戏地图共用）
// 墙壁按边存储，相邻两格共用同一面墙；游戏地图边缘环绕（最下方/最右侧的墙即最上方/最左侧的墙）

class GridMap
{
  public:
    GridMap() = default;
    GridMap(const int size, const bool wrap)
        : wrap_(wrap), cells_(size, vector<Grid>(size)),
          h_walls_(wrap ? size : size + 1, vector<Wall>(size)),
          v_walls_(size, vector<Wall>(wrap ? size : size + 1)) {}

    size_t size() const { return cells_.size(); }
    vector<Grid>& operator[](const size_t x) { return cells_[x]; }
    const vector<Grid>& operator[](const size_t x) const { return cells_[x]; }
    vector<Grid>& at(const size_t x) { return cells_.at(x); }

    // (x, y) 格 direction 方向的墙
    Wall& WallAt(const int x, const int y, const Direct direction)
    {
        const WallPos p = Locate(x, y, direction);
        return (p.horizontal ? h_walls_ : v_walls_)[p.row][p.col];
    }
    const Wall& WallAt(const int x, const int y, const Direct direction) const
    {
        const WallPos p = Locate(x, y, direction);
        return (p.horizontal ? h_walls_ : v_walls_)[p.row][p.col];
    }

    // 是否存在 type 类型的墙
    bool ContainsWallType(const WallType type) const
    {
        for (const auto* walls : {&h_walls_, &v_walls_}) {
            for (const auto& line : *walls) {
                for (const Wall& wall : line) {
                    if (wall.Type() == type) return true;
                }
            }
        }
        return false;
    }

    // 将区块 block 放到 (x, y) 处：格子直接覆盖；墙壁与已有的墙合并（边界墙按优先级覆盖）
    void PlaceBlock(const int x, const int y, const GridMap& block)
    {
        const int n = static_cast<int>(block.size());
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                cells_[x + i][y + j] = block[i][j];
                WallAt(x + i, y + j, Direct::UP).Merge(block.WallAt(i, j, Direct::UP));
                WallAt(x + i, y + j, Direct::LEFT).Merge(block.WallAt(i, j, Direct::LEFT));
            }
            WallAt(x + i, y + n - 1, Direct::RIGHT).Merge(block.WallAt(i, n - 1, Direct::RIGHT));  // 右边界
        }
        for (int j = 0; j < n; ++j) {
            WallAt(x + n - 1, y + j, Direct::DOWN).Merge(block.WallAt(n - 1, j, Direct::DOWN));    // 下边界
        }
    }

  private:
    // 墙在存储中的位置：横向墙 h_walls_[行][列] 位于该格上方，纵向墙 v_walls_[行][列] 位于该格左侧
    struct WallPos {
        bool horizontal;
        int row;
        int col;
    };

    WallPos Locate(const int x, const int y, const Direct direction) const
    {
        const int n = static_cast<int>(cells_.size());
        switch (direction) {
            case Direct::UP:    return {true, x, y};
            case Direct::DOWN:  return {true, wrap_ ? (x + 1) % n : x + 1, y};
            case Direct::LEFT:  return {false, x, y};
            case Direct::RIGHT: return {false, x, wrap_ ? (y + 1) % n : y + 1};
        }
        return {true, x, y};
    }

    bool wrap_ = false;
    vector<vector<Grid>> cells_;
    vector<vector<Wall>> h_walls_;
    vector<vector<Wall>> v_walls_;
};
