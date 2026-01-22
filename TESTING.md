# GB28181 测试指南

本文档说明如何测试 GB28181Client 的各项功能。

## 测试环境搭建

### 方案一：使用 ZLMediaKit 模拟下级设备

ZLMediaKit 是一个开源的流媒体服务器，支持 GB28181 协议。

#### 1. 下载 ZLMediaKit

```bash
# Windows
https://github.com/ZLMediaKit/ZLMediaKit/releases

# Linux
git clone https://github.com/ZLMediaKit/ZLMediaKit.git
cd ZLMediaKit
mkdir build && cd build
cmake ..
make -j4
```

#### 2. 配置 ZLMediaKit

编辑 `config.ini`：

```ini
[general]
# 流媒体服务器配置
mediaServerId=your_server_id

[sip]
# GB28181 配置
enabled=1
# 本设备ID (20位)
id=34020000001320000001
# 本设备域
realm=3402000000
# 本设备IP
host=192.168.1.100
# 本设备端口
port=5061

# 上级平台配置
[sip.realm.3402000000]
# 上级平台ID
id=34020000002000000001
# 上级平台IP
host=192.168.1.101
# 上级平台端口
port=5060
# 认证用户名
username=admin
# 认证密码
password=admin123
# 注册有效期
expires=3600
# 心跳间隔
keepalive_interval=60
```

#### 3. 启动 ZLMediaKit

```bash
# Windows
MediaServer.exe -c config.ini

# Linux
./MediaServer -c config.ini
```

#### 4. 推流测试

使用 FFmpeg 推流到 ZLMediaKit：

```bash
ffmpeg -re -i test.mp4 -vcodec h264 -acodec aac -f rtsp rtsp://127.0.0.1/live/test
```

### 方案二：使用真实 GB28181 设备

如果有真实的 GB28181 摄像头或 NVR，直接配置上级平台参数即可。

**配置示例（海康威视）**：
1. 登录设备 Web 界面
2. 进入"配置" -> "网络" -> "高级配置" -> "平台接入"
3. 选择"国标GB28181"
4. 填写上级平台信息：
   - 平台接入码：34020000002000000001
   - 平台IP：192.168.1.101
   - 平台端口：5060
   - 本地SIP端口：5061
   - 设备编码：34020000001320000001
   - 通道编码：34020000001320000001（摄像头）
   - 注册有效期：3600
   - 心跳周期：60
   - 最大心跳超时次数：3

## 测试步骤

### 1. 启动 GB28181Client

1. 运行 `GB28181Client.exe`
2. 配置服务器参数：
   - 平台ID：34020000002000000001
   - IP：0.0.0.0（监听所有网卡）
   - 端口：5060
3. 点击"启动服务"

**预期结果**：
- 日志显示"服务器已启动"
- 按钮状态变化（启动按钮禁用，停止按钮启用）

### 2. 设备注册测试

启动下级设备（ZLMediaKit 或真实设备）。

**预期结果**：
- 日志显示"设备注册: xxx"
- 设备树中出现新设备节点
- 设备状态显示"在线"

**检查点**：
- 设备ID 是否正确（20位数字）
- 设备IP 是否正确
- 注册时间是否正确

### 3. 心跳测试

等待 60 秒（心跳间隔）。

**预期结果**：
- 日志定期显示心跳信息
- 设备保持在线状态

**异常测试**：
1. 停止下级设备
2. 等待 3 个心跳周期（180秒）

**预期结果**：
- 设备状态变为"离线"
- 日志显示"设备超时"

### 4. 通道查询测试

1. 在设备树中选中设备
2. 点击"查询通道"按钮

**预期结果**：
- 日志显示"查询设备通道: xxx"
- 日志显示"收到设备 xxx 的 N 个通道"
- 设备节点下出现通道列表
- 通道名称、ID、状态正确显示

**检查点**：
- 通道数量是否正确
- 通道ID 格式是否正确（20位）
- 通道状态是否正确

### 5. 视频播放测试

1. 在设备树中选中通道
2. 点击"开始播放"按钮

**预期结果**：
- 日志显示"开始播放通道: xxx"
- 日志显示"媒体流就绪: IP:Port"
- 视频窗口显示实时画面
- 画面流畅，无明显卡顿

