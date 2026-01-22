@echo off
REM GB28181Client 使用 vcpkg 编译脚本

echo ========================================
echo GB28181Client 编译脚本 (vcpkg)
echo ========================================
echo.

REM 设置 vcpkg 路径
set VCPKG_ROOT=C:\vcpkg

if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo [错误] 找不到 vcpkg，请先运行 install_deps_vcpkg.bat
    pause
    exit /b 1
)

REM 检查 CMake
where cmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [错误] 找不到 CMake，请先安装 CMake
    echo 下载地址: https://cmake.org/download/
    pause
    exit /b 1
)

REM 创建构建目录
if not exist build mkdir build
cd build

REM 配置项目
echo ========================================
echo 配置项目...
echo ========================================
echo.

cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake

if %ERRORLEVEL% neq 0 (
    echo [错误] CMake 配置失败
    cd ..
    pause
    exit /b 1
)

echo.
echo 配置完成
echo.

REM 编译项目
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
echo ========================================
echo 编译成功！
echo ========================================
echo.
echo 可执行文件位置: build\Release\GB28181Client.exe
echo.

cd ..
pause
