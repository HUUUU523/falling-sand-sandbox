#pragma once
#include "particle.h"
#include <vector>
#include <algorithm>

// 二维网格：存储粒子类型 + 每格额外数据（如火的寿命、植物生长阶段）
class Grid {
public:
    Grid(int w, int h)
        : width_(w), height_(h),
          cells_(static_cast<size_t>(w) * h, Particle::Empty),
          data_(static_cast<size_t>(w) * h, 0) {}

    int width()  const { return width_; }
    int height() const { return height_; }

    bool inBounds(int x, int y) const {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    Particle get(int x, int y) const {
        return cells_[index(x, y)];
    }

    uint8_t getData(int x, int y) const {
        return data_[index(x, y)];
    }

    void set(int x, int y, Particle p, uint8_t data = 0) {
        size_t i = index(x, y);
        cells_[i] = p;
        data_[i] = data;
    }

    void setData(int x, int y, uint8_t data) {
        data_[index(x, y)] = data;
    }

    // 交换两个格子的全部状态
    void swap(int x1, int y1, int x2, int y2) {
        size_t a = index(x1, y1);
        size_t b = index(x2, y2);
        std::swap(cells_[a], cells_[b]);
        std::swap(data_[a], data_[b]);
    }

    // 判断某格是否为空
    bool isEmpty(int x, int y) const {
        return inBounds(x, y) && get(x, y) == Particle::Empty;
    }

    // 清空整个网格
    void clear() {
        std::fill(cells_.begin(), cells_.end(), Particle::Empty);
        std::fill(data_.begin(), data_.end(), 0);
    }

    // 在指定位置画一个半径为 r 的圆形笔刷
    void paint(int cx, int cy, int radius, Particle p, uint8_t data = 0) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx * dx + dy * dy <= radius * radius) {
                    int x = cx + dx, y = cy + dy;
                    if (inBounds(x, y)) {
                        // 擦除模式直接清空；否则只在空格或同类型上画（避免覆盖已有粒子太粗暴）
                        if (p == Particle::Empty || isEmpty(x, y)) {
                            set(x, y, p, data);
                        }
                    }
                }
            }
        }
    }

private:
    size_t index(int x, int y) const {
        return static_cast<size_t>(y) * width_ + x;
    }

    int width_, height_;
    std::vector<Particle> cells_;
    std::vector<uint8_t> data_;
};
