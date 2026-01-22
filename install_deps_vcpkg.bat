@echo off
echo ========================================
echo 使用 vcpkg 安装依赖
echo ========================================
echo.

REM 检查 vcpkg 是否已安装
if exist "C:\vcpkg\vcpkg.exe" (
    echo vcpkg 已安装
    set VCPKG_ROOT=C:\vcpkg
) else (
    echo 正在安装 vcpkg...
    cd C:\
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    call bootstrap-vcpkg.bat
    set VCPKG_ROOT=C:\vcpkg
)

echo.
echo 安装依赖库...
echo.

REM 安装 Qt6
echo 安装 Qt6 (这可能需要较长时间)...
%VCPKG_ROOT%\vcpkg install qt6-base:x64-windows

REM 安装 FFmpeg
echo 安装 FFmpeg...
%VCPKG_ROOT%\vcpkg install ffmpeg:x64-windows

REM 安装 eXosip2
echo 安装 eXosip2...
%VCPKG_ROOT%\vcpkg install exosip:x64-windows

echo.
echo ========================================
echo 依赖安装完成！
echo ========================================
echo.
echo 现在可以运行 build_vcpkg.bat 编译项目
pause
