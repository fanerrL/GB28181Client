#ifndef GB28181_TYPES_H
#define GB28181_TYPES_H

#include <QString>
#include <QDateTime>
#include <QList>
#include <QMap>

namespace GB28181 {

// 设备状态
enum class DeviceStatus {
    Offline,
    Online,
    Registering
};

// 通道状态
enum class ChannelStatus {
    Offline,
    Online
};

// 视频编码格式
enum class VideoCodec {
    H264,
    H265,
    SVAC,
    Unknown
};

// 传输协议
enum class TransportProtocol {
    UDP,
    TCP
};

// PTZ 命令类型
enum class PTZCommand {
    Stop = 0,
    Up = 1,
    Down = 2,
    Left = 3,
    Right = 4,
    UpLeft = 5,
    UpRight = 6,
    DownLeft = 7,
    DownRight = 8,
    ZoomIn = 9,
    ZoomOut = 10,
    FocusNear = 11,
    FocusFar = 12,
    IrisOpen = 13,
    IrisClose = 14
};

// 设备信息
struct DeviceInfo {
    QString deviceId;           // 设备国标编码 (20位)
    QString name;               // 设备名称
    QString manufacturer;       // 厂商
    QString model;              // 型号
    QString firmware;           // 固件版本
    QString ip;                 // IP地址
    int port;                   // 端口
    QString transport;          // 传输协议 (UDP/TCP)
    DeviceStatus status;        // 设备状态
    QDateTime registerTime;     // 注册时间
    QDateTime lastKeepalive;    // 最后心跳时间
    int keepaliveInterval;      // 心跳间隔(秒)
    int keepaliveTimeout;       // 心跳超时次数

    DeviceInfo()
        : port(5060)
        , status(DeviceStatus::Offline)
        , keepaliveInterval(60)
        , keepaliveTimeout(0) {}
};

// 通道信息
struct ChannelInfo {
    QString channelId;          // 通道国标编码 (20位)
    QString deviceId;           // 所属设备ID
    QString name;               // 通道名称
    QString manufacturer;       // 厂商
    QString model;              // 型号
    ChannelStatus status;       // 通道状态
    QString parentId;           // 父设备ID
    bool hasSubChannel;         // 是否有子通道
    QString longitude;          // 经度
    QString latitude;           // 维度
    bool ptzSupport;            // 是否支持PTZ

    ChannelInfo()
        : status(ChannelStatus::Offline)
        , hasSubChannel(false)
        , ptzSupport(false) {}
};

// 媒体流信息
struct MediaStreamInfo {
    QString ssrc;               // SSRC
    QString ip;                 // 媒体流IP
    int port;                   // 媒体流端口
    TransportProtocol protocol; // 传输协议
    VideoCodec codec;           // 视频编码
    int bitrate;                // 码率
    int framerate;              // 帧率

    MediaStreamInfo()
        : port(0)
        , protocol(TransportProtocol::UDP)
        , codec(VideoCodec::H264)
        , bitrate(0)
        , framerate(0) {}
};

// SIP 会话信息
struct SipSession {
    int callId;                 // eXosip call ID
    int dialogId;               // eXosip dialog ID
    QString deviceId;           // 设备ID
    QString channelId;          // 通道ID
    MediaStreamInfo mediaInfo;  // 媒体流信息
    QDateTime startTime;        // 会话开始时间

    SipSession() : callId(-1), dialogId(-1) {}
};

// 录像文件信息
struct RecordFileInfo {
    QString deviceId;           // 设备ID
    QString name;               // 文件名
    QString filePath;           // 文件路径
    QString address;            // 录像地址
    QDateTime startTime;        // 开始时间
    QDateTime endTime;          // 结束时间
    int secrecy;                // 保密属性
    QString type;               // 录像类型
    QString recorderId;         // 录像触发者ID

    RecordFileInfo() : secrecy(0) {}
};

// GB28181 配置
struct GB28181Config {
    QString serverId;           // 本平台ID (20位)
    QString serverIp;           // 本平台IP
    int serverPort;             // 本平台SIP端口
    QString domain;             // SIP域
    int expires;                // 注册有效期(秒)
    int keepaliveInterval;      // 心跳间隔(秒)
    int keepaliveTimeout;       // 心跳超时次数
    QString username;           // 认证用户名
    QString password;           // 认证密码
    int maxChannels;            // 最大通道数

    GB28181Config()
        : serverId("34020000002000000001")
        , serverIp("0.0.0.0")
        , serverPort(5060)
        , domain("3402000000")
        , expires(3600)
        , keepaliveInterval(60)
        , keepaliveTimeout(3)
        , username("admin")
        , password("admin123")
        , maxChannels(256) {}
};

} // namespace GB28181

#endif // GB28181_TYPES_H
