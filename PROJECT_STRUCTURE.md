# 项目结构说明

```
GB28181Client/
├── CMakeLists.txt              # CMake 构建配置
├── README.md                   # 项目说明文档
├── WINDOWS_BUILD.md            # Windows 构建指南
├── build.bat                   # Windows 快速构建脚本
│
├── include/                    # 头文件目录
│   ├── GB28181Types.h         # GB28181 数据类型定义
│   ├── GB28181Server.h        # SIP 服务器（核心）
│   ├── DeviceManager.h        # 设备管理器
│   ├── ChannelManager.h       # 通道管理器
│   ├── RtpReceiver.h          # RTP 接收器
│   ├── PsStreamParser.h       # PS 流解析器
│   ├── VideoDecoder.h         # 视频解码器
│   ├── VideoPlayer.h          # 视频播放器（待实现）
│   ├── PTZController.h        # PTZ 控制器（待实现）
│   ├── SipMessageHandler.h    # SIP 消息处理器（待实现）
│   └── MainWindow.h           # 主窗口
│
├── src/                        # 源文件目录
│   ├── main.cpp               # 程序入口
│   ├── GB28181Server.cpp      # SIP 服务器实现
│   ├── DeviceManager.cpp      # 设备管理器实现
│   ├── ChannelManager.cpp     # 通道管理器实现
│   ├── RtpReceiver.cpp        # RTP 接收器实现
│   ├── PsStreamParser.cpp     # PS 流解析器实现
│   ├── VideoDecoder.cpp       # 视频解码器实现
│   └── MainWindow.cpp         # 主窗口实现
│
├── ui/                         # UI 文件目录（可选）
│   └── MainWindow.ui          # Qt Designer UI 文件
│
├── resources/                  # 资源文件目录
│   ├── resources.qrc          # Qt 资源文件
│   ├── icons/                 # 图标
│   └── images/                # 图片
│
├── 3rdparty/                   # 第三方库目录
│   ├── ffmpeg/                # FFmpeg 库
│   │   ├── include/
│   │   └── lib/
│   └── exosip2/               # eXosip2 库
│       ├── include/
│       └── lib/
│
├── build/                      # 构建输出目录（自动生成）
│   ├── Debug/
│   └── Release/
│
└── docs/                       # 文档目录
    ├── API.md                 # API 文档
    ├── PROTOCOL.md            # GB28181 协议说明
    └── DEVELOPMENT.md         # 开发指南
```

## 核心模块说明

### 1. GB28181Server (SIP 服务器)

**职责**：
- 作为 GB28181 上级平台，处理 SIP 协议
- 接收下级设备注册
- 发送 Catalog 查询
- 发送 INVITE 点播视频
- 发送 PTZ 控制命令

**关键方法**：
```cpp
bool start(const GB28181Config &config);
bool sendCatalogQuery(const QString &deviceId);
bool sendInvite(const QString &deviceId, const QString &channelId, ...);
bool sendPTZControl(const QString &deviceId, const QString &channelId, PTZCommand cmd, int speed);
```

**信号**：
```cpp
void deviceRegistered(const DeviceInfo &device);
void catalogReceived(const QString &deviceId, const QList<ChannelInfo> &channels);
void mediaStreamReady(const SipSession &session);
```

### 2. DeviceManager (设备管理器)

**职责**：
- 管理已注册设备列表
- 监控设备心跳状态
- 处理设备超时

**关键方法**：
```cpp
void addDevice(const DeviceInfo &device);
void updateDeviceKeepalive(const QString &deviceId);
QList<DeviceInfo> getAllDevices() const;
```

### 3. ChannelManager (通道管理器)

**职责**：
- 管理设备通道列表
- 维护设备与通道的关系

**关键方法**：
```cpp
void addChannel(const ChannelInfo &channel);
QList<ChannelInfo> getDeviceChannels(const QString &deviceId) const;
```

### 4. RtpReceiver (RTP 接收器)

**职责**：
- 接收 RTP 数据包
- 提取 RTP payload
- 重组 PS 流

**关键方法**：
```cpp
bool start(const QString &ip, int port);
void processRtpPacket(const QByteArray &packet);
```

**信号**：
```cpp
void psStreamData(const QByteArray &data);
```

### 5. PsStreamParser (PS 流解析器)

**职责**：
- 解析 PS 包结构
- 提取 PES 包
- 分离音视频流
- 提取 H264/H265 NALU

**关键方法**：
```cpp
void parse(const QByteArray &data);
bool parsePesPacket(const QByteArray &data, int &offset);
void extractH264Frame(const QByteArray &pesPayload);
```

**信号**：
```cpp
void videoFrame(const QByteArray &frame, VideoCodec codec, bool isKeyFrame);
void audioFrame(const QByteArray &frame);
```

### 6. VideoDecoder (视频解码器)

