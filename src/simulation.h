#pragma once
#include "grid.h"
#include <random>

// 物理模拟器：逐帧更新网格中所有粒子的状态
class Simulation {
public:
    Simulation(int width, int height);

    // 推进一帧模拟
    void step();

    // 获取网格（只读访问）
    const Grid& grid() const { return grid_; }
    Grid& grid() { return grid_; }

    // 画笔控制
    void setBrush(Particle p) { brush_ = p; }
    Particle brush() const { return brush_; }
    void setBrushRadius(int r) { brushRadius_ = std::max(1, r); }
    int brushRadius() const { return brushRadius_; }
    void paintAt(int x, int y) { grid_.paint(x, y, brushRadius_, brush_, initialData(brush_)); }

    // 统计当前粒子数量（用于 UI）
    int countParticle(Particle p) const;

    // 暂停/继续
    void togglePause() { paused_ = !paused_; }
    bool paused() const { return paused_; }

    // 清空
    void clear() { grid_.clear(); }

private:
    // 更新单个格子的粒子
    void updateParticle(int x, int y);

    // 尝试将 (x,y) 的粒子移动到 (nx,ny)
    // 目标为空则直接移动；目标密度更小则交换
    // 返回是否成功移动/交换
    bool tryMove(int x, int y, int nx, int ny);

    // 判断粒子 a 是否比粒子 b 密度大（可以沉入/穿过）
    static bool denserThan(Particle a, Particle b);

    // 新生成粒子时的初始额外数据
    static uint8_t initialData(Particle p);

    // 各种粒子的具体行为
    void updateSand(int x, int y);
    void updateWater(int x, int y);
    void updateFire(int x, int y);
    void updateOil(int x, int y);
    void updateSteam(int x, int y);
    void updatePlant(int x, int y);
    void updateLava(int x, int y);
    void updateSmoke(int x, int y);

    // 点燃周围的可燃物
    void igniteNeighbors(int x, int y, int chance);

    // 随机数
    int randInt(int lo, int hi);
    bool randChance(int percent); // percent: 0-100

    Grid grid_;
    std::mt19937 rng_;
    Particle brush_ = Particle::Sand;
    int brushRadius_ = 2;
    bool paused_ = false;
    int frame_ = 0;
};
