REM filepath: C:\Users\jin\Desktop\magicwound\build.bat
@echo off
REM 确保 app.ico 已放在本目录（或修改 ICON 路径）
set ICON=app.ico

REM 编译资源（使用 MinGW 的 windres）
REM 若 windres 不在 PATH，请写全路径，例如 "C:\MinGW\bin\windres.exe"
windres resource.rc resource.o

REM 编译并链接，注意把 resource.o 加入链接输入
g++ -std=c++17 -I"C:\path\to\better-enums" -I"C:\path\to\boost" main.cpp magicwound.cpp resource.o -lws2_32 -mconsole -pthread -Wl,-Bstatic "C:\\Program Files (x86)\\Dev-Cpp\\MinGW32\\lib\\libmcfgthread-1.dll" -o MagicWound.exe

pause