**职责**：
- 使用 FFmpeg 解码 H264/H265
- 转换为 RGB 格式
- 输出 QImage

**关键方法**：
```cpp
bool init(VideoCodec codec);
bool decode(const QByteArray &data);
```

**信号**：
```cpp
void frameDecoded(const QImage &image);
```

### 7. MainWindow (主窗口)

**职责**：
- 用户界面
- 协调各模块工作
- 显示设备和通道
- 视频播放
- PTZ 控制

**UI 组件**：
- 设备树形视图
- 视频显示区域
- PTZ 控制面板
- 日志输出

## 数据流程

### 设备注册流程

```
下级设备 -> SIP REGISTER -> GB28181Server
                              ↓
                         DeviceManager (添加设备)
                              ↓
                         MainWindow (更新设备树)
```

### 通道查询流程

```
MainWindow (点击查询) -> GB28181Server::sendCatalogQuery()
                              ↓
                         SIP MESSAGE (Catalog)
                              ↓
                         下级设备响应
                              ↓
                         GB28181Server::parseCatalog()
                              ↓
                         ChannelManager (添加通道)
                              ↓
                         MainWindow (更新通道树)
```

### 视频播放流程

```
MainWindow (点击播放) -> RtpReceiver::start()
                              ↓
                         VideoDecoder::init()
                              ↓
                         GB28181Server::sendInvite()
                              ↓
                         下级设备发送 RTP 流
                              ↓
                         RtpReceiver (接收 RTP)
                              ↓
                         PsStreamParser (解析 PS)
                              ↓
                         VideoDecoder (解码 H264)
                              ↓
                         MainWindow (显示图像)
```

## 线程模型

- **主线程**：UI 和事件循环
- **SIP 事件线程**：eXosip 事件处理（通过 QTimer）
- **RTP 接收线程**：QUdpSocket 自动管理
- **解码线程**：可选，当前在主线程

## 配置参数

### GB28181Config

```cpp
struct GB28181Config {
    QString serverId;           // 平台ID (20位)
    QString serverIp;           // 监听IP
    int serverPort;             // SIP端口 (默认5060)
    QString domain;             // SIP域
    int expires;                // 注册有效期(秒)
    int keepaliveInterval;      // 心跳间隔(秒)
    int keepaliveTimeout;       // 心跳超时次数
    QString username;           // 认证用户名
    QString password;           // 认证密码
};
```

## 扩展开发

### 添加新功能

1. **录像回放**
   - 实现 RecordManager 类
   - 发送 RecordInfo 查询
   - 发送 Playback INVITE
   - 支持回放控制（暂停、快进、慢放）

2. **多窗口播放**
   - 创建 VideoWidget 类
   - 支持 1/4/9/16 分屏
   - 每个窗口独立的 RtpReceiver 和 VideoDecoder

3. **录像和截图**
   - 使用 FFmpeg 编码保存视频
   - QImage::save() 保存截图

4. **告警处理**
   - 解析 Alarm 消息
   - 显示告警列表
   - 告警联动

### 性能优化

1. **多线程解码**
   - 将 VideoDecoder 移到工作线程
   - 使用 QThread 或 QtConcurrent

2. **缓冲优化**
   - 实现 JitterBuffer 抖动缓冲
   - 优化 PS 流缓冲区大小

3. **GPU 加速**
   - 使用 FFmpeg 硬件加速（DXVA2、NVDEC）
   - OpenGL 渲染

## 调试技巧

### 1. 启用详细日志

```cpp
// main.cpp
qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] %{file}:%{line} - %{message}");
```

### 2. 抓包分析

使用 Wireshark 过滤器：
```
sip || rtp
```

### 3. 断点调试

关键位置：
- `GB28181Server::handleRegister()` - 设备注册
- `GB28181Server::handleMessage()` - 消息处理
- `RtpReceiver::processRtpPacket()` - RTP 接收
- `PsStreamParser::parsePesPacket()` - PS 解析
- `VideoDecoder::decode()` - 视频解码

## 常见问题

### 1. 编译错误

- 检查依赖库路径
- 确保所有库版本兼容
- 查看 CMake 输出日志

### 2. 运行时错误

- 使用 Dependency Walker 检查 DLL
- 检查防火墙设置
- 查看应用程序日志

### 3. 视频无法播放

- 检查 RTP 端口是否开放
- 确认视频编码格式
- 查看 FFmpeg 解码日志

## 参考资料

- [GB/T 28181-2016 标准](http://www.gb688.cn/bzgk/gb/newGbInfo?hcno=469659DC56B9B8187671FF08748CEC89)
- [eXosip2 文档](http://savannah.nongnu.org/projects/exosip)
- [FFmpeg 文档](https://ffmpeg.org/documentation.html)
- [Qt 文档](https://doc.qt.io/qt-6/)
