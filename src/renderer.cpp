#include "renderer.h"
#include <windows.h>
#include <conio.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

// 控制台窗口总高度 = 网格高度 + UI行数
static constexpr int UI_ROWS = 4;

Renderer::Renderer(int gridWidth, int gridHeight)
    : gridW_(gridWidth), gridH_(gridHeight),
      brushX_(gridWidth / 2), brushY_(gridHeight / 2) {}

Renderer::~Renderer() {
    if (initialized_) shutdown();
}

void Renderer::init() {
    hOut_ = GetStdHandle(STD_OUTPUT_HANDLE);

    // 保存原始模式
    GetConsoleMode(hOut_, &originalOutMode_);

    // 启用 ANSI 转义码支持 + 禁用自动换行（避免末尾换行滚动）
    unsigned long mode = originalOutMode_ | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    mode &= ~DISABLE_NEWLINE_AUTO_RETURN;
    SetConsoleMode(hOut_, mode);

    // 设置 UTF-8 代码页，确保中文正常显示
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    // 设置控制台标题
    SetConsoleTitleA("Falling Sand Sandbox - 终端掉落沙盒");

    // 尝试调整窗口和缓冲区大小
    CONSOLE_SCREEN_BUFFER_INFO sbInfo;
    if (GetConsoleScreenBufferInfo(hOut_, &sbInfo)) {
        int totalH = gridH_ + UI_ROWS + 1;
        COORD newSize{ static_cast<SHORT>(gridW_ + 2), static_cast<SHORT>(totalH) };
        SetConsoleScreenBufferSize(hOut_, newSize);

        SMALL_RECT newWin{ 0, 0, static_cast<SHORT>(gridW_ + 1), static_cast<SHORT>(totalH - 1) };
        SetConsoleWindowInfo(hOut_, TRUE, &newWin);
    }

    // 隐藏光标、清屏
    std::cout << "\x1b[?25l\x1b[2J\x1b[H";
    initialized_ = true;
}

void Renderer::shutdown() {
    if (!initialized_) return;

    // 恢复光标、重置颜色、清屏
    std::cout << "\x1b[?25h\x1b[0m\x1b[2J\x1b[H";

    // 恢复控制台模式
    if (hOut_) SetConsoleMode(hOut_, originalOutMode_);

    initialized_ = false;
}

void Renderer::renderGridRow(const Grid& grid, int y) {
    int currentColor = -1;

    for (int x = 0; x < gridW_; x++) {
        Particle p = grid.get(x, y);
        uint8_t data = grid.getData(x, y);
        int color = particleColor(p, data);

        // 画笔区域高亮（白色边框效果：画笔覆盖范围内用白色背景）
        int dx = x - brushX_;
        int dy = y - brushY_;
        if (dx * dx + dy * dy <= 4) {
            color = 15; // 亮白
        }

        if (color != currentColor) {
            std::cout << "\x1b[48;5;" << color << "m";
            currentColor = color;
        }
        std::cout << ' ';
    }
    std::cout << "\x1b[0m\n";
}

void Renderer::renderUI(const Simulation& sim) {
    // 统计主要粒子数量
    int sand  = sim.countParticle(Particle::Sand);
    int water = sim.countParticle(Particle::Water);
    int fire  = sim.countParticle(Particle::Fire);
    int total = sand + water + fire + sim.countParticle(Particle::Stone)
              + sim.countParticle(Particle::Oil) + sim.countParticle(Particle::Plant)
              + sim.countParticle(Particle::Lava);

    std::cout << "\x1b[48;5;234m\x1b[38;5;250m"; // 深灰底 + 浅灰字

    // 第一行：状态信息
    char line1[128];
    std::snprintf(line1, sizeof(line1),
        " 画笔:%-4s  半径:%d  %s  粒子总数:%-5d  沙:%-4d 水:%-4d 火:%-4d ",
        particleName(sim.brush()), sim.brushRadius(),
        sim.paused() ? "[暂停]" : "[运行]", total, sand, water, fire);
    std::cout << line1;
    // 填充剩余宽度
    for (int i = static_cast<int>(std::strlen(line1)); i < gridW_; i++)
        std::cout << ' ';
    std::cout << '\n';

    // 第二行：粒子选择快捷键
    const char* line2 =
        " [1]沙子 [2]水 [3]石头 [4]火 [5]油 [6]蒸汽 [7]植物 [8]熔岩 [0]擦除 ";
    std::cout << line2;
    for (int i = static_cast<int>(std::strlen(line2)); i < gridW_; i++)
        std::cout << ' ';
    std::cout << '\n';

    // 第三行：操作说明
    const char* line3 =
        " 方向键移动画笔  空格绘制  [/]调整笔刷  P暂停  C清空  R重置场景  ESC退出 ";
    std::cout << line3;
    for (int i = static_cast<int>(std::strlen(line3)); i < gridW_; i++)
        std::cout << ' ';
    std::cout << '\n';

    // 第四行：分隔线
    std::cout << "\x1b[48;5;237m";
    for (int i = 0; i < gridW_; i++) std::cout << ' ';
    std::cout << "\x1b[0m\n";
}

