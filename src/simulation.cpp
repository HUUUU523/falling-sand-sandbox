#include "simulation.h"
#include <algorithm>

Simulation::Simulation(int width, int height)
    : grid_(width, height), rng_(std::random_device{}()) {}

int Simulation::randInt(int lo, int hi) {
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(rng_);
}

bool Simulation::randChance(int percent) {
    return randInt(0, 99) < percent;
}

uint8_t Simulation::initialData(Particle p) {
    switch (p) {
        case Particle::Fire:  return 200;   // 火焰寿命
        case Particle::Smoke: return 150;   // 烟寿命
        case Particle::Lava:  return 0;     // 用于移动计数
        default:              return 0;
    }
}

bool Simulation::denserThan(Particle a, Particle b) {
    if (b == Particle::Empty) return true;
    uint8_t pa = particleProps(a);
    uint8_t pb = particleProps(b);

    // 可移动固体 > 液体/气体
    if ((pa & Prop::Movable) && (pa & Prop::Solid) &&
        (pb & (Prop::Liquid | Prop::Gas)))
        return true;

    // 液体 > 气体
    if ((pa & Prop::Liquid) && (pb & Prop::Gas))
        return true;

    // 水 > 油（水沉到油下面）
    if (a == Particle::Water && b == Particle::Oil)
        return true;

    // 熔岩 > 水/油
    if (a == Particle::Lava && (b == Particle::Water || b == Particle::Oil))
        return true;

    return false;
}

bool Simulation::tryMove(int x, int y, int nx, int ny) {
    if (!grid_.inBounds(nx, ny)) return false;

    Particle cur = grid_.get(x, y);
    Particle dst = grid_.get(nx, ny);

    if (dst == Particle::Empty) {
        grid_.swap(x, y, nx, ny);
        return true;
    }
    if (denserThan(cur, dst)) {
        grid_.swap(x, y, nx, ny);
        return true;
    }
    return false;
}

void Simulation::igniteNeighbors(int x, int y, int chance) {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (!grid_.inBounds(nx, ny)) continue;
            Particle p = grid_.get(nx, ny);
            if ((particleProps(p) & Prop::Flammable) && randChance(chance)) {
                grid_.set(nx, ny, Particle::Fire, initialData(Particle::Fire));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// 各种粒子的更新逻辑
// ---------------------------------------------------------------------------

void Simulation::updateSand(int x, int y) {
    // 正下方
    if (tryMove(x, y, x, y + 1)) return;
    // 斜下方（随机方向）
    int dir = randChance(50) ? -1 : 1;
    if (tryMove(x, y, x + dir, y + 1)) return;
    if (tryMove(x, y, x - dir, y + 1)) return;
}

void Simulation::updateWater(int x, int y) {
    // 正下方
    if (tryMove(x, y, x, y + 1)) return;
    // 斜下方
    int dir = randChance(50) ? -1 : 1;
    if (tryMove(x, y, x + dir, y + 1)) return;
    if (tryMove(x, y, x - dir, y + 1)) return;
    // 水平流动
    if (randChance(70)) {
        if (tryMove(x, y, x + dir, y)) return;
        if (tryMove(x, y, x - dir, y)) return;
    }
}

void Simulation::updateOil(int x, int y) {
    // 油比水粘稠，水平流动概率低
    if (tryMove(x, y, x, y + 1)) return;
    int dir = randChance(50) ? -1 : 1;
    if (tryMove(x, y, x + dir, y + 1)) return;
    if (tryMove(x, y, x - dir, y + 1)) return;
    if (randChance(30)) {
        if (tryMove(x, y, x + dir, y)) return;
        if (tryMove(x, y, x - dir, y)) return;
    }
}

void Simulation::updateFire(int x, int y) {
    uint8_t life = grid_.getData(x, y);

    // 周围有水 → 火被浇灭，产生蒸汽
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx, ny = y + dy;
            if (grid_.inBounds(nx, ny) && grid_.get(nx, ny) == Particle::Water) {
                grid_.set(nx, ny, Particle::Steam, initialData(Particle::Steam));
                grid_.set(x, y, Particle::Smoke, initialData(Particle::Smoke));
                return;
            }
        }
    }

    // 点燃周围可燃物
    igniteNeighbors(x, y, 8);

    // 寿命递减
    if (life <= 1) {
        // 火熄灭：小概率变成烟，否则消失
        if (randChance(30))
            grid_.set(x, y, Particle::Smoke, initialData(Particle::Smoke));
        else
            grid_.set(x, y, Particle::Empty);
        return;
    }
    grid_.setData(x, y, life - 1);

    // 火焰向上飘动
    if (randChance(60)) {
        int dir = randChance(50) ? -1 : 1;
        if (tryMove(x, y, x + dir, y - 1)) return;
        if (tryMove(x, y, x, y - 1)) return;
    }
}

