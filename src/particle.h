#pragma once
#include <cstdint>

// 粒子类型
enum class Particle : uint8_t {
    Empty = 0,
    Sand,       // 沙子：下落、堆积
    Water,      // 水：下落、水平流动
    Stone,      // 石头：静止不动
    Fire,       // 火：上升、点燃可燃物、会熄灭
    Oil,        // 油：下落流动、浮在水面、易燃
    Steam,      // 蒸汽：上升、遇冷凝结成水
    Plant,      // 植物：遇水生长、可燃
    Lava,       // 熔岩：缓慢下落、点燃一切、遇水变石
    Smoke,      // 烟：上升消散
    Count
};

// 粒子属性标签（位掩码，用于快速判断行为）
namespace Prop {
    constexpr uint8_t None    = 0;
    constexpr uint8_t Solid   = 1 << 0;  // 固体（不流动）
    constexpr uint8_t Liquid  = 1 << 1;  // 液体（水平流动）
    constexpr uint8_t Gas     = 1 << 2;  // 气体（上升）
    constexpr uint8_t Flammable = 1 << 3; // 可燃
    constexpr uint8_t Movable = 1 << 4;  // 受重力影响
}

// 获取粒子的属性标签
inline uint8_t particleProps(Particle p) {
    switch (p) {
        case Particle::Sand:  return Prop::Solid | Prop::Movable;
        case Particle::Water: return Prop::Liquid | Prop::Movable;
        case Particle::Stone: return Prop::Solid;
        case Particle::Fire:  return Prop::Gas;
        case Particle::Oil:   return Prop::Liquid | Prop::Movable | Prop::Flammable;
        case Particle::Steam: return Prop::Gas;
        case Particle::Plant: return Prop::Solid | Prop::Flammable;
        case Particle::Lava:  return Prop::Liquid | Prop::Movable;
        case Particle::Smoke: return Prop::Gas;
        default:              return Prop::None;
    }
}

// 粒子对应的 ANSI 256 色号（用于背景色渲染）
inline int particleColor(Particle p, uint8_t data = 0) {
    switch (p) {
        case Particle::Empty: return 0;    // 黑
        case Particle::Sand:  return 222;  // 金黄
        case Particle::Water: return 27;   // 蓝
        case Particle::Stone: return 240;  // 深灰
        case Particle::Fire:  // 火焰颜色随寿命变化：红→橙→黄
            return (data > 128) ? 226 : (data > 64) ? 208 : 196;
        case Particle::Oil:   return 130;  // 暗紫棕
        case Particle::Steam: return 252;  // 浅灰白
        case Particle::Plant: return 34;   // 绿
        case Particle::Lava:  return (data & 1) ? 196 : 202; // 亮红/橙红
        case Particle::Smoke: return 244;  // 中灰
        default:              return 0;
    }
}

// 粒子中文名（用于 UI 显示）
inline const char* particleName(Particle p) {
    switch (p) {
        case Particle::Empty: return "擦除";
        case Particle::Sand:  return "沙子";
        case Particle::Water: return "水";
        case Particle::Stone: return "石头";
        case Particle::Fire:  return "火";
        case Particle::Oil:   return "油";
        case Particle::Steam: return "蒸汽";
        case Particle::Plant: return "植物";
        case Particle::Lava:  return "熔岩";
        case Particle::Smoke: return "烟";
        default:              return "?";
    }
}
