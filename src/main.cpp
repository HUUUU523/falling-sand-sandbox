#include "simulation.h"
#include "renderer.h"
#include <windows.h>
#include <cstdio>

// 网格尺寸（字符单元）
static constexpr int GRID_W = 80;
static constexpr int GRID_H = 30;

// 每帧毫秒数（约 60 FPS）
static constexpr int FRAME_MS = 16;

int main() {
    Simulation sim(GRID_W, GRID_H);
    Renderer renderer(GRID_W, GRID_H);

    renderer.init();

    // 初始场景：底部石头 + 两侧沙水
    {
        Grid& g = sim.grid();
        for (int x = 0; x < g.width(); x++)
            for (int y = g.height() - 2; y < g.height(); y++)
                g.set(x, y, Particle::Stone);
        // 左侧沙堆
        for (int y = g.height() - 8; y < g.height() - 2; y++)
            for (int x = 3; x < 18; x++)
                g.set(x, y, Particle::Sand);
        // 右侧水池
        for (int y = g.height() - 10; y < g.height() - 2; y++)
            for (int x = g.width() - 22; x < g.width() - 5; x++)
                g.set(x, y, Particle::Water);
    }

    bool running = true;
    while (running) {
        running = renderer.processInput(sim);
        sim.step();
        renderer.render(sim);
        Sleep(FRAME_MS);
    }

    renderer.shutdown();
    std::printf("已退出 Falling Sand Sandbox。\n");
    return 0;
}
