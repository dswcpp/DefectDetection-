@echo off
echo Starting DefectDetection in debug mode...
cd /d %~dp0
set PATH=G:\Qt\6.8.1\mingw_64\bin;F:\Code\QT\DefectDetection\third_party\opencv\x64\mingw\bin;%PATH%
set QT_PLUGIN_PATH=G:\Qt\6.8.1\mingw_64\plugins
start src\app\DefectDetection.exe
echo Program launched.
pause