void Renderer::render(const Simulation& sim) {
    // 光标移到左上角，逐行重绘
    std::cout << "\x1b[H";

    const Grid& g = sim.grid();
    for (int y = 0; y < gridH_; y++) {
        renderGridRow(g, y);
    }

    renderUI(sim);
    std::cout.flush();
}

bool Renderer::handleKey(int key, Simulation& sim) {
    // 数字键选择粒子
    static const struct { char key; Particle p; } kMap[] = {
        {'1', Particle::Sand},  {'2', Particle::Water},
        {'3', Particle::Stone}, {'4', Particle::Fire},
        {'5', Particle::Oil},   {'6', Particle::Steam},
        {'7', Particle::Plant}, {'8', Particle::Lava},
        {'0', Particle::Empty},
    };
    for (auto& m : kMap) {
        if (key == m.key) { sim.setBrush(m.p); return true; }
    }

    switch (key) {
        case ' ': // 空格绘制
            sim.paintAt(brushX_, brushY_);
            break;
        case 'p': case 'P':
            sim.togglePause();
            break;
        case 'c': case 'C':
            sim.clear();
            break;
        case 'r': case 'R': {
            // 重置：清空并生成一个默认场景（底部石头 + 一些沙子和水）
            sim.clear();
            Grid& g = sim.grid();
            // 底部石头层
            for (int x = 0; x < g.width(); x++)
                for (int y = g.height() - 3; y < g.height(); y++)
                    g.set(x, y, Particle::Stone);
            // 左侧一堆沙子
            for (int y = g.height() - 6; y < g.height() - 3; y++)
                for (int x = 5; x < 20; x++)
                    if (rand() % 3 != 0) g.set(x, y, Particle::Sand);
            // 右侧水
            for (int y = g.height() - 8; y < g.height() - 3; y++)
                for (int x = g.width() - 25; x < g.width() - 10; x++)
                    g.set(x, y, Particle::Water);
            break;
        }
        case '[':
            sim.setBrushRadius(sim.brushRadius() - 1);
            break;
        case ']':
            sim.setBrushRadius(sim.brushRadius() + 1);
            break;
        case 27: // ESC
            return false;
        default:
            break;
    }
    return true;
}

bool Renderer::processInput(Simulation& sim) {
    // 处理所有待处理的按键（非阻塞）
    while (_kbhit()) {
        int key = _getch();

        // 方向键是双字节：第一个 0xE0(224) 或 0x00，第二个是扫描码
        if (key == 0xE0 || key == 0x00) {
            int scan = _getch();
            switch (scan) {
                case 72: brushY_ = std::max(0, brushY_ - 1); break; // 上
                case 80: brushY_ = std::min(gridH_ - 1, brushY_ + 1); break; // 下
                case 75: brushX_ = std::max(0, brushX_ - 1); break; // 左
                case 77: brushX_ = std::min(gridW_ - 1, brushX_ + 1); break; // 右
                default: break;
            }
            continue;
        }

        if (!handleKey(key, sim)) return false;
    }
    return true;
}