**检查点**：
- 视频分辨率是否正确
- 帧率是否正常（15-25fps）
- 是否有花屏或绿屏
- 延迟是否可接受（< 2秒）

**性能测试**：
- CPU 占用率（应 < 30%）
- 内存占用（应 < 200MB）
- 网络带宽（根据码率）

### 6. PTZ 控制测试

前提：通道支持 PTZ 控制。

1. 选中支持 PTZ 的通道
2. 点击 PTZ 控制按钮

**测试项**：
- 上：摄像头向上转动
- 下：摄像头向下转动
- 左：摄像头向左转动
- 右：摄像头向右转动
- 放大：镜头拉近
- 缩小：镜头拉远

**预期结果**：
- 摄像头响应控制命令
- 动作流畅，无延迟
- 停止后摄像头停止转动

### 7. 停止播放测试

点击"停止播放"按钮。

**预期结果**：
- 日志显示"停止播放"
- 视频窗口变黑
- 按钮状态恢复
- 资源正确释放（内存不增长）

### 8. 多设备测试

启动多个下级设备（2-5个）。

**预期结果**：
- 所有设备正常注册
- 设备树正确显示所有设备
- 可以分别查询每个设备的通道
- 可以切换播放不同设备的视频

### 9. 长时间稳定性测试

连续运行 24 小时。

**检查点**：
- 程序不崩溃
- 内存不泄漏（内存占用稳定）
- 设备保持在线
- 视频播放正常

## 抓包分析

### 使用 Wireshark 抓包

1. 启动 Wireshark
2. 选择网卡
3. 设置过滤器：`sip || rtp`
4. 开始抓包

### SIP 消息分析

#### REGISTER 消息

```
REGISTER sip:34020000002000000001@3402000000 SIP/2.0
Via: SIP/2.0/UDP 192.168.1.100:5061;rport;branch=z9hG4bK...
From: <sip:34020000001320000001@3402000000>;tag=...
To: <sip:34020000001320000001@3402000000>
Call-ID: ...
CSeq: 1 REGISTER
Contact: <sip:34020000001320000001@192.168.1.100:5061>
Max-Forwards: 70
Expires: 3600
User-Agent: ...
Content-Length: 0
```

#### Catalog 查询

```
MESSAGE sip:34020000001320000001@3402000000 SIP/2.0
...
Content-Type: Application/MANSCDP+xml

<?xml version="1.0" encoding="GB2312"?>
<Query>
<CmdType>Catalog</CmdType>
<SN>1</SN>
<DeviceID>34020000001320000001</DeviceID>
</Query>
```

#### INVITE 消息

```
INVITE sip:34020000001320000001@3402000000 SIP/2.0
...
Content-Type: application/sdp

v=0
o=34020000002000000001 0 0 IN IP4 192.168.1.101
s=Play
c=IN IP4 192.168.1.101
t=0 0
m=video 9000 RTP/AVP 96 98 97
a=rtpmap:96 PS/90000
a=rtpmap:98 H264/90000
a=rtpmap:97 MPEG4/90000
a=recvonly
a=ssrc:0123456789
y=0123456789
```

### RTP 流分析

1. 在 Wireshark 中选择一个 RTP 包
2. 右键 -> "Decode As" -> "RTP"
3. 菜单 -> "Telephony" -> "RTP" -> "Stream Analysis"

**检查项**：
- Packet Loss（丢包率应 < 1%）
- Jitter（抖动应 < 30ms）
- Sequence（序列号连续）

## 常见问题排查

### 1. 设备无法注册

**检查步骤**：
1. 网络连通性：`ping 192.168.1.101`
2. 端口开放：`telnet 192.168.1.101 5060`
3. 防火墙规则
4. 设备配置（平台ID、IP、端口）
5. Wireshark 抓包查看 SIP 消息

### 2. 通道查询无响应

**检查步骤**：
1. 设备是否在线
2. Wireshark 查看是否发送 Catalog 查询
3. 设备是否返回响应
4. XML 解析是否正确

### 3. 视频无法播放

