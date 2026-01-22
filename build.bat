@echo off
REM GB28181Client Windows 构建脚本
REM 使用前请修改以下路径为你的实际安装路径

echo ========================================
echo GB28181Client 构建脚本
echo ========================================
echo.

REM ===== 配置路径 =====
REM 请根据实际安装路径修改以下变量

REM Qt 安装路径
set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64

REM FFmpeg 路径
set FFMPEG_DIR=C:\ffmpeg

REM eXosip2 路径
set EXOSIP2_DIR=C:\exosip2

REM 如果使用 vcpkg，取消下面的注释并设置正确路径
REM set CMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake

REM Visual Studio 版本 (可选: "Visual Studio 16 2019" 或 "Visual Studio 17 2022")
set VS_GENERATOR=Visual Studio 17 2022

REM ===== 检查依赖 =====
echo 检查依赖...

if not exist "%Qt6_DIR%" (
    echo [错误] 找不到 Qt 目录: %Qt6_DIR%
    echo 请修改脚本中的 Qt6_DIR 变量
    pause
    exit /b 1
)

if not exist "%FFMPEG_DIR%" (
    echo [警告] 找不到 FFmpeg 目录: %FFMPEG_DIR%
    echo 如果使用 vcpkg，可以忽略此警告
)

if not exist "%EXOSIP2_DIR%" (
    echo [警告] 找不到 eXosip2 目录: %EXOSIP2_DIR%
    echo 如果使用 vcpkg，可以忽略此警告
)

echo.
echo 依赖检查完成
echo.

REM ===== 创建构建目录 =====
if not exist build (
    echo 创建构建目录...
    mkdir build
)

cd build

REM ===== 配置项目 =====
echo ========================================
echo 配置项目...
echo ========================================
echo.

if defined CMAKE_TOOLCHAIN_FILE (
    REM 使用 vcpkg
    cmake .. -G "%VS_GENERATOR%" -A x64 ^
        -DCMAKE_PREFIX_PATH=%Qt6_DIR% ^
        -DCMAKE_TOOLCHAIN_FILE=%CMAKE_TOOLCHAIN_FILE%
) else (
    REM 手动指定路径
    cmake .. -G "%VS_GENERATOR%" -A x64 ^
        -DCMAKE_PREFIX_PATH=%Qt6_DIR% ^
        -DFFMPEG_DIR=%FFMPEG_DIR% ^
        -DEXOSIP2_DIR=%EXOSIP2_DIR%
)

if %ERRORLEVEL% neq 0 (
    echo [错误] CMake 配置失败
    cd ..
    pause
    exit /b 1
)

echo.
echo 配置完成
echo.

REM ===== 编译项目 =====
echo ========================================
echo 编译项目 (Release)...
echo ========================================
echo.

cmake --build . --config Release

if %ERRORLEVEL% neq 0 (
    echo [错误] 编译失败
    cd ..
    pause
    exit /b 1
)

echo.
echo 编译完成
echo.

REM ===== 部署依赖 =====
echo ========================================
echo 部署依赖文件...
echo ========================================
echo.

cd Release

REM 使用 windeployqt 部署 Qt 依赖
echo 部署 Qt 依赖...
"%Qt6_DIR%\bin\windeployqt.exe" GB28181Client.exe

REM 复制 FFmpeg DLL
if exist "%FFMPEG_DIR%\bin" (
    echo 复制 FFmpeg DLL...
    copy /Y "%FFMPEG_DIR%\bin\*.dll" . >nul 2>&1
)

REM 复制 eXosip2 DLL
if exist "%EXOSIP2_DIR%\bin" (
    echo 复制 eXosip2 DLL...
    copy /Y "%EXOSIP2_DIR%\bin\*.dll" . >nul 2>&1
)

cd ..\..

echo.
echo ========================================
echo 构建成功！
echo ========================================
echo.
echo 可执行文件位置: build\Release\GB28181Client.exe
echo.
echo 按任意键运行程序...
pause >nul

REM 运行程序
start "" "build\Release\GB28181Client.exe"
