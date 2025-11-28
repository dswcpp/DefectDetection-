@echo off
echo Starting DefectDetection in debug mode with console output...
cd /d %~dp0
set PATH=G:\Qt\6.8.1\mingw_64\bin;F:\Code\QT\DefectDetection\third_party\opencv\x64\mingw\bin;%PATH%
set QT_PLUGIN_PATH=G:\Qt\6.8.1\mingw_64\plugins
set QT_LOGGING_RULES=*.debug=true

echo.
echo ========================================
echo 正在启动缺陷检测系统调试模式...
echo 调试信息将显示在控制台窗口
echo ========================================
echo.

src\app\DefectDetection.exe

echo.
echo ========================================
echo 程序已退出
echo ========================================
pause