@echo off
REM 快速上传到 GitHub 脚本

echo ========================================
echo 上传 GB28181Client 到 GitHub
echo ========================================
echo.

REM 检查 Git
where git >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [错误] 找不到 Git，请先安装 Git
    echo 下载地址: https://git-scm.com/download/win
    pause
    exit /b 1
)

echo 请输入你的 GitHub 用户名:
set /p GITHUB_USERNAME=

if "%GITHUB_USERNAME%"=="" (
    echo [错误] 用户名不能为空
    pause
    exit /b 1
)

echo.
echo 将要创建仓库: https://github.com/%GITHUB_USERNAME%/GB28181Client
echo.
echo 请确认:
echo 1. 你已经在 GitHub 上创建了名为 GB28181Client 的仓库
echo 2. 你已经配置了 Git 用户名和邮箱
echo.
echo 如果还没有创建仓库，请:
echo 1. 访问 https://github.com/new
echo 2. Repository name 填写: GB28181Client
echo 3. 点击 Create repository
echo.
pause

echo.
echo 开始上传...
echo.

REM 初始化 Git 仓库
if not exist .git (
    echo [1/5] 初始化 Git 仓库...
    git init
    echo.
)

REM 添加所有文件
echo [2/5] 添加文件...
git add .
echo.

REM 创建提交
echo [3/5] 创建提交...
git commit -m "Initial commit: GB28181 Client implementation

- 完整的 GB28181 级联上级客户端实现
- 支持设备注册、通道查询、视频点播、PTZ 控制
- 基于 Qt C++ 开发
- 包含完整文档和 GitHub Actions 自动编译配置
"
echo.

REM 添加远程仓库
echo [4/5] 添加远程仓库...
git remote remove origin 2>nul
git remote add origin https://github.com/%GITHUB_USERNAME%/GB28181Client.git
echo.

REM 推送到 GitHub
echo [5/5] 推送到 GitHub...
git branch -M main
git push -u origin main

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================
    echo 上传成功！
    echo ========================================
    echo.
    echo 仓库地址: https://github.com/%GITHUB_USERNAME%/GB28181Client
    echo Actions 页面: https://github.com/%GITHUB_USERNAME%/GB28181Client/actions
    echo.
    echo 接下来:
    echo 1. 访问 Actions 页面查看编译进度
    echo 2. 编译完成后下载 Artifacts
    echo 3. 解压运行 GB28181Client.exe
    echo.
) else (
    echo.
    echo ========================================
    echo 上传失败！
    echo ========================================
    echo.
    echo 可能的原因:
    echo 1. 仓库不存在，请先在 GitHub 上创建
    echo 2. 需要登录，请配置 Git 凭据
    echo 3. 网络问题，请检查网络连接
    echo.
    echo 配置 Git 凭据:
    echo git config --global user.name "Your Name"
    echo git config --global user.email "your.email@example.com"
    echo.
)

pause
