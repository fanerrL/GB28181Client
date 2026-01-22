@echo off
REM 环境检查脚本

echo ========================================
echo GB28181Client 环境检查
echo ========================================
echo.

set MISSING_DEPS=0

REM 检查 CMake
echo [1/5] 检查 CMake...
where cmake >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo     [OK] CMake 已安装
    cmake --version | findstr /C:"version"
) else (
    echo     [X] CMake 未安装
    echo         下载: https://cmake.org/download/
    set MISSING_DEPS=1
)
echo.

REM 检查 Visual Studio
echo [2/5] 检查 Visual Studio...
if exist "C:\Program Files\Microsoft Visual Studio\2022" (
    echo     [OK] Visual Studio 2022 已安装
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019" (
    echo     [OK] Visual Studio 2019 已安装
) else (
    echo     [X] Visual Studio 未安装
    echo         下载: https://visualstudio.microsoft.com/downloads/
    echo         需要安装 "Desktop development with C++" 工作负载
    set MISSING_DEPS=1
)
echo.

REM 检查 Qt
echo [3/5] 检查 Qt...
if exist "C:\Qt" (
    echo     [OK] Qt 已安装在 C:\Qt
    dir /B "C:\Qt" | findstr /R "^6\."
) else (
    echo     [X] Qt 未安装
    echo         下载: https://www.qt.io/download-qt-installer
    set MISSING_DEPS=1
)
echo.

REM 检查 vcpkg
echo [4/5] 检查 vcpkg...
if exist "C:\vcpkg\vcpkg.exe" (
    echo     [OK] vcpkg 已安装
) else (
    echo     [X] vcpkg 未安装
    echo         可以运行 install_deps_vcpkg.bat 自动安装
    set MISSING_DEPS=1
)
echo.

REM 检查 Git
echo [5/5] 检查 Git...
where git >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo     [OK] Git 已安装
    git --version
) else (
    echo     [X] Git 未安装
    echo         下载: https://git-scm.com/download/win
    set MISSING_DEPS=1
)
echo.

echo ========================================
if %MISSING_DEPS% equ 0 (
    echo 环境检查完成！所有依赖都已安装。
    echo 可以运行 build.bat 或 build_vcpkg.bat 编译项目。
) else (
    echo 环境检查完成！发现缺少依赖。
    echo.
    echo 建议安装步骤：
    echo 1. 安装 Visual Studio 2022 Community（免费）
    echo    https://visualstudio.microsoft.com/downloads/
    echo    安装时选择 "Desktop development with C++"
    echo.
    echo 2. 安装 CMake
    echo    https://cmake.org/download/
    echo    安装时选择 "Add CMake to system PATH"
    echo.
    echo 3. 安装 Git（如果还没有）
    echo    https://git-scm.com/download/win
    echo.
    echo 4. 运行 install_deps_vcpkg.bat 自动安装其他依赖
    echo    （这会安装 Qt、FFmpeg、eXosip2）
)
echo ========================================
echo.
pause
