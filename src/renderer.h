#pragma once
#include "simulation.h"

// 控制台渲染器：负责初始化终端、渲染网格与 UI、处理键盘输入
class Renderer {
public:
    Renderer(int gridWidth, int gridHeight);
    ~Renderer();

    // 初始化控制台（ANSI 支持、隐藏光标、设置窗口大小）
    void init();

    // 恢复控制台状态
    void shutdown();

    // 渲染一帧（网格 + UI）
    void render(const Simulation& sim);

    // 处理键盘输入，修改 sim 状态；返回 false 表示用户请求退出
    bool processInput(Simulation& sim);

    // 画笔位置
    int brushX() const { return brushX_; }
    int brushY() const { return brushY_; }

private:
    // 渲染一行网格（合并连续相同颜色以减少输出量）
    void renderGridRow(const Grid& grid, int y);

    // 渲染 UI 信息行
    void renderUI(const Simulation& sim);

    // 处理单个按键
    bool handleKey(int key, Simulation& sim);

    int gridW_, gridH_;
    int brushX_, brushY_;
    bool initialized_ = false;

    // 保存的控制台模式，用于恢复
    unsigned long originalOutMode_ = 0;
    void* hOut_ = nullptr;
};
