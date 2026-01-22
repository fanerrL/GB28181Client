#ifndef PS_STREAM_PARSER_H
#define PS_STREAM_PARSER_H

#include <QObject>
#include <QByteArray>
#include "GB28181Types.h"

namespace GB28181 {

// PS 包类型
enum class PsPacketType {
    PackHeader = 0xBA,      // PS 包头
    SystemHeader = 0xBB,    // 系统头
    ProgramMap = 0xBC,      // 节目流映射
    PrivateStream1 = 0xBD,  // 私有流1
    PaddingStream = 0xBE,   // 填充流
    PrivateStream2 = 0xBF,  // 私有流2
    AudioStream = 0xC0,     // 音频流 (0xC0-0xDF)
    VideoStream = 0xE0,     // 视频流 (0xE0-0xEF)
    Unknown = 0xFF
};

class PsStreamParser : public QObject
{
    Q_OBJECT

public:
    explicit PsStreamParser(QObject *parent = nullptr);
    ~PsStreamParser();

    // 解析 PS 流
    void parse(const QByteArray &data);

    // 重置解析器
    void reset();

signals:
    // H264/H265 视频帧
    void videoFrame(const QByteArray &frame, VideoCodec codec, bool isKeyFrame);

    // 音频帧
    void audioFrame(const QByteArray &frame);

    // 错误
    void errorOccurred(const QString &error);

private:
    // 解析 PS 包头
    bool parsePsPackHeader(const QByteArray &data, int &offset);

    // 解析系统头
    bool parseSystemHeader(const QByteArray &data, int &offset);

    // 解析节目流映射
    bool parseProgramStreamMap(const QByteArray &data, int &offset);

    // 解析 PES 包
    bool parsePesPacket(const QByteArray &data, int &offset);

    // 提取 H264 帧
    void extractH264Frame(const QByteArray &pesPayload);

    // 检查是否为关键帧
    bool isH264KeyFrame(const QByteArray &data);

    // 查找起始码
    int findStartCode(const QByteArray &data, int offset);

private:
    QByteArray m_videoBuffer;
    QByteArray m_audioBuffer;
    VideoCodec m_videoCodec;
    bool m_hasSystemHeader;
};

} // namespace GB28181

#endif // PS_STREAM_PARSER_H