void Simulation::updateSteam(int x, int y) {
    // 上升
    int dir = randChance(50) ? -1 : 1;
    if (tryMove(x, y, x + dir, y - 1)) return;
    if (tryMove(x, y, x - dir, y - 1)) return;
    if (tryMove(x, y, x, y - 1)) return;

    // 凝结成水（越靠近顶部概率越高）
    int condenseChance = 1 + (y < grid_.height() / 3 ? 3 : 0);
    if (randChance(condenseChance)) {
        grid_.set(x, y, Particle::Water);
    }
}

void Simulation::updatePlant(int x, int y) {
    // 周围有水 → 生长（消耗水，变成植物）
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (grid_.inBounds(nx, ny) &&
                grid_.get(nx, ny) == Particle::Water &&
                randChance(4)) {
                grid_.set(nx, ny, Particle::Plant);
                return;
            }
        }
    }
}

void Simulation::updateLava(int x, int y) {
    // 熔岩移动缓慢：每 2 帧移动一次
    uint8_t cnt = grid_.getData(x, y);
    grid_.setData(x, y, cnt + 1);
    if (cnt % 2 != 0) return;

    // 遇水 → 变石头，水变蒸汽
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (grid_.inBounds(nx, ny) && grid_.get(nx, ny) == Particle::Water) {
                grid_.set(nx, ny, Particle::Steam, initialData(Particle::Steam));
                grid_.set(x, y, Particle::Stone);
                return;
            }
        }
    }

    // 点燃周围一切可燃物
    igniteNeighbors(x, y, 25);

    // 像沙子一样移动（但更慢，已在上面控制帧率）
    if (tryMove(x, y, x, y + 1)) return;
    int dir = randChance(50) ? -1 : 1;
    if (tryMove(x, y, x + dir, y + 1)) return;
    if (tryMove(x, y, x - dir, y + 1)) return;
}

void Simulation::updateSmoke(int x, int y) {
    uint8_t life = grid_.getData(x, y);
    if (life <= 1) {
        grid_.set(x, y, Particle::Empty);
        return;
    }
    grid_.setData(x, y, life - 1);

    // 上升飘动
    int dir = randChance(50) ? -1 : 1;
    if (tryMove(x, y, x + dir, y - 1)) return;
    if (tryMove(x, y, x, y - 1)) return;
    if (randChance(30)) tryMove(x, y, x - dir, y - 1);
}

// ---------------------------------------------------------------------------
// 单粒子分发
// ---------------------------------------------------------------------------

void Simulation::updateParticle(int x, int y) {
    switch (grid_.get(x, y)) {
        case Particle::Sand:  updateSand(x, y);  break;
        case Particle::Water: updateWater(x, y); break;
        case Particle::Oil:   updateOil(x, y);   break;
        case Particle::Fire:  updateFire(x, y);  break;
        case Particle::Steam: updateSteam(x, y); break;
        case Particle::Plant: updatePlant(x, y); break;
        case Particle::Lava:  updateLava(x, y);  break;
        case Particle::Smoke: updateSmoke(x, y); break;
        default: break; // Stone / Empty 不动
    }
}

// ---------------------------------------------------------------------------
// 整帧更新
// ---------------------------------------------------------------------------

void Simulation::step() {
    if (paused_) return;
    frame_++;

    // 从底部向上遍历，每行交替左右方向，避免方向性偏差
    for (int y = grid_.height() - 1; y >= 0; y--) {
        if (y % 2 == 0) {
            for (int x = 0; x < grid_.width(); x++)
                updateParticle(x, y);
        } else {
            for (int x = grid_.width() - 1; x >= 0; x--)
                updateParticle(x, y);
        }
    }
}

int Simulation::countParticle(Particle p) const {
    int count = 0;
    for (int y = 0; y < grid_.height(); y++)
        for (int x = 0; x < grid_.width(); x++)
            if (grid_.get(x, y) == p) count++;
    return count;
}
