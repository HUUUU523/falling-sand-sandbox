@echo off
chcp 65001 >nul
echo ========================================
echo   Falling Sand Sandbox - 编译中...
echo ========================================
g++ -std=c++17 -O2 -Wall -Wextra -o falling_sand.exe src\main.cpp src\simulation.cpp src\renderer.cpp
if %errorlevel% equ 0 (
    echo.
    echo [成功] 编译完成！运行 falling_sand.exe 开始游戏
) else (
    echo.
    echo [失败] 编译出错，请检查上方错误信息
)
pause
