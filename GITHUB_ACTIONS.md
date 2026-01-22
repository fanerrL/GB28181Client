# GitHub Actions 自动编译指南

本项目配置了 GitHub Actions 自动编译，可以在 GitHub 服务器上自动编译项目，无需本地安装任何开发环境。

## 使用步骤

### 步骤 1：创建 GitHub 仓库

1. 登录 GitHub：https://github.com
2. 点击右上角 "+" → "New repository"
3. 填写仓库信息：
   - Repository name: `GB28181Client`
   - Description: `GB28181 级联上级客户端`
   - 选择 Public 或 Private
4. 点击 "Create repository"

### 步骤 2：上传代码

在项目目录中打开命令行（Git Bash 或 CMD），运行：

```bash
cd C:\Users\unfan\Desktop\player\GB28181Client

# 初始化 Git 仓库
git init

# 添加所有文件
git add .

# 创建第一次提交
git commit -m "Initial commit: GB28181 Client implementation"

# 添加远程仓库（替换 YOUR_USERNAME 为你的 GitHub 用户名）
git remote add origin https://github.com/YOUR_USERNAME/GB28181Client.git

# 推送到 GitHub
git branch -M main
git push -u origin main
```

**注意**：如果提示需要登录，使用你的 GitHub 用户名和 Personal Access Token（不是密码）。

### 步骤 3：触发自动编译

代码推送后，GitHub Actions 会自动开始编译。

查看编译进度：
1. 打开你的 GitHub 仓库页面
2. 点击顶部的 "Actions" 标签
3. 可以看到正在运行的编译任务

### 步骤 4：下载编译结果

编译完成后（约 20-30 分钟）：

1. 在 "Actions" 页面，点击最新的编译任务
2. 滚动到页面底部，找到 "Artifacts" 部分
3. 下载 `GB28181Client-Windows-Release.zip`
4. 解压后即可运行 `GB28181Client.exe`

## 编译配置说明

项目包含两个 GitHub Actions 工作流：

### 1. build.yml（完整编译）

- 编译 Windows 和 Linux 版本
- 使用 vcpkg 管理所有依赖
- 编译时间：约 30-40 分钟
- 触发条件：推送到 main/master 分支

### 2. build-fast.yml（快速编译，推荐）

- 仅编译 Windows 版本
- 使用缓存加速编译
- 编译时间：首次约 30 分钟，后续约 10 分钟
- 触发条件：推送到 main/master 分支或手动触发

## 手动触发编译

如果想手动触发编译（不推送代码）：

1. 打开 GitHub 仓库的 "Actions" 页面
2. 选择 "Build (Fast)" 工作流
3. 点击右侧 "Run workflow" 按钮
4. 选择分支（通常是 main）
5. 点击 "Run workflow"

## 查看编译日志

如果编译失败，可以查看详细日志：

1. 在 "Actions" 页面点击失败的任务
2. 点击左侧的任务步骤（如 "Build"）
3. 查看详细的错误信息

## 常见问题

### Q1：编译失败怎么办？

**A**：查看编译日志，常见原因：
- 代码语法错误
- 依赖库安装失败
- CMake 配置错误

可以在 GitHub Issues 中报告问题。

### Q2：编译时间太长？

**A**：首次编译需要下载和编译所有依赖（约 30 分钟）。后续编译会使用缓存，只需 5-10 分钟。

### Q3：如何下载历史版本？

**A**：
1. 进入 "Actions" 页面
2. 选择历史编译任务
3. 下载对应的 Artifact

### Q4：Artifact 保留多久？

**A**：默认保留 30 天。可以在 `.github/workflows/build-fast.yml` 中修改 `retention-days`。

### Q5：可以编译 Linux 版本吗？

**A**：可以。使用 `build.yml` 工作流会同时编译 Windows 和 Linux 版本。

## 本地测试 GitHub Actions

如果想在本地测试 GitHub Actions 配置：

```bash
# 安装 act
# Windows: choco install act-cli
# Linux: curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash

# 运行测试
act -j build-windows-fast
```

## 修改编译配置

如果需要修改编译选项，编辑以下文件：

- `.github/workflows/build-fast.yml` - 快速编译配置
- `.github/workflows/build.yml` - 完整编译配置
- `CMakeLists.txt` - CMake 构建配置

修改后推送到 GitHub，会自动触发新的编译。

## 编译产物说明

下载的 ZIP 文件包含：

```
GB28181Client-Windows-Release.zip
├── GB28181Client.exe          # 主程序
├── Qt6Core.dll                # Qt 核心库
├── Qt6Gui.dll                 # Qt GUI 库
├── Qt6Widgets.dll             # Qt 控件库
├── Qt6Network.dll             # Qt 网络库
├── avcodec-*.dll              # FFmpeg 解码库
├── avformat-*.dll             # FFmpeg 格式库
├── avutil-*.dll               # FFmpeg 工具库
├── swscale-*.dll              # FFmpeg 缩放库
├── exosip2.dll                # eXosip2 SIP 库
├── osip2.dll                  # oSIP 库
└── platforms/                 # Qt 平台插件
    └── qwindows.dll
```

直接运行 `GB28181Client.exe` 即可。

## 发布版本

如果想创建正式发布版本：

1. 在 GitHub 仓库页面点击 "Releases"
2. 点击 "Create a new release"
3. 填写版本号（如 v1.0.0）
4. 上传编译好的 ZIP 文件
5. 发布

## 持续集成

每次推送代码到 GitHub，都会自动触发编译，确保代码始终可以编译通过。

## 获取帮助

如果遇到问题：

1. 查看 [GitHub Actions 文档](https://docs.github.com/en/actions)
2. 在仓库中创建 Issue
3. 查看编译日志排查问题

---

**现在你可以开始上传代码到 GitHub 了！** 🚀
