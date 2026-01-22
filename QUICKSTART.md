# 快速开始指南

5 分钟快速上手 GB28181Client。

## 前提条件

- Windows 10/11 (64位)
- 已安装 Qt 6.x
- 已安装 FFmpeg
- 已安装 eXosip2（或使用 vcpkg）

## 步骤 1：获取代码

```powershell
git clone <your-repo-url> GB28181Client
cd GB28181Client
```

## 步骤 2：配置构建脚本

编辑 `build.bat`，修改以下路径：

```batch
set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64
set FFMPEG_DIR=C:\ffmpeg
set EXOSIP2_DIR=C:\exosip2
```

## 步骤 3：编译项目

```powershell
.\build.bat
```

等待编译完成（约 2-5 分钟）。

## 步骤 4：运行程序

编译成功后，程序会自动启动。或手动运行：

```powershell
.\build\Release\GB28181Client.exe
```

## 步骤 5：配置服务器

在程序界面中：

1. **平台ID**：输入 `34020000002000000001`（或你的平台编码）
2. **IP**：输入 `0.0.0.0`（监听所有网卡）
3. **端口**：输入 `5060`（默认 SIP 端口）
4. 点击"启动服务"

## 步骤 6：配置下级设备

### 使用 ZLMediaKit 模拟设备

1. 下载 ZLMediaKit：https://github.com/ZLMediaKit/ZLMediaKit/releases
2. 编辑 `config.ini`：

```ini
[sip]
enabled=1
id=34020000001320000001
realm=3402000000
host=192.168.1.100
port=5061

[sip.realm.3402000000]
id=34020000002000000001
host=192.168.1.101
port=5060
username=admin
password=admin123
```

3. 启动 ZLMediaKit：

```bash
MediaServer.exe -c config.ini
```

### 使用真实设备

在设备 Web 界面配置上级平台：

- 平台接入码：`34020000002000000001`
- 平台IP：你的电脑IP（如 `192.168.1.101`）
- 平台端口：`5060`
- 设备编码：`34020000001320000001`

## 步骤 7：查看设备

设备注册成功后，会出现在左侧设备树中。

## 步骤 8：查询通道

1. 选中设备
2. 点击"查询通道"
3. 等待通道列表显示

## 步骤 9：播放视频

1. 选中通道
2. 点击"开始播放"
3. 视频会显示在中间窗口

## 步骤 10：PTZ 控制

选中支持 PTZ 的通道后，使用右侧控制面板控制云台。

## 常见问题

### Q1：编译失败

**A**：检查依赖库路径是否正确，查看 CMake 输出日志。

### Q2：设备无法注册

**A**：检查网络连通性、防火墙设置、设备配置。

### Q3：视频无法播放

**A**：检查 RTP 端口是否开放、视频编码格式是否支持。

## 下一步

- 阅读 [README.md](README.md) 了解详细功能
- 阅读 [WINDOWS_BUILD.md](WINDOWS_BUILD.md) 了解构建细节
- 阅读 [TESTING.md](TESTING.md) 了解测试方法
- 阅读 [PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md) 了解代码结构

## 获取帮助

- 查看文档：[README.md](README.md)
- 提交 Issue：[GitHub Issues](https://github.com/your-repo/issues)
- 邮件联系：your-email@example.com

---

**祝你使用愉快！** 🎉