**检查步骤**：
1. RTP 端口是否开放（默认 9000）
2. Wireshark 查看是否收到 RTP 包
3. 视频编码格式（必须是 H264/H265）
4. FFmpeg 解码日志
5. PS 流解析是否正确

### 4. 视频卡顿或花屏

**可能原因**：
- 网络丢包严重
- 码率过高
- 解码性能不足
- PS 流解析错误

**解决方法**：
- 降低视频码率
- 优化解码性能
- 检查 PS 流解析逻辑

### 5. PTZ 控制无响应

**检查步骤**：
1. 通道是否支持 PTZ（PTZType > 0）
2. Wireshark 查看是否发送 PTZ 命令
3. 设备是否返回 200 OK
4. PTZ 命令字节是否正确

## 性能测试

### CPU 占用测试

使用任务管理器或 Process Explorer 监控 CPU 占用。

**基准**：
- 空闲：< 5%
- 单路视频播放：< 20%
- 4路视频播放：< 50%

### 内存占用测试

**基准**：
- 启动后：< 100MB
- 单路视频播放：< 150MB
- 4路视频播放：< 300MB
- 长时间运行：内存不增长

### 网络带宽测试

使用 Wireshark 或 NetLimiter 监控网络流量。

**基准**：
- 720P (2Mbps)：约 250KB/s
- 1080P (4Mbps)：约 500KB/s

## 自动化测试

### 单元测试

使用 Qt Test 框架编写单元测试：

```cpp
#include <QtTest>
#include "PsStreamParser.h"

class TestPsStreamParser : public QObject
{
    Q_OBJECT

private slots:
    void testParsePsPackHeader();
    void testParsePesPacket();
    void testExtractH264Frame();
};

void TestPsStreamParser::testParsePsPackHeader()
{
    // 测试代码
}

QTEST_MAIN(TestPsStreamParser)
#include "test_ps_stream_parser.moc"
```

### 集成测试

编写脚本自动测试完整流程：

```python
import socket
import time

def test_device_register():
    # 模拟设备注册
    pass

def test_catalog_query():
    # 测试通道查询
    pass

def test_video_play():
    # 测试视频播放
    pass

if __name__ == '__main__':
    test_device_register()
    test_catalog_query()
    test_video_play()
```

## 测试报告模板

```markdown
# GB28181Client 测试报告

## 测试环境
- 操作系统：Windows 10 64位
- CPU：Intel i5-8400
- 内存：16GB
- 网络：千兆以太网

## 测试设备
- 设备类型：ZLMediaKit / 海康威视 DS-2CD2xxx
- 设备数量：3
- 通道数量：6

## 测试结果

### 功能测试
| 测试项 | 结果 | 备注 |
|--------|------|------|
| 设备注册 | ✅ 通过 | 3个设备全部注册成功 |
| 心跳保活 | ✅ 通过 | 心跳正常，无超时 |
| 通道查询 | ✅ 通过 | 6个通道全部查询成功 |
| 视频播放 | ✅ 通过 | 画面流畅，延迟 < 1秒 |
| PTZ 控制 | ✅ 通过 | 响应及时，动作准确 |
| 停止播放 | ✅ 通过 | 资源正确释放 |

### 性能测试
| 指标 | 测试值 | 基准值 | 结果 |
|------|--------|--------|------|
| CPU 占用（单路） | 15% | < 20% | ✅ |
| 内存占用（单路） | 120MB | < 150MB | ✅ |
| 视频延迟 | 0.8秒 | < 2秒 | ✅ |
| 丢包率 | 0.1% | < 1% | ✅ |

### 稳定性测试
- 连续运行时间：24小时
- 崩溃次数：0
- 内存泄漏：无
- 结论：✅ 通过

## 问题记录
1. 无

## 总结
所有测试项均通过，系统运行稳定。
```

## 参考资料

- [GB/T 28181-2016 测试规范](http://www.gb688.cn/bzgk/gb/newGbInfo?hcno=469659DC56B9B8187671FF08748CEC89)
- [Wireshark 使用指南](https://www.wireshark.org/docs/)
- [ZLMediaKit 文档](https://github.com/ZLMediaKit/ZLMediaKit/wiki)
