# GB28181 级联上级客户端

基于 Qt C++ 开发的 GB28181 级联上级平台客户端，可作为上级平台接收下级设备注册、查询通道目录、实时视频点播和 PTZ 控制。

## 功能特性

### 核心功能
- ✅ **SIP 服务器**：作为 GB28181 上级平台，接收下级设备注册
- ✅ **设备管理**：设备注册、心跳保活、状态监控
- ✅ **通道管理**：通道目录查询、通道状态管理
- ✅ **实时视频**：视频点播、RTP/PS 流接收、H264/H265 解码播放
- ✅ **PTZ 控制**：云台方向控制、变倍控制
- ✅ **录像查询**：历史录像检索（待完善）

### 技术特点
- 基于 eXosip2 实现 SIP 协议栈
- 支持 UDP/TCP 传输协议
- RTP/PS 流解析
- FFmpeg 视频解码
- Qt Widgets 图形界面

## 系统架构

```
┌─────────────────────────────────────────┐
│         GB28181Client (Qt)              │
├─────────────────────────────────────────┤
│  MainWindow (主界面)                    │
│  ├─ 设备树形视图                        │
│  ├─ 视频播放窗口                        │
│  └─ PTZ 控制面板                        │
├─────────────────────────────────────────┤
│  GB28181Server (SIP 服务器)             │
│  ├─ 设备注册处理                        │
│  ├─ Catalog 查询                        │
│  ├─ INVITE 视频点播                     │
│  └─ PTZ 控制命令                        │
├─────────────────────────────────────────┤
│  RtpReceiver (RTP 接收)                 │
│  └─ PsStreamParser (PS 流解析)          │
├─────────────────────────────────────────┤
│  VideoDecoder (FFmpeg 解码)             │
└─────────────────────────────────────────┘
```

## 依赖库

### 必需依赖
- **Qt 6.x**：GUI 框架
  - Qt6::Core
  - Qt6::Gui
  - Qt6::Widgets
  - Qt6::Network

- **eXosip2 5.x**：SIP 协议栈
  - libeXosip2
  - libosip2
  - libosipparser2

- **FFmpeg 4.x/5.x**：音视频编解码
  - libavcodec
  - libavformat
  - libavutil
  - libswscale
  - libswresample

### Windows 依赖
- Visual Studio 2019/2022 或 MinGW
- CMake 3.16+

## 编译构建

### Windows (Visual Studio)

1. **安装依赖**

```powershell
# 安装 Qt 6
# 从 https://www.qt.io/download 下载安装

# 下载预编译的 FFmpeg
# https://github.com/BtbN/FFmpeg-Builds/releases

# 下载预编译的 eXosip2
# https://github.com/cisco/libexosip2/releases
```

2. **配置环境变量**

```powershell
# 设置 Qt 路径
$env:Qt6_DIR = "C:\Qt\6.5.0\msvc2019_64"

# 设置 FFmpeg 路径
$env:FFMPEG_DIR = "C:\ffmpeg"

# 设置 eXosip2 路径
$env:EXOSIP2_DIR = "C:\exosip2"
```

3. **编译项目**

```powershell
cd GB28181Client
mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Linux

1. **安装依赖**

```bash
# Ubuntu/Debian
sudo apt-get install qt6-base-dev libexosip2-dev libavcodec-dev libavformat-dev libswscale-dev

# CentOS/RHEL
sudo yum install qt6-qtbase-devel libexosip2-devel ffmpeg-devel
```

2. **编译项目**

```bash
cd GB28181Client
mkdir build && cd build
cmake ..
make -j4
```

## 使用说明

### 1. 启动服务器

1. 配置平台参数：
   - **平台ID**：20位国标编码（如：34020000002000000001）
   - **IP地址**：本机IP或 0.0.0.0（监听所有网卡）
   - **端口**：SIP 端口（默认 5060）

2. 点击"启动服务"按钮

### 2. 设备注册

下级设备配置上级平台信息后，会自动注册到本客户端。注册成功后，设备会出现在左侧设备树中。

**下级设备配置示例**：
```
上级平台 ID: 34020000002000000001
上级平台 IP: 192.168.1.100
上级平台端口: 5060
本地设备 ID: 34020000001320000001
```

### 3. 查询通道

1. 在设备树中选中设备
2. 点击"查询通道"按钮
3. 等待通道列表返回并显示在设备节点下

### 4. 视频播放

1. 在设备树中选中通道
2. 点击"开始播放"按钮
3. 视频流会显示在中间的播放窗口

### 5. PTZ 控制

选中支持 PTZ 的通道后，使用右侧云台控制面板：
- **方向控制**：上、下、左、右
- **变倍控制**：放大、缩小

## 配置文件

### GB28181 配置参数

```cpp
GB28181Config config;
config.serverId = "34020000002000000001";  // 平台ID
config.serverIp = "0.0.0.0";               // 监听IP
config.serverPort = 5060;                  // SIP端口
config.domain = "3402000000";              // SIP域
config.expires = 3600;                     // 注册有效期(秒)
config.keepaliveInterval = 60;             // 心跳间隔(秒)
config.keepaliveTimeout = 3;               // 心跳超时次数
config.username = "admin";                 // 认证用户名
config.password = "admin123";              // 认证密码
```

## 国标编码规则

GB28181 设备编码为 20 位数字，格式如下：

```
中心编码(8位) + 行业编码(2位) + 类型编码(3位) + 网络标识(1位) + 序号(6位)

示例：
34020000002000000001
├─ 3402000000: 中心编码(省市区县)
├─ 20: 行业编码(视频监控)
├─ 000: 类型编码(平台)
├─ 0: 网络标识
└─ 000001: 序号
```

## 常见问题

### 1. 设备无法注册

**检查项**：
- 网络连通性（ping 测试）
- 防火墙是否开放 5060 端口
- 设备配置的上级平台 ID 是否正确
- SIP 域配置是否匹配

### 2. 视频无法播放

**检查项**：
- RTP 端口是否被占用（默认 9000）
- 防火墙是否开放 RTP 端口
- 设备是否支持 PS 封装
- 视频编码格式是否为 H264/H265

### 3. PTZ 控制无响应

**检查项**：
- 通道是否支持 PTZ（PTZType > 0）
- 设备是否正确连接云台
- PTZ 协议配置是否正确

## 开发计划

- [ ] 录像回放功能
- [ ] 多窗口分屏播放
- [ ] 视频录制和截图
- [ ] 设备配置管理
- [ ] 告警信息接收
- [ ] 语音对讲
- [ ] 级联下级平台

## 参考资料

- [GB/T 28181-2016 标准文档](http://www.gb688.cn/bzgk/gb/newGbInfo?hcno=469659DC56B9B8187671FF08748CEC89)
- [eXosip2 官方文档](http://savannah.nongnu.org/projects/exosip)
- [FFmpeg 官方文档](https://ffmpeg.org/documentation.html)
- [Qt 6 官方文档](https://doc.qt.io/qt-6/)

## 许可证

MIT License

## 贡献

欢迎提交 Issue 和 Pull Request。

## 联系方式

如有问题，请提交 Issue 或联系开发者。
